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

#ifndef KDEVGOLANGPARSESESSION_H
#define KDEVGOLANGPARSESESSION_H

#include <language/editor/rangeinrevision.h>
#include <language/duchain/topducontext.h>
#include <serialization/indexedstring.h>

#include "goparserexport.h"
#include "parser/goast.h"

namespace KDevPG
{
class MemoryPool;
}

namespace go
{
class Lexer;
class Parser;
class StartAst;
}

typedef QPair<KDevelop::DUContextPointer, KDevelop::RangeInRevision> SimpleUse;

class KDE_EXPORT ParseSession
{
public:
  
    ParseSession(const QByteArray& contents, int priority, bool appendWithNewline=true);
    
    virtual ~ParseSession();
  
    static KDevelop::IndexedString languageString();
    
    bool startParsing();
    
    bool parseExpression(go::ExpressionAst **node);
    
    go::StartAst* ast();
    
    QString symbol(qint64 index);

    KDevelop::RangeInRevision findRange(go::AstNode* from, go::AstNode* to);

    KDevelop::IndexedString currentDocument();

    void setCurrentDocument(const KDevelop::IndexedString& document);

    KDevelop::IndexedString url();

    QList<KDevelop::ReferencedTopDUContext> contextForImport(QString package);

    QList<KDevelop::ReferencedTopDUContext> contextForThisPackage(KDevelop::IndexedString package);

    /**
     * Schedules for parsing with given priority and features.
     * NOTE this will not schedule recursive imports (imports of imports and so on)
     * to schedule any file use BackgroundParser instead
     **/
    bool scheduleForParsing(const KDevelop::IndexedString& url, int priority, KDevelop::TopDUContext::Features features);

    /**
     * Reschedules this file with same features.
     **/
    void rescheduleThisFile();

    void reparseImporters(KDevelop::DUContext* context);

    void setFeatures(KDevelop::TopDUContext::Features features);

    QString textForNode(go::AstNode* node);

    void setIncludePaths(const QList<QString> &paths);

    void setCanonicalImports(QHash<QString, QString>* imports);

    /**
     * Returns doc comment preceding given token.
     * GoDoc comments are multilined /*-style comments
     * or several consecutive single-lined //-style comments
     * with no empty line between them.
     * Comment must start on a new line and end a line before given declaration.
     **/
    QByteArray commentBeforeToken(qint64 token);

    /**
     *	Don't use this function!
     *  Most of the times you don't need to access lexer of parseSession directly,
     *  This only exists, because parser test application uses DebugVisitor, which needs a lexer
     */
    friend go::Lexer* getLexer(const ParseSession& session) { return session.m_lexer; }
    
    void mapAstUse(go::AstNode* node, const SimpleUse& use)
    {
        Q_UNUSED(node);
        Q_UNUSED(use);
    }

private:
    
    bool lex();
    
    KDevPG::MemoryPool* m_pool;
    go::Lexer* m_lexer;
    go::Parser* m_parser;
    go::StartAst* m_ast;
    QByteArray m_contents;
    
    int m_priority;
    KDevelop::IndexedString m_document;
    KDevelop::TopDUContext::Features m_features;
    bool forExport;
    QList<QString> m_includePaths;
    QHash<QString, QString>* m_canonicalImports;
  
};

#endif