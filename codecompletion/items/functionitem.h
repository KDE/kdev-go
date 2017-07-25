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

#ifndef GOLANGFUNCTIONITEM_H
#define GOLANGFUNCTIONITEM_H

#include "completionitem.h"

using namespace KDevelop;

namespace go
{

class FunctionCompletionItem : public CompletionItem
{
public:
    FunctionCompletionItem(KDevelop::DeclarationPointer declaration = KDevelop::DeclarationPointer(),
                                    int depth=0, int atArgument=-1);


    virtual void executed(KTextEditor::View* view, const KTextEditor::Range& word) override;
    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;
    virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const override;
    virtual int argumentHintDepth() const override;
    virtual int inheritanceDepth() const override;

private:
    int m_depth;
    int m_atArgument;
    int m_currentArgStart;
    int m_currentArgEnd;
    QString m_prefix;
    QString m_arguments;
};

}

#endif
