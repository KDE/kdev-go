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
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/classdeclaration.h>
#include <duchain/types/gointegraltype.h>
#include <duchain/types/gomaptype.h>
#include <duchain/types/gochantype.h>

#include "parser/golexer.h"
#include "parser/goparser.h"
#include "expressionvisitor.h"
#include "types/gostructuretype.h"
#include "items/completionitem.h"
#include "items/functionitem.h"
#include "items/importcompletionitem.h"
#include "helper.h"
#include "completiondebug.h"

using namespace KDevelop;



namespace go
{
    
CodeCompletionContext::CodeCompletionContext(const KDevelop::DUContextPointer& context,
                                             const QString& text, 
                                             const KDevelop::CursorInRevision& position, int depth): 
                                             KDevelop::CodeCompletionContext(context,
                                                                             extractLastExpression(text), position, depth), m_fullText(text)
{
}

bool CodeCompletionContext::isImportAndMemberCompletion()
{
    int pos = m_text.size() - 1;
    QChar lastChar;
    do {
        lastChar = m_text.size() > 0 ? m_text.at(pos) : QLatin1Char('\0');
        --pos;
    } while((lastChar == QLatin1Char(' ') || lastChar == QLatin1Char('\n') || lastChar == QLatin1Char('\t')) && pos >= 0);
    return lastChar == QLatin1Char('.');
}

QList< CompletionTreeItemPointer > CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    qCDebug(COMPLETION) << m_text;
    QList<CompletionTreeItemPointer> items;

    //We shouldn't need anything before last semicolon (previous statements)
    if(m_text.lastIndexOf(';') != -1)
        m_text = m_text.mid(m_text.lastIndexOf(';'));

    //"import" + [optional package alias] + [opening double quote] + [cursor at EOL]
    if(m_text.contains(QRegExp("import[^\"]*\"[^\"]*$")))
    {
        items << importCompletion();
        return items;
    }

    if(isInsideCommentOrString())
        return items;

    setTypeToMatch();
    items << functionCallTips();
    items << (isImportAndMemberCompletion() ? importAndMemberCompletion() : normalCompletion());
    return items;
}

QList< CompletionTreeItemPointer > CodeCompletionContext::normalCompletion()
{
    //all declarations
    QList<CompletionTreeItemPointer> items;
    DUChainReadLocker lock;
    auto declarations = m_duContext->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext());
    for(const QPair<Declaration*, int> &decl : declarations)
    {
        if(decl.first->topContext() != m_duContext->topContext())
            continue;
        if(decl.first->identifier() == globalImportIdentifier() || decl.first->identifier() == globalAliasIdentifier()
            || decl.first->identifier() == Identifier())
            continue;
        items << itemForDeclaration(decl);
    }
    return items;
}

QList< CompletionTreeItemPointer > CodeCompletionContext::functionCallTips()
{
    QStack<ExpressionStackEntry> stack = expressionStack(m_text);
    QList<CompletionTreeItemPointer> items;
    int depth = 1;
    bool isTopOfStack = true;
    while(!stack.empty())
    {
        ExpressionStackEntry entry = stack.pop();
        if (entry.startPosition > 0 && m_text.at(entry.startPosition - 1) == QLatin1Char('('))
        {
            DeclarationPointer function = lastDeclaration(m_text.left(entry.startPosition - 1));
            if(function && fastCast<go::GoFunctionType*>(function->abstractType().constData()))
            {
                FunctionCompletionItem* item = new FunctionCompletionItem(function, depth, entry.commas);
                depth++;
                items << CompletionTreeItemPointer(item);

                if(isTopOfStack && !m_typeMatch.singleType())
                {
                    GoFunctionType::Ptr ftype(fastCast<GoFunctionType*>(function->abstractType().constData()));
                    auto args = ftype->arguments();
                    if(args.count() != 0)
                    {
                        int argument = entry.commas >= args.count() ? args.count()-1 : entry.commas;
                        m_typeMatch.setSingleType(args.at(argument));
                    }
                }
            }
        }
        isTopOfStack = false;
    }
    return items;
}

void CodeCompletionContext::setTypeToMatch()
{
    QStack<ExpressionStackEntry> stack = expressionStack(m_text);
    if(stack.empty())
    {
        return;
    }

    ExpressionStackEntry entry = stack.pop();
    if(entry.operatorStart <= entry.startPosition)
    {
        // handle indexing a[%CURSOR]
        if(entry.startPosition > 0 && m_text.at(entry.startPosition - 1) == QLatin1Char('['))
        {
            auto declEntry = stack.pop();
            auto declText = m_text.mid(declEntry.startPosition, entry.startPosition - 1 - declEntry.startPosition);
            auto type = lastType(declText);
            if(fastCast<ArrayType*>(type.data()))
            {
                m_typeMatch.setSingleType(GoIntegralType::Ptr(new GoIntegralType(GoIntegralType::TypeInt)));
            }
            else if(auto map = fastCast<GoMapType*>(type.data()))
            {
                auto keyType = map->keyType();
                m_typeMatch.setSingleType(map->keyType());
            }
        }
        return;
    }

    auto operatorText = m_text.mid(entry.operatorStart, entry.operatorEnd-entry.operatorStart);
    if(operatorText == "=" || operatorText == ":=")
    {
        auto leftText = m_text.mid(entry.startPosition, entry.operatorStart - entry.startPosition);
        auto rightText = m_text.mid(entry.operatorEnd);
        // On left side we have comma-separated declarations while on right side we can have function calls
        // whose can have extra commas. Since entry.commas contains *all* commas in *current* expression,
        // (e.g. commas inside function calls will not be counted) use it and amount of commas on left side to determine
        // amount of commas on right side.
        auto pos = entry.commas - leftText.count(',');
        auto declarations = leftText.split(',');
        if(pos == 0 && declarations.size() > 1)
        {
            QList<KDevelop::AbstractType::Ptr> types;
            for(auto i = 0; i < declarations.size(); ++i)
            {
                types.append(lastType(declarations[i]));
            }
            m_typeMatch.setMultipleTypes(types);
        }
        if(declarations.size() > pos)
        {
            if(auto type = lastType(declarations[pos]))
            {
                m_typeMatch.setSingleType(type);
            }
        }
    }
    else if(operatorText == "<-")
    {
        auto leftText = m_text.left(entry.operatorStart);
        if(auto type = lastType(leftText))
        {
            if(auto channelType = fastCast<GoChanType*>(type.data()))
            {
                if(channelType->valueType())
                {
                    m_typeMatch.setSingleType(channelType->valueType());
                }
            }
        }
    }
    //don't show matching in var declarations e.g. "a := b"
    //and expression lists e.g. "a(), b()
    else if(operatorText != ",")
    {
        if(auto type = lastType(m_text.left(entry.operatorStart)))
        {
            m_typeMatch.setSingleType(type);
        }
    }
}

QList<CompletionTreeItemPointer> CodeCompletionContext::getImportableDeclarations(Declaration *sourceDeclaration)
{
    QList<CompletionTreeItemPointer> items;
    auto declarations = getDeclarations(sourceDeclaration->qualifiedIdentifier(), m_duContext.data());
    for(Declaration* declaration : declarations)
    {
        DUContext* context = declaration->internalContext();
        if(!context) continue;
        auto innerDeclarations = context->allDeclarations(CursorInRevision::invalid(), sourceDeclaration->topContext(), false);
        for(const QPair<Declaration*, int> innerDeclaration : innerDeclarations)
        {
            if(innerDeclaration.first == declaration)
                continue;
            QualifiedIdentifier fullname = innerDeclaration.first->qualifiedIdentifier();
            Identifier identifier = fullname.last();
            if(identifier.toString().size() <= 0)
                continue;
            //import only declarations that start with capital letter(Go language rule)
            if(m_duContext->topContext() != declaration->topContext())
                if(!identifier.toString().at(0).isLetter() || (identifier.toString().at(0) != identifier.toString().at(0).toUpper()))
                    continue;
            items << itemForDeclaration(innerDeclaration);
        }
    }
    return items;
}

QList< CompletionTreeItemPointer > CodeCompletionContext::importAndMemberCompletion()
{
    QList<CompletionTreeItemPointer> items;

    int pos = m_text.size();
    QChar lastChar;
    do {
        --pos;
        lastChar = m_text.size() > 0 ? m_text.at(pos) : QLatin1Char('\0');
    } while(lastChar != QLatin1Char('.') && lastChar != QLatin1Char('\0'));

    AbstractType::Ptr type = lastType(m_text.left(pos));

    if(type)
    {
        if(auto ptype = fastCast<PointerType*>(type.constData()))
        {
            DUChainReadLocker lock;
            if(ptype->baseType())
            {
                type = ptype->baseType();
            }
        }
        if(auto structure = fastCast<StructureType*>(type.constData()))
        {
            DUChainReadLocker lock;
            Declaration* declaration = structure->declaration(m_duContext->topContext());
            if(declaration)
            {
                items << getImportableDeclarations(declaration);
            }
        }
        if(auto structure = dynamic_cast<GoStructureType*>(type.data()))
        {
            DUContext* context = structure->context();
            DUChainReadLocker lock;

            auto declarations = context->allDeclarations(CursorInRevision::invalid(), m_duContext->topContext(), false);
            lock.unlock();
            for(const QPair<Declaration*, int> &decl : declarations)
            {
                items << itemForDeclaration(decl);
            }
        }
    }
    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::importCompletion()
{
    auto searchPaths = Helper::getSearchPaths();
    QList<CompletionTreeItemPointer> items;
    QString fullPath = m_text.mid(m_text.lastIndexOf('"')+1);

    //import "parentPackage/childPackage"
    QStringList pathChain = fullPath.split('/', QString::SkipEmptyParts);
    qCDebug(COMPLETION) << pathChain;
    for(const QString& path : searchPaths)
    {
        QDir dir(path);
        if(dir.exists())
        {
            bool isValid = true;
            for(const QString& nextDir : pathChain)
            {
                isValid = dir.cd(nextDir);
                if(!isValid)
                    break;
            }
            if(!dir.exists() || !isValid)
                continue;
            for(const QString& package : dir.entryList(QDir::Dirs))
            {
                if(package.startsWith('.'))
                    continue;
                items << CompletionTreeItemPointer(new ImportCompletionItem(package));
            }
        }
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
            if(lexer.size() > 1 && lexer.at(lexer.index()-2).kind == Parser::Token_IDENT)
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
            break;
        default:
            // The last operator of every sub-expression is stored on the stack
            // so that "A = foo." can know that attributes of foo having the same
            // type as A should be highlighted.
            qCDebug(COMPLETION) << token.kind;
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
    qCDebug(COMPLETION) << lastExpression;

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

DeclarationPointer CodeCompletionContext::lastDeclaration(const QString& expression)
{
    QStack<ExpressionStackEntry> stack = expressionStack(expression);
    QString lastExpression(expression.mid(stack.top().operatorEnd));
    qCDebug(COMPLETION) << lastExpression;

    ParseSession session(lastExpression.toUtf8(), 0, false);
    ExpressionAst* expressionAst;
    if(!session.parseExpression(&expressionAst))
        return DeclarationPointer();

    ExpressionVisitor expVisitor(&session, this->m_duContext.data());
    expVisitor.visitExpression(expressionAst);
    if(expVisitor.lastDeclaration())
        return expVisitor.lastDeclaration();

    return DeclarationPointer();
}

CompletionTreeItemPointer CodeCompletionContext::itemForDeclaration(QPair<Declaration*, int > declaration)
{
    if(declaration.first->isFunctionDeclaration())
        return CompletionTreeItemPointer(new FunctionCompletionItem(DeclarationPointer(declaration.first)));
    return CompletionTreeItemPointer(new go::CompletionItem(DeclarationPointer(declaration.first),
                                                        QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), declaration.second));
}

bool CodeCompletionContext::isInsideCommentOrString()
{
    bool inLineComment = false;
    bool inComment = false;
    bool inQuotes = false;
    bool inDoubleQuotes = false;
    bool inBackQuotes = false;
    QString text = ' ' + m_fullText;
    for(int index = 0; index < text.size()-1; ++index)
    {
        const QChar c = text.at(index);
        const QChar next = text.at(index + 1);
        if(inLineComment)
        {
            if(c == QLatin1Char('\n'))
            {
                inLineComment = false;
                continue;
            }
        }
        if(inComment)
        {
            if(c == QLatin1Char('*') && next == QLatin1Char('/'))
            {
                inComment = false;
                continue;
            }
        }
        else if(inQuotes)
        {
            if(c != QLatin1Char('\\') && next == QLatin1Char('\''))
            {
                inQuotes = false;
                continue;
            }
        }
        else if(inDoubleQuotes)
        {
            if(c != QLatin1Char('\\') && next == QLatin1Char('\"'))
            {
                inDoubleQuotes = false;
                continue;
            }
        }
        else if(inBackQuotes)
        {
            if(c != QLatin1Char('\\') && next == QLatin1Char('\`'))
            {
                inBackQuotes = false;
                continue;
            }
        }
        else
        {
            if(c == QLatin1Char('/') && next == QLatin1Char('/'))
                inLineComment = true;
            if(c == QLatin1Char('/') && next == QLatin1Char('*'))
                inComment = true;
            if(next == QLatin1Char('\''))
                inQuotes = true;
            if(next == QLatin1Char('\"'))
                inDoubleQuotes = true;
            if(next == QLatin1Char('\`'))
                inBackQuotes = true;
        }
    }
    if(inLineComment || inComment || inQuotes || inDoubleQuotes || inBackQuotes)
        return true;
    return false;
}

QString CodeCompletionContext::extractLastExpression(const QString &str)
{
    QString result;
    int prevLineEnd = str.lastIndexOf('\n');
    if(prevLineEnd != -1)
    {
        result = str.mid(prevLineEnd+1);
        if(prevLineEnd - 1 >= 0)
        {
            auto previousText = str.left(prevLineEnd);
            if(endsWithDot(previousText))
            {
                result = extractLastExpression(previousText) + "\n" + result;
            }
        }
    }
    else
    {
        result = str;
    }
    return result;
}

bool CodeCompletionContext::endsWithDot(const QString &str)
{
    auto dotPos = str.lastIndexOf('.');
    if(dotPos == -1)
    {
        return false;
    }
    for(auto i = dotPos + 1; i < str.size(); ++i)
    {
        if(!str[i].isSpace())
        {
            return false;
        }
    }
    return true;
}

}