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
#include "contextbuilder.h"
#include "duchain/goduchainexport.h"
#include "duchain/declarations/functiondeclaration.h"
#include "duchain/types/gofunctiontype.h"

namespace go
{

typedef KDevelop::AbstractTypeBuilder<AstNode, IdentifierAst, ContextBuilder> TypeBuilderBase;

class KDEVGODUCHAIN_EXPORT TypeBuilder : public TypeBuilderBase
{
public:
    virtual void visitTypeName(go::TypeNameAst* node);
    virtual void visitArrayOrSliceType(go::ArrayOrSliceTypeAst* node);
    virtual void visitPointerType(go::PointerTypeAst* node);
    virtual void visitStructType(go::StructTypeAst* node);
    virtual void visitFieldDecl(go::FieldDeclAst* node);
    virtual void visitInterfaceType(go::InterfaceTypeAst* node);
    virtual void visitMethodSpec(go::MethodSpecAst* node);
    virtual void visitMapType(go::MapTypeAst* node);
    virtual void visitChanType(go::ChanTypeAst* node);
    virtual void visitFunctionType(go::FunctionTypeAst* node);
    virtual void visitParameter(go::ParameterAst* node);

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
    virtual GoFunctionDeclaration* declareFunction(go::IdentifierAst* id, const GoFunctionType::Ptr& type,
                                                   DUContext* paramContext, DUContext* retparamContext) = 0;



    /**
     * opens GoFunctionType, parses it's parameters and return declaration if @param declareParameters is true.
     **/
    go::GoFunctionDeclaration* parseSignature(go::SignatureAst* node, bool declareParameters, go::IdentifierAst* name=0);

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