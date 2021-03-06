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

#ifndef TYPEBUILDER_H
#define TYPEBUILDER_H

#include <language/duchain/builders/abstracttypebuilder.h>
#include <language/duchain/functiondefinition.h>
#include <declarations/functiondefinition.h>
#include "contextbuilder.h"
#include "duchain/kdevgoduchain_export.h"
#include "duchain/declarations/functiondeclaration.h"
#include "duchain/types/gofunctiontype.h"

namespace go
{

typedef KDevelop::AbstractTypeBuilder<AstNode, IdentifierAst, ContextBuilder> TypeBuilderBase;

class KDEVGODUCHAIN_EXPORT TypeBuilder : public TypeBuilderBase
{
public:
    void visitTypeName(go::TypeNameAst* node) override;
    void visitArrayOrSliceType(go::ArrayOrSliceTypeAst* node) override;
    void visitPointerType(go::PointerTypeAst* node) override;
    void visitStructType(go::StructTypeAst* node) override;
    void visitFieldDecl(go::FieldDeclAst* node) override;
    void visitInterfaceType(go::InterfaceTypeAst* node) override;
    void visitMethodSpec(go::MethodSpecAst* node) override;
    void visitMapType(go::MapTypeAst* node) override;
    void visitChanType(go::ChanTypeAst* node) override;
    void visitFunctionType(go::FunctionTypeAst* node) override;
    void visitParameter(go::ParameterAst* node) override;

    /**
     * When building named types we often have IdentifierAst instead of TypeNameAst,
     * so it makes sense to have this convenience function
     **/
    void buildTypeName(go::IdentifierAst* typeName, go::IdentifierAst* fullName = 0);

    /**
     * Used by external classes like ExpressionVisitor after building a type.
     */
    AbstractType::Ptr getLastType() { return lastType(); }

protected:

    //when building some types we need to open declarations
    //so next methods are placeholders for that, which will be implemented in DeclarationBuilder
    //that way we can keep type building logic in TypeBuilder

    /**
     * declared here as pure virtual so we can use that when building functions, structs and interfaces.
     **/
    virtual void declareVariable(go::IdentifierAst* id, const KDevelop::AbstractType::Ptr& type) = 0;

    /**
     * declared here as pure virtual so we can use that when building functions
     **/
    virtual go::GoFunctionDeclaration* declareFunction(go::IdentifierAst* id, const GoFunctionType::Ptr& type,
                                                       DUContext* paramContext, DUContext* retparamContext,
                                                       const QByteArray& comment = {}, DUContext* bodyContext = nullptr) = 0;

    /**
     * opens GoFunctionType, parses it's parameters and return declaration if @param declareParameters is true.
     **/
    go::GoFunctionType::Ptr parseSignature(go::SignatureAst *node, bool declareParameters, DUContext **parametersContext = nullptr,
                                               DUContext **returnArgsContext = nullptr, const QualifiedIdentifier &identifier = {},
                                               const QByteArray &comment = {});

    /**
     * Convenience function that parses function parameters.
     * @param parseParemeters if true - add parameter to function arguments, otherwise add it to return params
     * @param declareParameters open parameter declarations if true
     **/
    void parseParameters(go::ParametersAst* node, bool parseParameters=true, bool declareParameters=false);

    /**
     * Convenience function that adds argument to function params or output params
     **/
    void addArgumentHelper(go::GoFunctionType::Ptr function, KDevelop::AbstractType::Ptr argument, bool parseArguments);

    KDevelop::QualifiedIdentifier m_contextIdentifier;

};

}

#endif