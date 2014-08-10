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

#ifndef KDEVGOLANGDECLBUILDER_H
#define KDEVGOLANGDECLBUILDER_H

#include <language/duchain/builders/abstractdeclarationbuilder.h>
#include <language/duchain/builders/abstracttypebuilder.h>

#include "goduchainexport.h"
#include "contextbuilder.h"
#include "parser/parsesession.h"
#include "parser/goast.h"
#include "types/gofunctiontype.h"
#include "declarations/functiondeclaration.h"



typedef KDevelop::AbstractTypeBuilder<go::AstNode, go::IdentifierAst, ContextBuilder> TypeBuilder;
typedef KDevelop::AbstractDeclarationBuilder<go::AstNode, go::IdentifierAst, TypeBuilder> DeclarationBuilderBase;

class KDEVGODUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
public:
    DeclarationBuilder(ParseSession* session, bool forExport);
    
    virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url,
                                                   go::AstNode* node,
                                                   KDevelop::ReferencedTopDUContext updateContext = KDevelop::ReferencedTopDUContext());
    virtual void startVisiting(go::AstNode* node);
    
    //virtual void visitVarDecl(go::VarDeclAst* node);
    virtual void visitVarSpec(go::VarSpecAst* node);
    //virtual void visitType(go::TypeAst* node);
    virtual void visitTypeName(go::TypeNameAst* node);
    virtual void visitArrayOrSliceType(go::ArrayOrSliceTypeAst* node);
    virtual void visitFunctionType(go::FunctionTypeAst* node);
    //virtual void visitSignature(go::SignatureAst* node);
    //virtual void visitParameters(go::ParametersAst* node);
    virtual void visitParameter(go::ParameterAst* node);
    virtual void visitFuncDeclaration(go::FuncDeclarationAst* node);
    virtual void visitMethodDeclaration(go::MethodDeclarationAst* node);
    virtual void visitPointerType(go::PointerTypeAst* node);
    virtual void visitStructType(go::StructTypeAst* node);
    virtual void visitFieldDecl(go::FieldDeclAst* node);
    virtual void visitInterfaceType(go::InterfaceTypeAst* node);
    virtual void visitMethodSpec(go::MethodSpecAst* node);
    virtual void visitMapType(go::MapTypeAst* node);
    virtual void visitChanType(go::ChanTypeAst* node);
    
    virtual void visitTypeSpec(go::TypeSpecAst* node);
    
    virtual void visitImportSpec(go::ImportSpecAst* node);
    virtual void visitSourceFile(go::SourceFileAst* node);
    virtual void visitShortVarDecl(go::ShortVarDeclAst* node);
  
    
    /*struct GoImport{
	GoImport(bool anon, KDevelop::TopDUContext* ctx) : anonymous(anon), context(ctx) {}
	bool anonymous;
	KDevelop::TopDUContext* context;
    };*/
    
    /**
     * This function exists because ExpressionVisitor needs to evaluate types of nodes when it encounters 
     * builtin functions like "make". I guess there should be TypeBuilder class for that, but when building
     * struct types we also need to open declarations, so it's a bit tricky to separate DeclarationBuilder and TypeBuilder
     * Think how to do it properly.
     */
    AbstractType::Ptr buildType(go::TypeAst* node);
    AbstractType::Ptr buildType(go::IdentifierAst* node, go::IdentifierAst* fullname=0);
    
private:
    go::GoFunctionDeclaration* parseSignature(go::SignatureAst* node, bool declareParameters, go::IdentifierAst* name=0);
    void parseParameters(go::ParametersAst* node, bool parseParameters=true, bool declareParameters=false);
    
    void addArgumentHelper(go::GoFunctionType::Ptr function, KDevelop::AbstractType::Ptr argument, bool parseArguments);
    go::TypeNameAst* typeNameFromIdentifier(go::IdentifierAst* id, go::IdentifierAst* fullname=0);
    void declareParameter(go::IdentifierAst* name, const AbstractType::Ptr& type);
    void declareVariables(go::IdentifierAst* id, go::IdListAst* idList, go::ExpressionAst* expression, go::ExpressionListAst* expressionList);
    
    void importThisPackage();
    bool m_export;
    
    //QHash<QString, TopDUContext*> m_anonymous_imports;
    
    QualifiedIdentifier m_contextIdentifier;
    bool m_preBuilding;
};

#endif