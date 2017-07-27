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

#ifndef KDEVGOLANGPARSEJOB_H
#define KDEVGOLANGPARSEJOB_H

#include <language/backgroundparser/parsejob.h>

class GoParseJob : public KDevelop::ParseJob
{
public:
     GoParseJob(const KDevelop::IndexedString& url, KDevelop::ILanguageSupport* languageSupport);
  
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:
    /**
     * Parses every .go file available in search paths,
     * looks for declarations like ` package pack //import "my_pack" `
     * which means that package pack must be imported under name my_pack.
     * Canonical imports paths should be available before any other parsing
     * can begin, so it must be fast.
     **/
    void parseCanonicalImports();

    /**
     * extracts package's canonical import name.
     **/
    QString extractCanonicalImport(QString string);
    static QHash<QString, QString> canonicalImports;

};

#endif