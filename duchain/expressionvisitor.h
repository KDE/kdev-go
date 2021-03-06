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
#include "builders/declarationbuilder.h"
#include "kdevgoduchain_export.h"

namespace go 
{

class KDEVGODUCHAIN_EXPORT ExpressionVisitor : public DefaultVisitor
{
public:
    ExpressionVisitor(ParseSession* session, KDevelop::DUContext* context, DeclarationBuilder* builder=0);

    QList<AbstractType::Ptr> lastTypes();

    void visitExpression(go::ExpressionAst* node) override;
    void visitUnaryExpression(go::UnaryExpressionAst* node) override;
    void visitPrimaryExpr(go::PrimaryExprAst* node) override;
    void visitBasicLit(go::BasicLitAst* node) override;
    void visitPrimaryExprResolve(go::PrimaryExprResolveAst* node) override;
    void visitCallOrBuiltinParam(go::CallOrBuiltinParamAst* node) override;
    void visitCallParam(go::CallParamAst* node) override;
    void visitTypeName(go::TypeNameAst* node) override;
    void visitStructType(go::StructTypeAst* node) override;
    void visitMapType(go::MapTypeAst* node) override;
    void visitPointerType(go::PointerTypeAst* node) override;
    void visitInterfaceType(go::InterfaceTypeAst* node) override;
    void visitChanType(go::ChanTypeAst* node) override;
    void visitParenType(go::ParenTypeAst* node) override;

    void visitBlock(go::BlockAst* node) override;
    /**
     * Visits range expression.
     * There isn't an actual range expression rule in grammar, so you have to manually
     * find range token and call this function on subsequent expression.
     **/
    void visitRangeClause(go::ExpressionAst* node);

    /**
     * Delete all stored types and uses
     */
    void clearAll();

    QList<DeclarationPointer> allDeclarations();
    QList<IdentifierAst*> allIds();

    /**
     * TODO Currently only codecompletion needs declarations
     **/
    DeclarationPointer lastDeclaration() { return m_declaration; }
 
private:
    void pushType(AbstractType::Ptr type);
 
    QList<AbstractType::Ptr> popTypes();

    void addType(AbstractType::Ptr type);

    void pushUse(IdentifierAst* node, Declaration* declaration);

    AbstractType::Ptr resolveTypeAlias(AbstractType::Ptr type);

    QualifiedIdentifier identifierForNode(IdentifierAst* node);

    /**
     * Converts complex type literals or conversions (imp.imptype{} or imp.imptype(1))
     * to identified types.
     **/
    bool handleComplexLiteralsAndConversions(PrimaryExprResolveAst* node, Declaration* decl);

    /**
     * Handle 'make', 'append' and 'new' methods since they don't follow go syntax
     * and cannot be handled by builtins.go
     **/
    bool handleBuiltinFunction(PrimaryExprAst* node);

    /**
     * Handles stuff like basic literals(3.1415, 'strings', ...), type conversions( interface{}(variable) ... ),
     * anonymous functions ( func() { callMe(); } ) and so on.
     **/
    void handleLiteralsAndConversions(PrimaryExprAst* node);

    ParseSession* m_session;
    DUContext* m_context;
    DeclarationBuilder* m_builder;
    //QList<AbstractType::Ptr> m_types;

private:
    QList<IdentifierAst*> m_ids;
    QList<DeclarationPointer> m_declarations;
    QList<AbstractType::Ptr> m_types;
    DeclarationPointer m_declaration;

};

}
#endif