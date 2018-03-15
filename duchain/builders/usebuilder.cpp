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

    auto id = identifierForNode(node->id);
    if(node->literalValue)
    {
        createUseForField(node->literalValue->element, id, context);
        if(node->literalValue->elementsSequence)
        {
            auto iter = node->literalValue->elementsSequence->front(), end = iter;
            do
            {
                auto element = iter->element;
                createUseForField(element, id, context);
                iter = iter->next;
            }
            while (iter != end);
        }
    }
    else
    {
        auto primaryExprResolveNode = node->primaryExprResolve;
        while(primaryExprResolveNode)
        {
            if(primaryExprResolveNode->selector)
            {
                id = id + identifierForNode(primaryExprResolveNode->selector);
            }
            if(primaryExprResolveNode->literalValue)
            {
                createUseForField(primaryExprResolveNode->literalValue->element, id, currentContext());
                if(primaryExprResolveNode->literalValue->elementsSequence)
                {
                    auto iter = primaryExprResolveNode->literalValue->elementsSequence->front(), end = iter;
                    do
                    {
                        auto element = iter->element;
                        createUseForField(element, id, currentContext());
                        iter = iter->next;
                    }
                    while (iter != end);
                }
            }
            primaryExprResolveNode = primaryExprResolveNode->primaryExprResolve;
        }
    }

    //build uses in subexpressions
    ContextBuilder::visitPrimaryExpr(node);
}

void UseBuilder::createUseForField(const ElementAst *fieldElement, const QualifiedIdentifier &typeId, DUContext *context)
{
    if(fieldElement)
    {
        if(auto keyOrValue = fieldElement->keyOrValue)
        {
            if(auto indexOrValue = keyOrValue->indexOrValue)
            {
                if(auto unaryExpression = indexOrValue->unaryExpression)
                {
                    if(auto primaryExpr = unaryExpression->primaryExpr)
                    {
                        if(auto idNode = primaryExpr->id)
                        {
                            auto identifier = typeId + identifierForNode(idNode);
                            if(auto declaration = getDeclaration(identifier, context))
                            {
                                newUse(idNode, declaration);
                            }
                        }
                    }
                }
            }
        }
    }
}

void UseBuilder::visitBlock(BlockAst* node)
{
    ContextBuilder::visitBlock(node);
}

void UseBuilder::visitMethodDeclaration(go::MethodDeclarationAst* node)
{
    IdentifierAst *typeIdentifierNode;
    if(node->methodRecv->type)
    {
        typeIdentifierNode = node->methodRecv->type;
    }
    else if(node->methodRecv->nameOrType)
    {
        typeIdentifierNode = node->methodRecv->nameOrType;
    }
    else
    {
        typeIdentifierNode = node->methodRecv->ptype;
    }

    QualifiedIdentifier id(identifierForNode(typeIdentifierNode));
    DUContext* context;
    {
        DUChainReadLocker lock;
        context = currentContext()->findContextIncluding(editorFindRange(typeIdentifierNode, 0));
    }
    DeclarationPointer declaration = getTypeDeclaration(id, context);
    if(declaration)
    {
        newUse(typeIdentifierNode, declaration);
    }
    ContextBuilder::visitMethodDeclaration(node);
}

void UseBuilder::visitShortVarDecl(go::ShortVarDeclAst *node)
{
    createUseInDeclaration(node->id);
    if(node->idList)
    {
        auto iter = node->idList->idSequence->front(), end = iter;
        do
        {
            createUseInDeclaration(iter->element);
            iter = iter->next;
        }
        while (iter != end);
    }
    ContextBuilder::visitShortVarDecl(node);
}

void UseBuilder::createUseInDeclaration(IdentifierAst *idNode)
{
    auto identifier = identifierForNode(idNode);
    auto declaration = getDeclaration(identifier, currentContext(), false);
    auto wasDeclaredInCurrentContext = declaration && declaration.data()->range() != editorFindRange(idNode, 0);
    if(identifier.toString() != "_" && wasDeclaredInCurrentContext)
    {
        newUse(idNode, declaration);
    }
}

void UseBuilder::visitLiteralValue(go::LiteralValueAst *node)
{
    ContextBuilder::visitLiteralValue(node);
}

}
