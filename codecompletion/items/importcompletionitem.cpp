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

#include "importcompletionitem.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <language/codecompletion/codecompletionmodel.h>

namespace go
{

ImportCompletionItem::ImportCompletionItem(QString packagename): KDevelop::NormalDeclarationCompletionItem(KDevelop::DeclarationPointer(),
                                                                  QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), 0), m_packageName(packagename)
{
}

QVariant ImportCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{
    if(role == Qt::DisplayRole && (index.column() == CodeCompletionModel::Name))
        return m_packageName;
    if(role == Qt::DisplayRole && (index.column() == CodeCompletionModel::Prefix))
        return "package";
    return NormalDeclarationCompletionItem::data(index, role, model);
}

void ImportCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
    KTextEditor::Document* document = view->document();
    KTextEditor::Range checkSuffix(word.end().line(), word.end().column(), word.end().line(), document->lineLength(word.end().line()));
    QString suffix = "\"";
    if(document->text(checkSuffix).startsWith('"'))
        suffix.clear();
    document->replaceText(word, m_packageName + suffix);
}

}
