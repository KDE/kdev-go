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

#ifndef GOLANGCOMPLETIONCONTEXT_H
#define GOLANGCOMPLETIONCONTEXT_H

#include <language/codecompletion/codecompletioncontext.h>
#include <language/codecompletion/codecompletionitem.h>

#include "kdevgocompletion_export.h"
#include <QStack>
#include <language/duchain/declaration.h>

namespace go
{
    
class KDEVGOCOMPLETION_EXPORT CodeCompletionContext : public KDevelop::CodeCompletionContext
{
public:
    CodeCompletionContext(const KDevelop::DUContextPointer& context, const QString& text,
                          const KDevelop::CursorInRevision& position, int depth = 0);

    virtual QList<KDevelop::CompletionTreeItemPointer> completionItems(bool& abort, bool fullCompletion = true);

    KDevelop::AbstractType::Ptr typeToMatch() { return m_typeToMatch; }

private:
    //See QmlJS plugin completion for details
    struct ExpressionStackEntry {
        int startPosition;
        int operatorStart;
        int operatorEnd;
        int commas;
    };
    
    QStack<ExpressionStackEntry> expressionStack(const QString& expression);
    
    KDevelop::AbstractType::Ptr lastType(const QString& expression);

    KDevelop::DeclarationPointer lastDeclaration(const QString& expression);
    
    QList<KDevelop::CompletionTreeItemPointer> getImportableDeclarations(KDevelop::Declaration *sourceDeclaration);

    QList<KDevelop::CompletionTreeItemPointer> importAndMemberCompletion();

    QList<KDevelop::CompletionTreeItemPointer> normalCompletion();

    /**
     * Creates FunctionCallTips and sets m_typeToMatch
     **/
    QList<KDevelop::CompletionTreeItemPointer> functionCallTips();

    QList<KDevelop::CompletionTreeItemPointer> importCompletion();

    /**
     * Return completion item for declaration.
     **/
    KDevelop::CompletionTreeItemPointer itemForDeclaration(QPair<KDevelop::Declaration*, int> declaration);

    /**
     * returns true if cursor is in comment and completion is not needed
     **/
    bool isInsideCommentOrString();

    KDevelop::AbstractType::Ptr m_typeToMatch;
    QString m_fullText;
};
}

#endif