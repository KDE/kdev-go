/*************************************************************************************
*  Copyright (C) 2014 by Pavel Petrushkov <onehundredof@gmail.com>                  *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

//Completion code is mostly based on KDevelop QmlJS plugin which should be referenced for more details amd comments

#include "context.h"

#include <language/codecompletion/normaldeclarationcompletionitem.h>

#include "parser/golexer.h"
#include "parser/goparser.h"
#include "expressionvisitor.h"
#include "types/gostructuretype.h"
#include "helper.h"

using namespace KDevelop;



namespace go
{
    
CodeCompletionContext::CodeCompletionContext(const KDevelop::DUContextPointer& context,
					     const QString& text, 
					     const KDevelop::CursorInRevision& position, int depth): 
					     KDevelop::CodeCompletionContext(context, extractLastLine(text), position, depth)
{
}

QList< CompletionTreeItemPointer > CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    kDebug() << "Completion items test";
    kDebug() << m_text;
    QList<CompletionTreeItemPointer> items;
    
    QChar lastChar = m_text.size() > 0 ? m_text.at(m_text.size() - 1) : QLatin1Char('\0');
    if(lastChar == QLatin1Char('.'))
    {//imports and member completions
	items << importAndMemberCompletion();
	
    }else
    {//all declarations
	DUChainReadLocker lock;
	auto declarations = m_duContext->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext());
	for(const QPair<Declaration*, int> &decl : declarations)
	{
	    items << CompletionTreeItemPointer(new NormalDeclarationCompletionItem(DeclarationPointer(decl.first), 
										   KSharedPtr<KDevelop::CodeCompletionContext>(), decl.second));
										  
	}
	
    }
    return items;
}

QList< CompletionTreeItemPointer > CodeCompletionContext::importAndMemberCompletion()
{
    QList<CompletionTreeItemPointer> items;
    AbstractType::Ptr lasttype = lastType(m_text.left(m_text.size()-1));
    if(lasttype)
    {
	if(fastCast<StructureType*>(lasttype.constData()))
	{//we have to look for namespace declarations
	    //TODO handle namespace aliases
	    DUChainReadLocker lock;
	    Declaration* lastdeclaration = fastCast<StructureType*>(lasttype.constData())->declaration(m_duContext->topContext());
	    //if(lastdeclaration->kind() == Declaration::Namespace)
	    //{
		//namespace could be splitted into multiple contexts
		//auto decls = m_duContext->findDeclarations(lastdeclaration->qualifiedIdentifier());
		auto decls = getDeclarations(lastdeclaration->qualifiedIdentifier(), m_duContext.data());
		for(Declaration* declaration : decls)
		{
		    DUContext* context = declaration->internalContext();
		    if(!context) continue;
		    auto declarations = context->allDeclarations(CursorInRevision::invalid(), declaration->topContext(), false);
		    for(const QPair<Declaration*, int> decl : declarations)
		    {
			if(decl.first == declaration)
			    continue;
			QualifiedIdentifier fullname = decl.first->qualifiedIdentifier();
			Identifier ident = fullname.last();
			if(ident.toString().size() <= 0)
			    continue;
			//import only declarations that start with capital letter(Go language rule)
			if(m_duContext->topContext() != declaration->topContext())
			    if(ident.toString().at(0) != ident.toString().at(0).toUpper())
				continue;
			items << CompletionTreeItemPointer(new NormalDeclarationCompletionItem(DeclarationPointer(decl.first), 
										    KSharedPtr<KDevelop::CodeCompletionContext>(), decl.second));
		    }
		}
	   // }
	}
	//this construction will descend through type hierarchy till it hits basic types
	//e.g. type mystruct struct{}; type mystruct2 mystruct; ...
	int count = 0;
	do {
	    count++;
	    GoStructureType* structure = fastCast<GoStructureType*>(lasttype.constData());
	    if(structure)
	    {//get members
		DUContext* context = structure->context();
		DUChainReadLocker lock;
		//auto declarations = context->findDeclarations(identifierForNode(node->selector));
		auto declarations = context->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext(), false);
		lock.unlock();
		for(const QPair<Declaration*, int> &decl : declarations)
		{
		    items << CompletionTreeItemPointer(new NormalDeclarationCompletionItem(DeclarationPointer(decl.first), 
										   KSharedPtr<KDevelop::CodeCompletionContext>(), decl.second));
		}
	    }
	    StructureType* identType = fastCast<StructureType*>(lasttype.constData());
	    if(identType)
	    {
		DUChainReadLocker lock;
		lasttype = identType->declaration(m_duContext->topContext())->abstractType();
	    }else 
		break;
	    
	}while(lasttype && count<100);
    }
    return items;
}


QStack< CodeCompletionContext::ExpressionStackEntry > CodeCompletionContext::expressionStack(const QString& expression)
{
    //for details see similar function in QmlJS KDevelop plugin
    QStack<CodeCompletionContext::ExpressionStackEntry> stack;
    QByteArray expr(expression.toUtf8());
    KDevPG::QByteArrayIterator iter(expr);
    Lexer lexer(iter);
    bool atEnd=false;
    ExpressionStackEntry entry;
    
    entry.startPosition = 0;
    entry.operatorStart = 0;
    entry.operatorEnd = 0;
    entry.commas = 0;
    
    stack.push(entry);
    
    qint64 line, lineEnd, column, columnEnd;
    while(!atEnd)
    {
	KDevPG::Token token(lexer.read());
	switch(token.kind)
	{
	case Parser::Token_EOF:
		atEnd=true;
		break;
	case Parser::Token_LBRACE:
	case Parser::Token_LBRACKET:
	case Parser::Token_LPAREN:
	    qint64 sline, scolumn;
	    lexer.locationTable()->positionAt(token.begin, &sline, &scolumn);
            entry.startPosition = scolumn+1;
            entry.operatorStart = entry.startPosition;
            entry.operatorEnd = entry.startPosition;
            entry.commas = 0;

            stack.push(entry);
            break;
	case Parser::Token_RBRACE:
	case Parser::Token_RBRACKET:
	case Parser::Token_RPAREN:
            if (stack.count() > 1) {
                stack.pop();
            }
            break;
	case Parser::Token_IDENT:
	    //temporary hack to allow completion in variable declarations
	    //two identifiers in a row is not possible? 
	    if(lexer.size() > 0 && lexer.at(lexer.index()-2).kind == Parser::Token_IDENT)
	    {
		lexer.locationTable()->positionAt(lexer.at(lexer.index()-2).begin, &line, &column);
		lexer.locationTable()->positionAt(lexer.at(lexer.index()-2).end+1, &lineEnd, &columnEnd);
		stack.top().operatorStart = column;
		stack.top().operatorEnd = columnEnd;
	    }
	    break;
	case Parser::Token_DOT:
            break;
	case Parser::Token_COMMA:
            stack.top().commas++;
        default:
            // The last operator of every sub-expression is stored on the stack
            // so that "A = foo." can know that attributes of foo having the same
            // type as A should be highlighted.
	    kDebug() << token.kind;
	    lexer.locationTable()->positionAt(token.begin, &line, &column);
	    lexer.locationTable()->positionAt(token.end+1, &lineEnd, &columnEnd);
            stack.top().operatorStart = column;
            stack.top().operatorEnd = columnEnd;
	    
	}
    }
    return stack;
}

AbstractType::Ptr CodeCompletionContext::lastType(const QString& expression)
{
    QStack<ExpressionStackEntry> stack = expressionStack(expression);
    QString lastExpression(expression.mid(stack.top().operatorEnd));
    kDebug() << lastExpression;

    ParseSession session(lastExpression.toUtf8(), 0, false);
    ExpressionAst* expressionAst;
    if(!session.parseExpression(&expressionAst))
	return AbstractType::Ptr();


    ExpressionVisitor expVisitor(&session, this->m_duContext.data());
    expVisitor.visitExpression(expressionAst);
    if(expVisitor.lastTypes().size() != 0)
    {
	 AbstractType::Ptr type = expVisitor.lastTypes().first();
	 return type;
    }

    return AbstractType::Ptr();
}


}