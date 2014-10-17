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
#include <language/codecompletion/codecompletionmodel.h>

#include "types/gofunctiontype.h"
#include "declarations/functiondeclaration.h"

namespace go
{

FunctionCompletionItem::FunctionCompletionItem(DeclarationPointer decl, int depth, int atArgument):
                                               NormalDeclarationCompletionItem(decl, QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), 0),
                                               m_depth(depth), m_atArgument(atArgument)
{
    auto function = decl.dynamicCast<GoFunctionDeclaration>();
    GoFunctionType::Ptr type(fastCast<GoFunctionType*>(decl->abstractType().constData()));
    if(!type)
        return;

    bool variadicArgs = false;
    if(type->modifiers() == go::GoFunctionType::VariadicArgument)
        variadicArgs = true;

    DUContext* argsContext = 0;
    if(function)
        argsContext = function->internalContext();
    m_arguments = "(";
    if(argsContext)
    {
        DUChainReadLocker lock;
        auto args = argsContext->allDeclarations(CursorInRevision::invalid(), decl->topContext(), false);
        //highlight last argument if it is variadic
        if(variadicArgs && m_atArgument >= args.size())
            m_atArgument = args.size() - 1;
        int count = 0;
        for(auto arg : args)
        {
            if(m_atArgument == count)
                m_currentArgStart = m_arguments.length();
            m_arguments += (arg.first->toString());
            if(m_atArgument == count)
                m_currentArgEnd = m_arguments.length();
            count++;
            if(count < args.size())
                m_arguments += ", ";
        }
    }else if(type->arguments().size() != 0)
    {
        DUChainReadLocker lock;
        auto args = type->arguments();
        int count = 0;
        if(variadicArgs && m_atArgument >= args.size())
            m_atArgument = args.size() - 1;
        for(auto arg : args)
        {
            if(m_atArgument == count)
                m_currentArgStart = m_arguments.length();
            m_arguments += (arg->toString());
            if(m_atArgument == count)
                m_currentArgEnd = m_arguments.length();
            count++;
            if(count < args.size())
                m_arguments += ", ";
        }
    }
    if(variadicArgs && m_arguments.lastIndexOf('[') != -1)
    {
        if(m_currentArgEnd >= m_arguments.lastIndexOf('['))
            m_currentArgEnd++; //fix highlighting
        m_arguments = m_arguments.replace(m_arguments.lastIndexOf('['), 2, "...");
    }
    m_arguments += ")";
    DUContext* returnContext = 0;
    if(function)
        returnContext = function->returnArgsContext();
    m_prefix = "";
    if(returnContext)
    {
        DUChainReadLocker lock;
        auto args = returnContext->allDeclarations(CursorInRevision::invalid(), decl->topContext(), false);
        int count = 0;
        for(auto arg : args)
        {
            m_prefix += (arg.first->toString());
            count++;
            if(count < args.size())
                m_prefix += ", ";
        }
    }
    //if there was no return context or it didn't contain anything try type
    if((!returnContext || m_prefix == "") && type->returnArguments().size() != 0)
    {
        int count = 0;
        DUChainReadLocker lock;
        auto args = type->returnArguments();
        for(auto arg : args)
        {
            m_prefix += arg->toString();
            count++;
            if(count < args.size())
                m_prefix += ", ";
        }
    }
}

void FunctionCompletionItem::executed(KTextEditor::View* view, const KTextEditor::Range& word)
{
    KTextEditor::Document* document = view->document();
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
            view->setCursorPosition(KTextEditor::Cursor(word.end().line(), word.end().column() + 1));
    }
}

QVariant FunctionCompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
    switch(role)
    {
        case Qt::DisplayRole:
        {
            switch (index.column()) {
                case CodeCompletionModel::Prefix:
                    return m_prefix;
                case CodeCompletionModel::Arguments:
                    return m_arguments;
        }
        break;
        }
        case CodeCompletionModel::CompletionRole:
            return (int)completionProperties();

        case CodeCompletionModel::HighlightingMethod:
            if (index.column() == CodeCompletionModel::Arguments)
            {
                return (int)CodeCompletionModel::CustomHighlighting;
            }
            break;
        case KDevelop::CodeCompletionModel::CustomHighlight:
        {
            if (index.column() == CodeCompletionModel::Arguments && m_atArgument != -1)
            {
                QTextFormat format;

                format.setBackground(QBrush(QColor::fromRgb(142, 186, 255)));   // Same color as kdev-python
                format.setProperty(QTextFormat::FontWeight, 99);

                return QVariantList()
                    << m_currentArgStart
                    << m_currentArgEnd - m_currentArgStart
                    << format;
            }
        }
    }
    return NormalDeclarationCompletionItem::data(index, role, model);
}

CodeCompletionModel::CompletionProperties FunctionCompletionItem::completionProperties() const
{
    return CodeCompletionModel::Function;
}


int FunctionCompletionItem::argumentHintDepth() const
{
    return m_depth;
}

int FunctionCompletionItem::inheritanceDepth() const
{
    return 0;
}





}
