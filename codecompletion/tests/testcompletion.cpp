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

#include "testcompletion.h"

#include "context.h"
#include "parser/parsesession.h"
#include "builders/declarationbuilder.h"

#include <QtTest/QtTest>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

using namespace KDevelop;

QTEST_MAIN(TestCompletion);

DUContext* getPackageContext(const QString& code)
{
    ParseSession session(code.toUtf8(), 0);
    static int testNumber = 0;
    session.setCurrentDocument(IndexedString(QString("file:///temp/%1").arg(testNumber++)));
    if(!session.startParsing())
        return 0;
    DeclarationBuilder builder(&session, false);
    ReferencedTopDUContext context =  builder.build(session.currentDocument(), session.ast());
    if(!context)
        return 0;

    DUChainReadLocker lock;
    auto decls = context->localDeclarations();
    if(decls.size() != 1)
        return 0;
    Declaration* packageDeclaration = decls.first();
    DUContext* packageContext = packageDeclaration->internalContext();
    return packageContext;
}


QList<CompletionTreeItemPointer> getCompletions(QString code)
{
    int cursorIndex = code.indexOf("%CURSOR");
    Q_ASSERT(cursorIndex != -1);
    DUContext* context = getPackageContext(code.replace(cursorIndex, 7, ""));
    CursorInRevision cursor(0, cursorIndex);
    DUChainReadLocker lock;
    context = context->findContextAt(cursor);
    Q_ASSERT(context);

    go::CodeCompletionContext* completion = new go::CodeCompletionContext(DUContextPointer(context),
                                                                         code.mid(0, cursorIndex),
                                                                         cursor);
    bool abort = false;
    return completion->completionItems(abort, true);
}

bool containsDeclaration(QString declaration, const QList<CompletionTreeItemPointer>& completions)
{
    for(const CompletionTreeItemPointer item : completions)
    {
        if(item->declaration()->toString() == declaration)
            return true;
    }
    //print completions if we haven't found anything
    qDebug() << "Completions debug print";
    for(const CompletionTreeItemPointer item : completions)
    {
        qDebug() << item->declaration()->toString();
    }
    return false;
}

void TestCompletion::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCompletion::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestCompletion::test_basicCompletion()
{
    QString code("package main; func main() { var a int; %CURSOR }");
    auto completions = getCompletions(code);
    QCOMPARE(completions.size(), 3);
    QVERIFY(containsDeclaration(" main ()", completions)); //function
    QVERIFY(containsDeclaration("<notype> main", completions)); //package TODO fix <notype>
    QVERIFY(containsDeclaration("int a", completions)); //variable
}
