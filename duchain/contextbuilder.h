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

#ifndef KDEVGOLANGCONTEXTBUILDER_H
#define KDEVGOLANGCONTEXTBUILDER_H

#include <language/duchain/builders/abstractcontextbuilder.h>

#include "parser/goast.h"
#include "parser/godefaultvisitor.h"
#include "parser/parsesession.h"
#include "goduchainexport.h"



typedef KDevelop::AbstractContextBuilder<go::AstNode, go::IdentifierAst> ContextBuilderBase;

class Editor
{
public:
    Editor(ParseSession** session)
    : m_session(session)
    {}

    ParseSession* parseSession() const
    {
        return *m_session;
    }
private:
    ParseSession** m_session;
};


class KDEVGODUCHAIN_EXPORT ContextBuilder: public ContextBuilderBase, public go::DefaultVisitor
{
public:
    ContextBuilder();
    virtual ~ContextBuilder();

    /*virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url, go::AstNode* node,
            KDevelop::ReferencedTopDUContext updateContext
            = KDevelop::ReferencedTopDUContext());*/
    
    virtual void startVisiting(go::AstNode* node);
  
    
    virtual KDevelop::DUContext* contextFromNode(go::AstNode* node);
    
    virtual void setContextOnNode(go::AstNode* node, KDevelop::DUContext* context);
    
    virtual KDevelop::RangeInRevision editorFindRange(go::AstNode* fromNode, go::AstNode* toNode);
    
    virtual KDevelop::QualifiedIdentifier identifierForNode(go::IdentifierAst* node);
  
    KDevelop::QualifiedIdentifier identifierForIndex(qint64 index); 
   
    void setParseSession(ParseSession* session);
    
    
    virtual KDevelop::TopDUContext* newTopContext(const KDevelop::RangeInRevision& range, KDevelop::ParsingEnvironmentFile* file=0) override;
    
    virtual KDevelop::DUContext* newContext(const KDevelop::RangeInRevision& range) override;
    
    
    KDevelop::QualifiedIdentifier createFullName(go::IdentifierAst* package, go::IdentifierAst* typeName);
    
    ParseSession* parseSession();
    
    Editor* editor() const { return m_editor.data(); }
    
    
protected:
  
    ParseSession* m_session;
    
    
    bool m_mapAst; // make KDevelop::AbstractContextBuilder happy
    QScopedPointer<Editor> m_editor; // make KDevelop::AbstractUseBuilder happy
  
};

#endif