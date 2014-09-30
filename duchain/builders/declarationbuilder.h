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

#include "duchain/goduchainexport.h"
#include "contextbuilder.h"
#include "typebuilder.h"
#include "parser/parsesession.h"
#include "parser/goast.h"


typedef KDevelop::AbstractDeclarationBuilder<go::AstNode, go::IdentifierAst, go::TypeBuilder> DeclarationBuilderBase;

class KDEVGODUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
public:
    DeclarationBuilder(ParseSession* session, bool forExport);
    
    virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url,
                                                   go::AstNode* node,
                                                   KDevelop::ReferencedTopDUContext updateContext = KDevelop::ReferencedTopDUContext());
    virtual void startVisiting(go::AstNode* node);

    virtual void visitVarSpec(go::VarSpecAst* node);
    virtual void visitShortVarDecl(go::ShortVarDeclAst* node);
    virtual void visitConstSpec(go::ConstSpecAst* node);
    virtual void visitConstDecl(go::ConstDeclAst* node);
    virtual void visitFuncDeclaration(go::FuncDeclarationAst* node);
    virtual void visitMethodDeclaration(go::MethodDeclarationAst* node);
    virtual void visitTypeSpec(go::TypeSpecAst* node);
    virtual void visitImportSpec(go::ImportSpecAst* node);
    virtual void visitSourceFile(go::SourceFileAst* node);
    virtual void visitForStmt(go::ForStmtAst* node);
    virtual void visitSwitchStmt(go::SwitchStmtAst* node);
    virtual void visitTypeCaseClause(go::TypeCaseClauseAst* node);
    virtual void visitExprCaseClause(go::ExprCaseClauseAst* node);
  
    
    /*struct GoImport{
	GoImport(bool anon, KDevelop::TopDUContext* ctx) : anonymous(anon), context(ctx) {}
	bool anonymous;
	KDevelop::TopDUContext* context;
    };*/

private:

    /**
     * Deduces types of expression with ExpressionVisitor and declares variables
     * from idList with respective types. If there is a single expression, returning multiple types
     * idList will get assigned those types. Otherwise we get only first type no matter how many of them
     * expression returns.(I believe this is how it works in Go, correct it if I'm wrong)
     * @param declareConstant whether to declare usual variables or constants
     */
    void declareVariables(go::IdentifierAst* id, go::IdListAst* idList, go::ExpressionAst* expression,
			    go::ExpressionListAst* expressionList, bool declareConstant);
    /**
     * declares variables or constants with names from id and idList of type type.
     */
    void declareVariablesWithType(go::IdentifierAst* id, go::IdListAst* idList, go::TypeAst* type, bool declareConstant);

    /**
     * Declares variable with identifier @param id of type @param type
     **/
    virtual void declareVariable(go::IdentifierAst* id, const AbstractType::Ptr& type) override;

    /**
     * Declares GoFunction and assigns contexts to it.
     * Called from typebuilder when building functions and methods
     **/
    virtual go::GoFunctionDeclaration* declareFunction(go::IdentifierAst* id, const go::GoFunctionType::Ptr& type, DUContext* paramContext, DUContext* retparamContext) override;

    void importThisPackage();
    bool m_export;
    
    //QHash<QString, TopDUContext*> m_anonymous_imports;
    
    bool m_preBuilding;
    QList<AbstractType::Ptr> m_constAutoTypes;
    QualifiedIdentifier m_thisPackage;
    QualifiedIdentifier m_switchTypeVariable;
};

#endif