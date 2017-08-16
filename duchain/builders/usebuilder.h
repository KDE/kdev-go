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

#ifndef GOLANGUSEBUILDER_H
#define GOLANGUSEBUILDER_H

#include <language/duchain/builders/abstractusebuilder.h>

#include "parser/goast.h"
#include "contextbuilder.h"

#include <QStack>

namespace go 
{
    
typedef KDevelop::AbstractUseBuilder<AstNode, IdentifierAst, ContextBuilder> UseBuilderBase;

class KDEVGODUCHAIN_EXPORT UseBuilder : public UseBuilderBase
{
public:
    UseBuilder(ParseSession* session);
    
    
    //virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url, AstNode* node,
    //KDevelop::ReferencedTopDUContext updateContext = KDevelop::ReferencedTopDUContext());
    
    void visitPrimaryExpr(go::PrimaryExprAst* node) override;
    void visitTypeName(go::TypeNameAst* node) override;
    void visitBlock(go::BlockAst* node) override;
    void visitMethodDeclaration(go::MethodDeclarationAst* node) override;
    void visitShortVarDecl(go::ShortVarDeclAst* node) override;

private:
    void createUseInDeclaration(IdentifierAst *idNode);

    QStack<KDevelop::AbstractType::Ptr> m_types;
};
    
}

#endif
