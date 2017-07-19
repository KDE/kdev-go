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

#include "golangparsejob.h"

#include <language/backgroundparser/urlparselock.h>
#include <language/backgroundparser/parsejob.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/interfaces/ilanguagesupport.h>

#include <QReadLocker>
#include <QProcess>
#include <QDirIterator>

#include "parsesession.h"
#include "duchain/builders/declarationbuilder.h"
#include "duchain/builders/usebuilder.h"
#include "duchain/helper.h"
#include "godebug.h"

using namespace KDevelop;

QHash<QString, QString> GoParseJob::canonicalImports;

GoParseJob::GoParseJob(const KDevelop::IndexedString& url, KDevelop::ILanguageSupport* languageSupport): ParseJob(url, languageSupport)
{
}

void GoParseJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
{
   qCDebug(Go) << "GoParseJob succesfully created for document " << document(); 

   UrlParseLock urlLock(document());

    if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
        return;
    }

    ProblemPointer p = readContents();
    if(p) {
         return;
    }

    QByteArray code = contents().contents;
    while(code.endsWith('\0'))
        code.chop(1);

    //ParseSession session(QString(contents().contents).toUtf8(), priority());
    ParseSession session(code, parsePriority());
    
    session.setCurrentDocument(document());
    session.setFeatures(minimumFeatures());

    if(abortRequested())
      return;


    ReferencedTopDUContext context;
    {
        DUChainReadLocker lock;
        context = DUChainUtils::standardContextForUrl(document().toUrl());
    }
    
    //ParsingEnvironmentFilePointer filep = context->parsingEnvironmentFile();
    
    if (context) {
        translateDUChainToRevision(context);
        context->setRange(RangeInRevision(0, 0, INT_MAX, INT_MAX));
    }
    qCDebug(Go) << "Job features: " << minimumFeatures();
    qCDebug(Go) << "Job priority: " << parsePriority();

    qCDebug(Go) << document();
    bool result = session.startParsing();
    //this is useful for testing parser, comment it for actual working
    //Q_ASSERT(result);

    //When switching between files(even if they are not modified) KDevelop decides they need to be updated
    //and calls parseJob with VisibleDeclarations feature
    //so for now feature, identifying import will be AllDeclarationsAndContexts, without Uses
    bool forExport = false;
    if((minimumFeatures() & TopDUContext::AllDeclarationsContextsAndUses) == TopDUContext::AllDeclarationsAndContexts)
        forExport = true;
    //qCDebug(Go) << contents().contents;

    if(!forExport)
        session.setIncludePaths(go::Helper::getSearchPaths(document().toUrl()));
    else
        session.setIncludePaths(go::Helper::getSearchPaths());

    if(canonicalImports.empty())
        parseCanonicalImports();
    session.setCanonicalImports(&canonicalImports);

    if(result)
    {
        QReadLocker parseLock(languageSupport()->parseLock());

        if(abortRequested())
            return abortJob();
        //qCDebug(Go) << QString(contents().contents);
        DeclarationBuilder builder(&session, forExport);
        context = builder.build(document(), session.ast(), context);

        if(!forExport)
        {
            go::UseBuilder useBuilder(&session);
            useBuilder.buildUses(session.ast());
        }
        //this notifies other opened files of changes
        //session.reparseImporters(context);

    }
    if(!context){
        DUChainWriteLocker lock;
        ParsingEnvironmentFile* file = new ParsingEnvironmentFile(document());
        file->setLanguage(ParseSession::languageString());
        context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
        DUChain::self()->addDocumentChain(context);
    }
        
    setDuChain(context);
    {
        DUChainWriteLocker lock;
        context->setProblems(session.problems());
        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
        DUChain::self()->updateContextEnvironment(context->topContext(), file.data());
    }
    highlightDUChain();

    DUChain::self()->emitUpdateReady(document(), duChain());

    if(result)
      qCDebug(Go) << "===Success===" << document().str();
    else
      qCDebug(Go) << "===Failed===" << document().str();
}

void GoParseJob::parseCanonicalImports()
{
    QList<QString> importPaths = go::Helper::getSearchPaths();
    for(const QString& path : importPaths)
    {
        QDirIterator iterator(path, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while(iterator.hasNext())
        {
            iterator.next();
            QDir dir(iterator.filePath());
            for(const QString& file : dir.entryList(QStringList("*.go"), QDir::Files | QDir::NoSymLinks))
            {
                QFile f(dir.filePath(file));
                f.open(QIODevice::ReadOnly);
                QByteArray contents = f.readAll();
                f.close();
                QString canonicalImport = extractCanonicalImport(QString(contents));
                if(canonicalImport.length() != 0)
                {
                    qCDebug(Go) << "Found canonical import for package " << iterator.filePath() << " import: " << canonicalImport;
                    canonicalImports[canonicalImport] = iterator.filePath();
                    break;
                }
            }
        }
    }
    //if no canonical imports were found add stab value to map
    //so we won't search for them again
    if(canonicalImports.empty())
        canonicalImports["<?>"] = QString("none");
}

QString GoParseJob::extractCanonicalImport(QString string)
{
    int i = 0;
    while(i < string.length())
    {
       if(string[i].isSpace())
           i++;
       else if(string[i] == '/')
       {
           if(i + 1 < string.length() && string[i+1] == '/')
           {
               i += 2;
               while(i<string.length() && string[i] != '\n')
                   ++i;
               ++i;
           }
           else if(i + 1 < string.length() && string[i+1] == '*')
           {
               i += 2;
               while(i+1<string.length() && !(string[i] == '*' && string[i+1] == '/'))
                   ++i;
               i+=2;
           }else
               return QString("");
       }else if(string[i] == 'p')
       {
           string = string.mid(i);
           //match "package name // or /* import \" "
           if(string.indexOf(QRegExp("^package\\s*\\w*\\s*(//|/\\*)\\s*import\\s*\"")) == 0)
           {
               int nameStart = string.indexOf("\"")+1;
               int nameEnd = string.indexOf("\"", nameStart);
               if(nameStart != -1 && nameEnd != -1)
                   return string.mid(nameStart, nameEnd - nameStart);
           }
           return QString("");
       }else
           return QString("");
    }
    return QString("");
}

