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

#include "usebuilder.h"

#include "expressionvisitor.h"
#include "helper.h"
#include "duchaindebug.h"

using namespace KDevelop;

namespace go 
{
    
UseBuilder::UseBuilder(ParseSession* session)
{
    setParseSession(session);
}

/*ReferencedTopDUContext UseBuilder::build(const IndexedString& url, AstNode* node, ReferencedTopDUContext updateContext)
{
    qCDebug(DUCHAIN) << "Uses builder run";
    return UseBuilderBase::build(url, node, updateContext);
}*/

void UseBuilder::visitTypeName(TypeNameAst* node)
{
    QualifiedIdentifier id(identifierForNode(node->name));
    if(node->type_resolve->fullName)
	id.push(identifierForNode(node->type_resolve->fullName));
    DUContext* context;
    {
	DUChainReadLocker lock;
	context = currentContext()->findContextIncluding(editorFindRange(node, 0));
    }
    DeclarationPointer decl = getTypeDeclaration(id, context);
    if(decl)
    {
	newUse(node, decl);
    }
}


void UseBuilder::visitPrimaryExpr(PrimaryExprAst* node)
{
    DUContext* context;
    {
	DUChainReadLocker lock;
	//context = currentContext()->findContextAt(editorFindRange(node, 0).start);
	context = currentContext()->findContextIncluding(editorFindRange(node, 0));
    }
    if(!context) return;
    ExpressionVisitor visitor(m_session, context);
    visitor.visitPrimaryExpr(node);
    auto ids = visitor.allIds();
    auto decls = visitor.allDeclarations();
    if(ids.size() != decls.size())
	return;
    for(int i=0; i<ids.size(); ++i)
	newUse(ids.at(i), decls.at(i));
    //build uses in subexpressions
    go::DefaultVisitor::visitPrimaryExpr(node);
}

void UseBuilder::visitBlock(BlockAst* node)
{
    go::DefaultVisitor::visitBlock(node);
}

}
