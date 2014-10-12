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

#include "functionitem.h"

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <language/duchain/declaration.h>

#include "types/gofunctiontype.h"

namespace go
{

FunctionCompletionItem::FunctionCompletionItem(DeclarationPointer decl,
                                               KSharedPtr< CodeCompletionContext > context, int inheritanceDepth): NormalDeclarationCompletionItem(decl, context, inheritanceDepth)
{

}

void FunctionCompletionItem::executed(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    QString suffix = "()";
    KTextEditor::Range checkSuffix(word.end().line(), word.end().column(), word.end().line(), document->lineLength(word.end().line()));
    if(document->text(checkSuffix).startsWith('('))
    {
        suffix.clear();
    }
    document->replaceText(word, declaration()->identifier().toString() + suffix);
    AbstractType::Ptr type = declaration()->abstractType();
    if(fastCast<GoFunctionType*>(type.constData()))
    {
        GoFunctionType* ftype = fastCast<GoFunctionType*>(type.constData());
        //put cursor inside parentheses if function takes arguments
        if(ftype->arguments().size() > 0)
            document->activeView()->setCursorPosition(KTextEditor::Cursor(word.end().line(), word.end().column() + 1));
    }
}


}
