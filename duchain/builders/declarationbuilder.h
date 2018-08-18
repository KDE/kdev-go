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
#include <declarations/functiondefinition.h>

#include "duchain/kdevgoduchain_export.h"
#include "contextbuilder.h"
#include "typebuilder.h"
#include "parser/parsesession.h"
#include "parser/goast.h"


typedef KDevelop::AbstractDeclarationBuilder<go::AstNode, go::IdentifierAst, go::TypeBuilder> DeclarationBuilderBase;

class KDEVGODUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
public:
    DeclarationBuilder(ParseSession* session, bool forExport);
    
    KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url,
                                                   go::AstNode* node,
                                                   const KDevelop::ReferencedTopDUContext& updateContext = KDevelop::ReferencedTopDUContext()) override;
    void startVisiting(go::AstNode* node) override;

    void visitVarSpec(go::VarSpecAst* node) override;
    void visitShortVarDecl(go::ShortVarDeclAst* node) override;
    void visitConstSpec(go::ConstSpecAst* node) override;
    void visitConstDecl(go::ConstDeclAst* node) override;
    void visitFuncDeclaration(go::FuncDeclarationAst* node) override;
    void visitMethodDeclaration(go::MethodDeclarationAst* node) override;
    void visitTypeSpec(go::TypeSpecAst* node) override;
    void visitImportSpec(go::ImportSpecAst* node) override;
    void visitSourceFile(go::SourceFileAst* node) override;
    void visitForStmt(go::ForStmtAst* node) override;
    void visitSwitchStmt(go::SwitchStmtAst* node) override;
    void visitTypeCaseClause(go::TypeCaseClauseAst* node) override;
    void visitExprCaseClause(go::ExprCaseClauseAst* node) override;
    void visitPrimaryExpr(go::PrimaryExprAst *node) override;

    /**
     * this handles variable declaration in select statements, e.g.
     * select { case i := <-mychan: bla bla...  }
     * NOTE: right hand side expression must be a receive operator, returning two values */
    void visitCommCase(go::CommCaseAst* node) override;
    void visitCommClause(go::CommClauseAst* node) override;

    void visitTypeDecl(go::TypeDeclAst* node) override;

    /**
     * A shortcut for ExpressionVisitor to build function type
     **/
    go::GoFunctionDeclaration* buildFunction(go::SignatureAst* node, go::BlockAst* block = nullptr, go::IdentifierAst* name = nullptr, const QByteArray& comment = {});

    go::GoFunctionDefinition* buildMethod(go::SignatureAst* node, go::BlockAst* block = nullptr, go::IdentifierAst* name = nullptr,
                                    go::GoFunctionDeclaration* pDeclaration = nullptr, const QByteArray &array = {}, const QualifiedIdentifier &identifier = {});

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
    void declareVariable(go::IdentifierAst* id, const AbstractType::Ptr& type) override;

    /**
     * Declares GoFunction and assigns contexts to it.
     * Called from typebuilder when building functions and methods
     **/
    go::GoFunctionDeclaration* declareFunction(go::IdentifierAst* id, const go::GoFunctionType::Ptr& type,
                                                       DUContext* paramContext, DUContext* retparamContext, const QByteArray& comment = {}, DUContext* bodyContext = nullptr) override;

    go::GoFunctionDefinition* declareMethod(go::IdentifierAst* id, const go::GoFunctionType::Ptr& type,
                                              DUContext* paramContext, DUContext* retparamContext, const QByteArray& comment=QByteArray(),
                                              DUContext* bodyContext = nullptr, go::GoFunctionDeclaration* declaration = nullptr, const QualifiedIdentifier &identifier = {});

    void importThisPackage();
    void importBuiltins();
    bool m_export;

    //QHash<QString, TopDUContext*> m_anonymous_imports;

    bool m_preBuilding;
    QList<AbstractType::Ptr> m_constAutoTypes;
    QualifiedIdentifier m_thisPackage;
    QualifiedIdentifier m_switchTypeVariable;
    QByteArray m_lastTypeComment, m_lastConstComment;
};

#endif
