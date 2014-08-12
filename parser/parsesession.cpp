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

#include "parsesession.h"

#include "parser/golexer.h"
#include "parser/goparser.h"
#include "parser/goast.h"
#include "parser/gotokentext.h"

#include <kdebug.h>
#include <language/duchain/duchainlock.h>
#include <interfaces/icore.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/ilanguagecontroller.h>
#include <QProcess>

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-token-stream.h"

using namespace KDevelop;

ParseSession::ParseSession(const QByteArray& contents, int priority, bool appendWithNewline) : m_pool(new KDevPG::MemoryPool()),
						      m_parser(new go::Parser()),
						      //converting to and back from QString eliminates all the \000 at the end of contents
						      //m_contents(QString(contents).toUtf8()),
						      m_contents(contents),
						      m_priority(priority),
						      m_features(TopDUContext::AllDeclarationsAndContexts)
{
    //appending with new line helps lexer to set correct semicolons
    //(lexer sets semicolons on newlines if some conditions are met because
    // in Go user can omit semicolons after statements, but grammar still needs them)
    // See more in Go Language Specification http://golang.org/ref/spec#Semicolons
    if(appendWithNewline)
	m_contents.append("\n");
    KDevPG::QByteArrayIterator iter(m_contents); 
    m_lexer = new go::Lexer(iter);
    m_parser->setMemoryPool(m_pool);
    m_parser->setTokenStream(m_lexer);
    forExport=false;
    
}

ParseSession::~ParseSession()
{
    delete m_pool;
    delete m_parser;
    //delete m_iter;
}


KDevelop::IndexedString ParseSession::languageString()
{
    static const KDevelop::IndexedString langString("go");
    return langString;
}

bool ParseSession::startParsing()
{
    if(!lex())
	return false;
    
    return m_parser->parseStart(&m_ast);
}

bool ParseSession::lex()
{
    KDevPG::Token token;
    int kind = go::Parser::Token_EOF;
    while((kind = m_lexer->read().kind) != go::Parser::Token_EOF)
    {
        //qDebug() << go::tokenText(kind);
	if(kind == go::Parser::Token_TEST)
	{
	    qint64 line, column;
	    m_lexer->locationTable()->positionAt(m_lexer->index(), &line, &column);
	    qDebug() << "Lexer error at: " << line << " : " << column;
	    return false;
	}
    }
    m_parser->rewind(0);
    return true;
}

bool ParseSession::parseExpression(go::ExpressionAst** node)
{
    if(!lex())
	return false;
    
    return m_parser->parseExpression(node);
}


go::StartAst* ParseSession::ast()
{
    return m_ast;
}

QString ParseSession::symbol(qint64 index)
{
    if(index >= m_lexer->size())
	return QString();
    return QString(m_contents.mid(m_lexer->at(index).begin, m_lexer->at(index).end - m_lexer->at(index).begin+1));
}

KDevelop::RangeInRevision ParseSession::findRange(go::AstNode* from, go::AstNode* to)
{
    qint64 line, column, lineEnd, columnEnd;
    m_lexer->locationTable()->positionAt(m_lexer->at(from->startToken).begin, &line, &column);
    m_lexer->locationTable()->positionAt(m_lexer->at(to->endToken).end+1, &lineEnd, &columnEnd);
    //for some reason I need to shift columns when not on a first line
    if(line != 0) column++;
    if(lineEnd != 0) columnEnd++;

    return KDevelop::RangeInRevision(KDevelop::CursorInRevision(line, column), KDevelop::CursorInRevision(lineEnd, columnEnd));
}

KDevelop::IndexedString ParseSession::currentDocument()
{
    return m_document;
}

void ParseSession::setCurrentDocument(const KDevelop::IndexedString& document)
{
    m_document = document;
}

/**
 * Currently priority order works in this way
 * 	-1: Direct imports of opened file
 * 	 0: opened files
 * 	 1: Imports of direct imports(needed to resolve types of some functions)
 * 	 2: Reparse of direct imports, after its imports are finished
 * THIS AND BELOW IS NOT PARSED ATM
 * 	 3: Import of imports of direct imports(currently not parsed nor they actually needed I think)
 * 	 4: Reparse of imports of direct imports
 * 	 5: ...
 */
QList<ReferencedTopDUContext> ParseSession::contextForImport(QString package)
{
    package = package.mid(1, package.length()-2);
    //try $GOPATH first
    QByteArray result = qgetenv("GOPATH");
    QStringList files;
    QDir path;
    if(result.isEmpty())
    {//try find path automatically
	QDir currentDir(m_document.toUrl().directory());
	kDebug() << currentDir.dirName();
	while(currentDir.exists() && currentDir.dirName() != "src")
	    currentDir.cdUp();
	if(currentDir.exists() && currentDir.dirName() == "src")
	{
	    currentDir.cdUp();
	    if(currentDir.exists())
		result = currentDir.absolutePath().toUtf8();
	}
    }
    if(!result.isEmpty())
    {
	path = QDir(result);
	if(path.exists() && path.cd("src") && path.cd(package) && path.exists())
	{
	    files = path.entryList(QStringList("*.go"), QDir::Files | QDir::NoSymLinks);
	    //kDebug() << "path found in GOPATH: " << path.absolutePath() << " contains files: " << files;
	}
    }
    //then search $GOROOT
    //these days most people don't set GOROOT manually
    //instead go tool can find correct value for GOROOT on its own
    //in order for this to work go exec must be in $PATH
    QProcess p;
    p.start("go env GOROOT");
    p.waitForFinished();
    result = p.readAllStandardOutput();
    if(result.endsWith("\n")) result.remove(result.length()-1, 1);
    if(!result.isEmpty() && files.empty())
    {
	path = QDir(result);
	if(path.exists() && path.cd("src") && path.cd("pkg") && path.cd(package) && path.exists())
	{
	    files = path.entryList(QStringList("*.go"), QDir::Files | QDir::NoSymLinks);
	    //kDebug() << "path found in GOROOT: " << path.absolutePath() << " contains files: " << files;
	}
    }
    QList<ReferencedTopDUContext> contexts;
    bool shouldReparse=false;
    //reduce priority if it is recursive import
    int priority = forExport ? m_priority + 2 : m_priority - 1;
    for(QString filename : files)
    {
	filename = path.filePath(filename);
	QFile file(filename);
	if(!file.exists())
	    continue;
	
	IndexedString url(filename);
	DUChainReadLocker lock; 
	ReferencedTopDUContext context = DUChain::self()->chainForDocument(url);
	lock.unlock();
	if(context)
	    contexts.append(context);
	else
	{
	    shouldReparse = true;
	    scheduleForParsing(url, priority, (TopDUContext::Features)(TopDUContext::ForceUpdate | TopDUContext::AllDeclarationsAndContexts));
	}
    }
    if(shouldReparse) 
	//scheduleForParsing(m_document, m_priority, TopDUContext::AllDeclarationsContextsAndUses);
	scheduleForParsing(m_document, priority+1, (TopDUContext::Features)(m_features | TopDUContext::ForceUpdate));
    return contexts;
}

void ParseSession::scheduleForParsing(const IndexedString& url, int priority, TopDUContext::Features features)
{
    BackgroundParser* bgparser = KDevelop::ICore::self()->languageController()->backgroundParser();
    //TopDUContext::Features features = (TopDUContext::Features)(TopDUContext::ForceUpdate | TopDUContext::VisibleDeclarationsAndContexts);//(TopDUContext::Features)
	//(TopDUContext::ForceUpdate | TopDUContext::AllDeclarationsContextsAndUses);

    //currently recursive imports work really slow, nor they usually needed
    if(m_priority >= 1)
	return;
	
    if (bgparser->isQueued(url)) 
    {
	if (bgparser->priorityForDocument(url) > priority ) 
	    // Remove the document and re-queue it with a greater priority
	    bgparser->removeDocument(url);
	else 
	    return;
    }
    bgparser->addDocument(url, features, priority, 0, ParseJob::FullSequentialProcessing);
}


void ParseSession::reparseImporters(DUContext* context)
{
    DUChainReadLocker lock;

    for (DUContext* importer : context->importers()) {
        scheduleForParsing(importer->url(), m_priority, TopDUContext::AllDeclarationsContextsAndUses);
    }
}

QList< ReferencedTopDUContext > ParseSession::contextForThisPackage(IndexedString package)
{
    QList<ReferencedTopDUContext> contexts;
    KUrl url = package.toUrl();
    QDir path(url.directory());
    if(path.exists())
    {
	 QStringList files = path.entryList(QStringList("*.go"), QDir::Files | QDir::NoSymLinks);
	 bool shouldReparse=false;
	 for(QString filename : files)
	 {
	    filename = path.filePath(filename);
	    QFile file(filename);
	    if(!file.exists())
		continue;
	
	    IndexedString url(filename);
	    DUChainReadLocker lock; 
	    ReferencedTopDUContext context = DUChain::self()->chainForDocument(url);
	    lock.unlock();
	    if(context)
		contexts.append(context);
	    else
	    {
		scheduleForParsing(url, m_priority-1, (TopDUContext::Features)(TopDUContext::ForceUpdate | TopDUContext::AllDeclarationsAndContexts));
		shouldReparse=true;
	    }
	     
	 }
	 if(shouldReparse)
	     scheduleForParsing(m_document, m_priority, (TopDUContext::Features)(m_features | TopDUContext::ForceUpdate));
    }
    return contexts;
}

void ParseSession::setFeatures(TopDUContext::Features features)
{
    m_features = features;
    if((m_features & TopDUContext::AllDeclarationsContextsAndUses) == TopDUContext::AllDeclarationsAndContexts)
	forExport = true;
}

QString ParseSession::textForNode(go::AstNode* node)
{
    return QString(m_contents.mid(m_lexer->at(node->startToken).begin, m_lexer->at(node->endToken).end - m_lexer->at(node->startToken).begin+1));
}


