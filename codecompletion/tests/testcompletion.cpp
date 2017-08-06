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
#include "model.h"
#include "parser/parsesession.h"
#include "builders/declarationbuilder.h"

#include <language/codecompletion/codecompletionmodel.h>
#include <QStandardItemModel>
#include <QtTest/QtTest>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

using namespace KDevelop;

QTEST_MAIN(TestCompletion);

QStandardItemModel& fakeModel() {
  static QStandardItemModel model;
  model.setColumnCount(10);
  model.setRowCount(10);
  return model;
}

go::CodeCompletionModel* model = 0;

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
    model = new go::CodeCompletionModel(nullptr);
    model->setCompletionContext(QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(completion));
    bool abort = false;
    return completion->completionItems(abort, true);
}

void debugPrint(const QList<CompletionTreeItemPointer>& completions, bool showQuality=false)
{
    qDebug() << "Completions debug print";
    QModelIndex prefixIdx = fakeModel().index(0, KDevelop::CodeCompletionModel::Prefix);
    QModelIndex nameIdx = fakeModel().index(0, KDevelop::CodeCompletionModel::Name);
    QModelIndex argsIdx = fakeModel().index(0, KDevelop::CodeCompletionModel::Arguments);
    for(const CompletionTreeItemPointer item : completions)
    {
        QStringList name;
        name << item->data(prefixIdx, Qt::DisplayRole, nullptr).toString();
        name << item->data(nameIdx, Qt::DisplayRole, nullptr).toString();
        name << item->data(argsIdx, Qt::DisplayRole, nullptr).toString();
        if(!showQuality)
            qDebug() << name.join(' ');
        else
        {
            int quality = item->data(nameIdx, KDevelop::CodeCompletionModel::MatchQuality, model).toInt();
            qDebug() << name.join(' ') << quality;
        }
    }
}

bool containsDeclaration(QString declaration, const QList<CompletionTreeItemPointer>& completions, int depth=-1, int quality=-1)
{
    QModelIndex prefixIdx = fakeModel().index(0, KDevelop::CodeCompletionModel::Prefix);
    QModelIndex nameIdx = fakeModel().index(0, KDevelop::CodeCompletionModel::Name);
    QModelIndex argsIdx = fakeModel().index(0, KDevelop::CodeCompletionModel::Arguments);
    for(const CompletionTreeItemPointer item : completions)
    {
        //if(item->declaration()->toString() == declaration)
        QStringList name;
        name << item->data(prefixIdx, Qt::DisplayRole, nullptr).toString();
        name << item->data(nameIdx, Qt::DisplayRole, nullptr).toString();
        name << item->data(argsIdx, Qt::DisplayRole, nullptr).toString();
        int itemQuality = item->data(nameIdx, KDevelop::CodeCompletionModel::MatchQuality, model).toInt();
        if(name.join(' ') == declaration && (depth == -1 || item->argumentHintDepth() == depth)
            && (quality == -1 || quality == itemQuality)
        )
            return true;
    }
    //print completions if we haven't found anything
    debugPrint(completions, quality == -1 ? false : true);
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
    QVERIFY(containsDeclaration("namespace main ", completions)); //package
    QVERIFY(containsDeclaration("int a ", completions)); //variable
}

void TestCompletion::test_functionCallTips_data()
{
    QTest::addColumn<QString>("declaration");
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("depth");
    QTest::addColumn<int>("size");

    QTest::newRow("normal") << "func myfunc() {};" << "myfunc(%CURSOR)" << " myfunc ()" << 1 << 4;
    QTest::newRow("args") << "func myfunc(a int) {};" << "myfunc(%CURSOR)" << " myfunc (int a)" << 1 << 4;
    QTest::newRow("args 2") << "type mytype int; func myfunc(a, b rune, c mytype) {};" << "myfunc(%CURSOR)"
        << " myfunc (rune a, rune b, main::mytype c)" << 1 << 5;
    QTest::newRow("rargs") << "func myfunc(a int) float32 {};" << "myfunc(%CURSOR)" << "float32 myfunc (int a)" << 1 << 4;
    QTest::newRow("rargs 2") << "func myfunc(a int) (float32, bool) {};" << "myfunc(%CURSOR)" << "float32, bool myfunc (int a)" << 1 << 4;
    QTest::newRow("rargs 3") << "func myfunc(a int) (r rune, b bool) {};" << "myfunc(%CURSOR)" << "rune r, bool b myfunc (int a)" << 1 << 4;
    QTest::newRow("variadic args") << "func myfunc(a ...int) {};" << "myfunc(%CURSOR)" << " myfunc (int... a)" << 1 << 4;
    QTest::newRow("variadic args 2") << "func myfunc(a int, i ...interface{}) {};" << "myfunc(%CURSOR)" << " myfunc (int a, interface{}... i)" << 1 << 4;
    QTest::newRow("var") << "var myvar func(a int);" << "myvar(%CURSOR)" << " myvar (int)" << 1 << 4;
    QTest::newRow("var 2") << "func functest(t int) rune;" << "myvar := functest; myvar(%CURSOR)" << "rune myvar (int)" << 1 << 5;
    QTest::newRow("struct") << "type mytype struct { f func(t int) rune; };" << "var myvar mytype; myvar.f(%CURSOR)" << "rune f (int)" << 1 << 5;
    QTest::newRow("method") << "type mytype int; func (m mytype) myfunc(c rune) {};" << "var myvar mytype; myvar.myfunc(%CURSOR)" << " myfunc (rune c)" << 1 << 5;
    QTest::newRow("method of embedded struct declared after method") << "type mytype struct {}; func (m mytype) myfunc(c rune) {}; type mytype2 struct {*mytype};" << "var myvar mytype2; myvar.myfunc(%CURSOR)" << " myfunc (rune c)" << 1 << 6;
    QTest::newRow("method of embedded struct declared before method") << "type mytype struct {}; func (m mytype) myfunc(c rune) {}; type mytype2 struct {*mytype};" << "var myvar mytype2; myvar.myfunc(%CURSOR)" << " myfunc (rune c)" << 1 << 6;
    QTest::newRow("paren") << "func myfunc(a int) {};" << "(myfunc)(%CURSOR)" << " myfunc (int a)" << 1 << 4;
    QTest::newRow("nested") << "func myfunc(a int) {};" << "f := myfunc; myfunc(f(%CURSOR))" << " myfunc (int a)" << 2 << 6;
    QTest::newRow("nested 2") << "func myfunc(a int) {};" << "f := myfunc; myfunc(f(%CURSOR))" << " f (int)" << 1 << 6;
    //TODO these don't work yet(they still pass, but they're wrong)
    //QTest::newRow("lambda") << "" << "f := func() int { \n}(%CURSOR)" << "int <unknown> ()" << 0 << 3;
    QTest::newRow("multiple lines") << "func myfunc(a, b int) {};" << "myfunc(5, \n %CURSOR)" << " myfunc (int a, int b)" << 0 << 3;
}

void TestCompletion::test_functionCallTips()
{
    QFETCH(QString, declaration);
    QFETCH(QString, expression);
    QFETCH(QString, result);
    QFETCH(int, depth);
    QFETCH(int, size);
    QString code(QString("package main; %1  func main() { %2 }").arg(declaration).arg(expression));
    auto completions = getCompletions(code);
    //debugPrint(completions);
    QCOMPARE(completions.size(), size);
    QVERIFY(containsDeclaration(result, completions, depth));
}

void TestCompletion::test_typeMatching_data()
{
    QTest::addColumn<QString>("declaration");
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("size");
    QTest::addColumn<int>("quality");

    QTest::newRow("assignment") << "var pi int = 3.14" << "var a int; a = %CURSOR a" << "int pi " << 4 << 5;
    QTest::newRow("const") << "const pi int = 3.14" << "var a int; a = %CURSOR a" << "int pi " << 4 << 5;
    QTest::newRow("function") << "func test() int {}" << "var a int; a = %CURSOR a" << "int test ()" << 4 << 5;
    QTest::newRow("function var") << "func test() int {}" << "f := test; var a int; a = %CURSOR a" << "function () int f " << 5 << 5;
    QTest::newRow("function type") << "func test() int {}" << "var a func() int; a = %CURSOR a" << "int test ()" << 4 << 5;
    QTest::newRow("function type var") << "func test() int {}" << "f := test; var a func() int; a = %CURSOR a" << "function () int f " << 5 << 5;
    QTest::newRow("arguments") << "func test(a int) {}" << "var a int; test(%CURSOR);" << "int a " << 5 << 5;
    QTest::newRow("sum") << "var a int" << "var b int; b + %CURSOR c" << "int a " << 4 << 5;
    QTest::newRow("multiple assignments") << "var a int = 1; var b bool = true" << "a, b = %CURSOR a, b" << "int a " << 4 << 5;
    QTest::newRow("multiple assignments 2") << "var a int = 1; var b bool = true" << "a, b = a, %CURSOR b" << "bool b " << 4 << 5;
    QTest::newRow("multiple assignments with function return args assigment") << "var a int = 1; var b bool = true" << "a, b = test(1, 3, 4), %CURSOR b" << "bool b " << 4 << 5;
    QTest::newRow("multiple assignments with function returning multiple args") << "var a int = 1; var b bool = true; func test() (x int, y bool) {}" << "a, b = %CURSOR b" << "int x, bool y test ()" << 5 << 10;
    QTest::newRow("multiple assignments with not declared var") << "var a int = 1; func test() (x int, y bool) {}" << "a, b := %CURSOR b" << "int x, bool y test ()" << 5 << 10;
    QTest::newRow("array index") << "var a int = 1" << "x := make([]int, 0, 10); x[%CURSOR 1]" << "int a " << 4 << 5;
    QTest::newRow("map index") << "var a string" << "x := make(map [string]int, 0, 10); x[%CURSOR 1]" << "string a " << 4 << 5;
}

void TestCompletion::test_typeMatching()
{
    QFETCH(QString, declaration);
    QFETCH(QString, expression);
    QFETCH(QString, result);
    QFETCH(int, size);
    QFETCH(int, quality);
    QString code(QString("package main; %1; func main() { %2 }").arg(declaration).arg(expression));
    auto completions = getCompletions(code);
    QCOMPARE(completions.size(), size);
    QVERIFY(containsDeclaration(result, completions, -1, quality));
}

void TestCompletion::test_commentCompletion_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<int>("size");

    QTest::newRow("line comment") << "// %CURSOR \n" << 0;
    QTest::newRow("line comment 2") << "// \n %CURSOR" << 2;
    QTest::newRow("comment") << "/* \"asdf \" %CURSOR */" << 0;
    QTest::newRow("comment 2") << "/* \'asdf \' */ %CURSOR" << 2;
    QTest::newRow("comment 3") << "/* ` %CURSOR */" << 0;
    QTest::newRow("comment 4") << "a := \"/* ` \" %CURSOR " << 3;
    QTest::newRow("string") << " a := \" %CURSOR \"" << 0;
    QTest::newRow("string 2") << " a := \" \" %CURSOR" << 3;
    QTest::newRow("string 3") << " a := ` %CURSOR `" << 0;
    QTest::newRow("string 4") << " a := ` ` %CURSOR" << 3;
    QTest::newRow("rune") << " a := \' %CURSOR\'" << 0;
    QTest::newRow("rune 2") << " a := \'a\' %CURSOR" << 3;
}

void TestCompletion::test_commentCompletion()
{
    QFETCH(QString, expression);
    QFETCH(int, size);
    QString code(QString("package main; func main() { %1 }").arg(expression));
    auto completions = getCompletions(code);
    QCOMPARE(completions.size(), size);
}

void TestCompletion::test_membersCompletion_data()
{
    QTest::addColumn<QString>("declaration");
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QString>("result");
    QTest::addColumn<int>("size");
    QTest::newRow("method multiple lines") << "type mytype int; func (m mytype) myfunc() (t mytype) {}; func (m mytype) myfunc2(c rune) {}" << "var myvar mytype; myvar.myfunc().\n%CURSOR x;" << " myfunc2 (rune c)" << 2;
}

void TestCompletion::test_membersCompletion()
{
    QFETCH(QString, declaration);
    QFETCH(QString, expression);
    QFETCH(QString, result);
    QFETCH(int, size);
    QString code(QString("package main; %1; func main() { %2 }").arg(declaration).arg(expression));
    auto completions = getCompletions(code);
    QCOMPARE(completions.size(), size);
    QVERIFY(containsDeclaration(result, completions));
}