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

#ifndef GOLANGEXPRVISITOR_H
#define GOLANGEXPRVISITOR_H

#include "parser/godefaultvisitor.h"
#include "parser/goast.h"
#include "declarationbuilder.h"
#include "goduchainexport.h"

namespace go 
{

class KDEVGODUCHAIN_EXPORT ExpressionVisitor : public DefaultVisitor
{
public:
    ExpressionVisitor(ParseSession* session, KDevelop::DUContext* context, DeclarationBuilder* builder=0);

    QList<AbstractType::Ptr> lastTypes();

    virtual void visitExpression(go::ExpressionAst* node);
    virtual void visitUnaryExpression(go::UnaryExpressionAst* node);
    virtual void visitPrimaryExpr(go::PrimaryExprAst* node);
    virtual void visitBasicLit(go::BasicLitAst* node);
    virtual void visitPrimaryExprResolve(go::PrimaryExprResolveAst* node);
    virtual void visitCallOrBuiltinParam(go::CallOrBuiltinParamAst* node);
    virtual void visitCallParam(go::CallParamAst* node);
    
    virtual void visitBlock(go::BlockAst* node);
    
    QList<DeclarationPointer> allDeclarations();
    QList<IdentifierAst*> allIds();
 
private:
    void pushType(AbstractType::Ptr type);
 
    QList<AbstractType::Ptr> popTypes();

    void addType(AbstractType::Ptr type);

    void pushUse(IdentifierAst* node, Declaration* declaration);

    AbstractType::Ptr resolveTypeAlias(AbstractType::Ptr type);

    QualifiedIdentifier identifierForNode(IdentifierAst* node);

    bool handleComplexLiterals(PrimaryExprResolveAst* node, Declaration* decl);
    bool handleBuiltinFunction(PrimaryExprAst* node);

    DeclarationBuilder* m_builder;
    ParseSession* m_session;
    DUContext* m_context;
    //QList<AbstractType::Ptr> m_types;

private:
    QList<IdentifierAst*> m_ids;
    QList<DeclarationPointer> m_declarations;
    QList<AbstractType::Ptr> m_types;

};

}
#endif