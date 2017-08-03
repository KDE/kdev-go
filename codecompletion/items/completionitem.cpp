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

#include "completionitem.h"

#include <language/duchain/declaration.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainlock.h>

#include "context.h"
#include "types/gofunctiontype.h"

namespace go
{

CompletionItem::CompletionItem(KDevelop::DeclarationPointer decl, QExplicitlySharedDataPointer< KDevelop::CodeCompletionContext > context, int inheritanceDepth):
    NormalDeclarationCompletionItem(decl, QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), 0),
    m_prefix("")
{
    DUChainReadLocker lock;
    if(!decl)
        return;
    //NormalDeclarationCompletionItem fails to get a meaningful prefix in these cases
    if(decl->abstractType() && decl->abstractType()->whichType() == KDevelop::AbstractType::TypeFunction)
        m_prefix = decl->abstractType()->toString();
    if(decl->kind() == KDevelop::Declaration::Import || decl->kind() == KDevelop::Declaration::NamespaceAlias)
        m_prefix = "namespace";
}

QVariant CompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{
    switch(role)
    {
        case Qt::DisplayRole:
        {
            if (index.column() == CodeCompletionModel::Prefix && m_prefix != "") {
                return m_prefix;
            }
            break;
        }
        case CodeCompletionModel::BestMatchesCount:
            return 5;
        case CodeCompletionModel::MatchQuality:
        {
            if(!declaration())
                return QVariant();
            //type aliases are actually different types
            if(declaration()->isTypeAlias())
                return QVariant();
            auto codeCompletionContext = static_cast<go::CodeCompletionContext*>(model->completionContext().data());
            if(!codeCompletionContext->typeToMatch().singleType())
            {
                return QVariant();
            }
            AbstractType::Ptr typeToMatch = codeCompletionContext->typeToMatch().singleType();
            if(!typeToMatch)
                return QVariant();
            AbstractType::Ptr declType = declaration()->abstractType();
            if (!declType)
                return QVariant();
            //ignore constants
            typeToMatch->setModifiers(AbstractType::NoModifiers);
            declType->setModifiers(AbstractType::NoModifiers);
            if(declType->equals(typeToMatch.constData()))
            {
                return 10;
            }
            else if(declType->whichType() == AbstractType::TypeFunction)
            {
                GoFunctionType* function = fastCast<GoFunctionType*>(declType.constData());
                auto args = function->returnArguments();
                if(args.size() != 0)
                {
                    auto multipleTypesMatch = codeCompletionContext->typeToMatch().multipleTypes();
                    if(args.size() == multipleTypesMatch.size())
                    {
                        bool allTypesMatches = true;
                        for(auto pos = 0; pos < args.size(); ++pos)
                        {
                            auto expected = multipleTypesMatch[pos];
                            auto actual = args[pos];
                            expected->setModifiers(AbstractType::NoModifiers);
                            actual->setModifiers(AbstractType::NoModifiers);
                            if(!actual->equals(expected.constData()) && expected->toString() != "_")
                            {
                                allTypesMatches = false;
                                break;
                            }
                        }
                        if(allTypesMatches)
                        {
                            return 20;
                        }
                    }

                    AbstractType::Ptr first = args.first();
                    first->setModifiers(AbstractType::NoModifiers);
                    if(args.size() == 1 && first->equals(typeToMatch.constData()))
                    {
                        return 10;
                    }
                }
            }
            else
            {
                return QVariant();
            }
        }
    }
    return NormalDeclarationCompletionItem::data(index, role, model);
}
}
