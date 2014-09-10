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

#include <language/duchain/types/delayedtype.h>

#include "contextbuilder.h"
#include "goducontext.h"
#include "duchaindebug.h"

using namespace KDevelop;

ContextBuilder::ContextBuilder()
{
    m_mapAst = false;
}


ContextBuilder::~ContextBuilder()
{

}

/*KDevelop::ReferencedTopDUContext ContextBuilder::build(const KDevelop::IndexedString& url, go::AstNode* node, KDevelop::ReferencedTopDUContext updateContext)
{
    return KDevelop::AbstractContextBuilder< go::AstNode, go::ExpressionAst >::build(url, node, updateContext);
}*/

void ContextBuilder::startVisiting(go::AstNode* node)
{
    //qCDebug(DUCHAIN) << "Start visiting";
    visitNode(node);
}




KDevelop::DUContext* ContextBuilder::contextFromNode(go::AstNode* node)
{
    return node->ducontext;
}

KDevelop::RangeInRevision ContextBuilder::editorFindRange(go::AstNode* fromNode, go::AstNode* toNode)
{
    if(!fromNode)
	return KDevelop::RangeInRevision();
    return m_session->findRange(fromNode, toNode ? toNode : fromNode);
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForNode(go::IdentifierAst* node)
{
    if(!node)
	return QualifiedIdentifier();
    return QualifiedIdentifier(m_session->symbol(node->id));
}

KDevelop::QualifiedIdentifier ContextBuilder::identifierForIndex(qint64 index)
{
    return QualifiedIdentifier(m_session->symbol(index));
}

void ContextBuilder::setContextOnNode(go::AstNode* node, KDevelop::DUContext* context)
{
    node->ducontext = context;
}

void ContextBuilder::setParseSession(ParseSession* session)
{
    m_session = session;
}


TopDUContext* ContextBuilder::newTopContext(const RangeInRevision& range, ParsingEnvironmentFile* file)
{
    
    if (!file) {
        file = new ParsingEnvironmentFile(m_session->currentDocument());
        file->setLanguage(m_session->languageString());
    }
    //return ContextBuilderBase::newTopContext(range, file);
    return new go::GoDUContext<TopDUContext>(m_session->currentDocument(), range, file);
}

DUContext* ContextBuilder::newContext(const RangeInRevision& range)
{
    return new go::GoDUContext<DUContext>(range, currentContext());
}


QualifiedIdentifier ContextBuilder::createFullName(go::IdentifierAst* package, go::IdentifierAst* typeName)
{
    QualifiedIdentifier id(m_session->symbol(package->id) + "." + m_session->symbol(typeName->id));
    return id;
}

ParseSession* ContextBuilder::parseSession()
{
    return m_session;
}

void ContextBuilder::visitIfStmt(go::IfStmtAst* node)
{
    //we need variables, declared in if pre-condition(if any) be available in if-block
    //and else-block, but not in parent context. We deal with it by opening another context
    //containing both if-block and else-block.
    openContext(node, editorFindRange(node, 0), DUContext::Other);
    DefaultVisitor::visitIfStmt(node);
    closeContext();
}

void ContextBuilder::visitBlock(go::BlockAst* node)
{
    openContext(node, editorFindRange(node, 0), DUContext::Other);
    go::DefaultVisitor::visitBlock(node);
    closeContext();
}
