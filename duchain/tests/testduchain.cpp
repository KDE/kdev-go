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

#include "testduchain.h"

#include "parser/parsesession.h"
#include "builders/declarationbuilder.h"
#include "types/gointegraltype.h"

#include <QtTest/QtTest>
//#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

QTEST_MAIN(TestDuchain);

using namespace KDevelop;

DUContext* getPackageContext(const QString& code);
DUContext* getMainContext(const QString& code);

void TestDuchain::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestDuchain::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDuchain::sanityCheck()
{
    QString code("package main; func main() {}");
    ParseSession session(code.toUtf8(), 0);
    session.setCurrentDocument(IndexedString("file:///temp/1"));
    QVERIFY(session.startParsing());
    DeclarationBuilder builder(&session, false);
    ReferencedTopDUContext context =  builder.build(session.currentDocument(), session.ast());
    QVERIFY(context.data());

    DUChainReadLocker lock;
    auto decls = context->localDeclarations();
    QCOMPARE(decls.size(), 1);
    Declaration* packageDeclaration = decls.first();
    QVERIFY(packageDeclaration);
    DUContext* packageContext = packageDeclaration->internalContext();
    QVERIFY(packageContext);
    decls = packageContext->localDeclarations();
    QCOMPARE(decls.size(), 1);
    Declaration* funcDeclaration = decls.first();
    QVERIFY(funcDeclaration);
    QCOMPARE(funcDeclaration->identifier().toString(), QString("main"));
}

void TestDuchain::builtinFunctions_data()
{
    QTest::addColumn<QString>("expression");
    QTest::addColumn<QString>("type");

    QTest::newRow("make slice") << "make([]int)" << "int[]";
    QTest::newRow("make slice 2") << "make([]mytype)" << "main::mytype[]";
    QTest::newRow("make chan") << "make(chan float64)" << "chan float64";
    QTest::newRow("make chan 2") << "make(chan unknown)" << "chan unknown";
    QTest::newRow("make map") << "make(map[uint]rune)" << "map[uint]rune";
    QTest::newRow("make map 2") << "make(map[string]mytype)" << "map[string]main::mytype";

    QTest::newRow("new") << "new(map[byte]mytype)" << "map[byte]main::mytype*";
    QTest::newRow("new 2") << "new(mytype)" << "main::mytype*";
    QTest::newRow("new 2") << "new([]unknown)" << "unknown[]*";
    QTest::newRow("append") << "append([]int, 1, 2)" << "int[]";

    QTest::newRow("cap") << "cap(myvar)" << "int";
    QTest::newRow("copy") << "copy(a, b)" << "int";
    QTest::newRow("len") << "len(array)" << "int";
    QTest::newRow("real") << "real(5i)" << "float64";
    QTest::newRow("real 2") << "real(myvar)" << "float32";
    QTest::newRow("imag") << "imag(2i)" << "float64";
    QTest::newRow("imag 2") << "imag(myvar)" << "float32";
    QTest::newRow("imag 3") << "imag(2+3i)" << "float64";
    QTest::newRow("complex") << "complex(-1, 0)" << "complex128";
    QTest::newRow("recover") << "recover()" << "interface {}";
    QTest::newRow("real + complex") << "real(complex(1, 1))" << "float64";
}


void TestDuchain::builtinFunctions()
{
    QFETCH(QString, expression);
    QFETCH(QString, type);

    QString code(QString("package main; type mytype int; var myvar complex64; func main() { testvar := %1; }").arg(expression));

    DUContext* context = getMainContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    auto decls = context->findDeclarations(QualifiedIdentifier("testvar"));
    QCOMPARE(decls.size(), 1);
    Declaration* decl = decls.first();
    AbstractType::Ptr result = decl->abstractType();

    QCOMPARE(result->toString(), type);
}

void TestDuchain::test_declareVariables()
{
    QString code("package main; func multitest() (int, bool) { return 1, true; } \n \
		func singletest() rune { return 'a'; } \n func main() { test1, test2 := multitest(); \
		test3, test4 := singletest(), 3., 100; var test5, test6, test7 = multitest(), singletest(); var _, test8, _ = 0., 5, true;}");

    DUContext* context = getMainContext(code);
    DUChainReadLocker lock;
    Declaration* decl = context->findDeclarations(QualifiedIdentifier("test1")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeInt));
    QVERIFY((decl->abstractType()->modifiers() & AbstractType::NoModifiers) == AbstractType::NoModifiers);
    decl = context->findDeclarations(QualifiedIdentifier("test2")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeBool));
    decl = context->findDeclarations(QualifiedIdentifier("test3")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeRune));
    decl = context->findDeclarations(QualifiedIdentifier("test4")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeFloat64));
    decl = context->findDeclarations(QualifiedIdentifier("test5")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeInt));
    decl = context->findDeclarations(QualifiedIdentifier("test6")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeRune));
    auto declarations = context->findDeclarations(QualifiedIdentifier("test7"));
    QCOMPARE(declarations.size(), 0);

    decl = context->findDeclarations(QualifiedIdentifier("test8")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeInt));
    declarations = context->findDeclarations(QualifiedIdentifier("_"));
    QCOMPARE(declarations.size(), 0);
}

void TestDuchain::test_constants()
{
    QString code("package main; const const1, const2 float32 = 1, 2; const ( const3, const4, const5 = 'a', 3, \"abc\"; ); ");
    DUContext* context = getPackageContext(code);
    DUChainReadLocker lock;
    Declaration* decl = context->findDeclarations(QualifiedIdentifier("const1")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeFloat32));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const2")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeFloat32));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const3")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeRune));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const4")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeInt));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const5")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeString));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
}

void TestDuchain::test_constants_omittedType()
{
    QString code("package main; const ( const1, const2 uint = 1, 2; const3, const4; ); const ( const5, const6 = 3, 4.; const7, const8; ); const const9;");
    DUContext* context = getPackageContext(code);
    DUChainReadLocker lock;
    Declaration* decl = context->findDeclarations(QualifiedIdentifier("const1")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeUint));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const2")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeUint));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const3")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeUint));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const4")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeUint));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);

    decl = context->findDeclarations(QualifiedIdentifier("const5")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeInt));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const6")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeFloat64));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const7")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeInt));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);
    decl = context->findDeclarations(QualifiedIdentifier("const8")).first();
    QCOMPARE(fastCast<go::GoIntegralType*>(decl->abstractType().constData())->dataType(), uint(go::GoIntegralType::TypeFloat64));
    QVERIFY(decl->abstractType()->modifiers() & AbstractType::ConstModifier);

    QCOMPARE(context->findDeclarations(QualifiedIdentifier("const9")).size(), 0);
}

void TestDuchain::test_indexexpressions_data()
{
    QTest::addColumn<QString>("vardecl");
    QTest::addColumn<QString>("indexexpr");
    QTest::addColumn<QString>("type");
    QTest::addColumn<bool>("mapaccess");

    QTest::newRow("array index") << "var array [5]int" << "array[1]" << "int" << false;
    QTest::newRow("slice index") << "var slice []string" << "slice[4]" << "string" << false;
    QTest::newRow("array pointer index") << "var array *[5]int" << "array[1]" << "int" << false;
    QTest::newRow("string index") << "var str string" << "str[4]" << "byte" << false;
    QTest::newRow("map index") << "var mymap map[int][]string" << "mymap[2*2]" << "string[]" << true;
    QTest::newRow("map index 2") << "var mymap map[rune]*mytype" << "mymap[\'o\']" << "main::mytype*" << true;
    QTest::newRow("slice expression") << "var slice []int" << "slice[1:4]" << "int[]" << false;
    QTest::newRow("slice expression 2") << "var slice []int" << "slice[:4]" << "int[]" << false;
    QTest::newRow("slice expression 3") << "var slice []int" << "slice[1:]" << "int[]" << false;
    QTest::newRow("slice expression 4") << "var slice []int" << "slice[:]" << "int[]" << false;
    QTest::newRow("array expression") << "var array [5-3]bool" << "array[0:2]" << "bool[]" << false;
    QTest::newRow("string expression") << "var str string" << "str[0:]" << "string" << false;
    QTest::newRow("array pointer expression") << "var array *[]bool" << "array[0:2]" << "bool[]" << false;
    QTest::newRow("full slice expression") << "var slice []int" << "slice[1:3:5]" << "int[]" << false;
    QTest::newRow("full slice expression 2") << "var slice []mytype" << "slice[:3:5]" << "main::mytype[]" << false;
}


void TestDuchain::test_indexexpressions()
{
    QFETCH(QString, vardecl);
    QFETCH(QString, indexexpr);
    QFETCH(QString, type);
    QString code(QString("package main; type mytype int; func main() { %1; testvar, ok := %2; }").arg(vardecl).arg(indexexpr));
    DUContext* context = getMainContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    auto decls = context->findDeclarations(QualifiedIdentifier("testvar"));
    QCOMPARE(decls.size(), 1);
    Declaration* decl = decls.first();
    AbstractType::Ptr result = decl->abstractType();

    QCOMPARE(result->toString(), type);

    QFETCH(bool, mapaccess);
    decls = context->findDeclarations(QualifiedIdentifier("ok"));
    if(mapaccess)
    {
        QCOMPARE(decls.size(), 1);
        decl = decls.first();
        result = decl->abstractType();
        QCOMPARE(result->toString(), QString("bool"));
    }else
        QCOMPARE(decls.size(), 0);
}

void TestDuchain::test_ifcontexts()
{
    QString code("package main; func main() { var test1 int; if test2:=0; true { var test3 int; } else if false {  } else { var test4 int; } }");
    DUContext* context = getMainContext(code);
    DUChainReadLocker lock;
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test1")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test2")).size(), 0);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test3")).size(), 0);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    DUContext* childContext = context->findContextAt(CursorInRevision(0, 65)); //if block context
    QVERIFY(context);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test1")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test2")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test3")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    childContext = context->findContextAt(CursorInRevision(0, 97)); //else-if block context
    QVERIFY(context);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test1")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test2")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test3")).size(), 0);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    childContext = context->findContextAt(CursorInRevision(0, 116)); //else-else block context
    QVERIFY(context);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test1")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test2")).size(), 1);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test3")).size(), 0);
    QCOMPARE(childContext->findDeclarations(QualifiedIdentifier("test4")).size(), 1);
}

void TestDuchain::test_funccontexts()
{
    QString code("package main; type mytype int; func (i mytype) main(a, b string, c float64) (d, e int) { var f mytype; }");
    DUContext* context = getPackageContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    auto decls = context->findDeclarations(QualifiedIdentifier("mytype::main"));
    QCOMPARE(decls.size(), 1);
    AbstractFunctionDeclaration* function = dynamic_cast<AbstractFunctionDeclaration*>(decls.first());
    QVERIFY(function);
    context = function->internalFunctionContext();
    QCOMPARE(context->localDeclarations().size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("i")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("a")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("b")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("c")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("d")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("e")).size(), 1);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("f")).size(), 1);
}

void TestDuchain::test_rangeclause_data()
{
    QTest::addColumn<QString>("vardecl");
    QTest::addColumn<QString>("rangeexpr");
    QTest::addColumn<QString>("type1");
    QTest::addColumn<QString>("type2");

    QTest::newRow("array range") << "var array [5]int" << "array" << "int" << "int";
    QTest::newRow("slice range") << "var slice []string" << "slice" << "int" << "string";
    QTest::newRow("array pointer range") << "var longestshittyname *[]string" << "longestshittyname" << "int" << "string";
    QTest::newRow("string range") << "var str string" << "str" << "int" << "rune";
    QTest::newRow("map range") << "var mymap map[int][]string" << "mymap" << "int" << "string[]";
    QTest::newRow("map range 2") << "var mymap map[rune]*mytype" << "mymap" << "rune" << "main::mytype*";
    QTest::newRow("chan range") << "var mychan chan *mytype" << "mychan" << "main::mytype*" << "nil";
    QTest::newRow("chan range 2") << "var mychan <- chan []int" << "mychan" << "int[]" << "nil";
    QTest::newRow("chan range 3") << "var mychan chan <- struct{ b int}" << "mychan" << "struct{ b int}" << "nil";
}

void TestDuchain::test_rangeclause()
{
    QFETCH(QString, vardecl);
    QFETCH(QString, rangeexpr);
    QFETCH(QString, type1);
    QFETCH(QString, type2);
    QString code(QString("package main; type mytype int; func main() { %1; for test, test2 := range %2 {   } }").arg(vardecl).arg(rangeexpr));
    DUContext* context = getMainContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    context = context->findContextAt(CursorInRevision(0, 80));
    QVERIFY(context);
    auto decls = context->findDeclarations(QualifiedIdentifier("test"));
    QCOMPARE(decls.size(), 1);
    Declaration* decl = decls.first();
    AbstractType::Ptr result = decl->abstractType();
    QCOMPARE(result->toString(), type1);
    if(type2 != "nil")
    {
        decls = context->findDeclarations(QualifiedIdentifier("test2"));
        QCOMPARE(decls.size(), 1);
        Declaration* decl = decls.first();
        result = decl->abstractType();
        QCOMPARE(result->toString(), type2);
    }else
    {
        QCOMPARE(context->findDeclarations(QualifiedIdentifier("test2")).size(), 0);
    }
}

void TestDuchain::test_typeswitch()
{
    QString code("package main; type mytype int; func main() { var test1 int \n  \
                switch test2:=2; test3:=test1.(type) { case rune: test4:=4. \n \
                    case func(int) string: test5:=\'a\' \n \
                    case nil: \n  \
                    case byte, float32:  \n \
                    default:  \n } }");
    DUContext* context = getMainContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    //DUContext* ctx = context->findContextAt(CursorInRevision(1, 0)); //
    QVERIFY(context);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test1")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test2")).size(), 0);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test3")).size(), 0);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    QCOMPARE(context->findDeclarations(QualifiedIdentifier("test5")).size(), 0);
    DUContext* ctx = context->findContextAt(CursorInRevision(1, 61)); //first case
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test1")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test2")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test3")).first()->abstractType()->toString(), QString("rune"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test4")).first()->abstractType()->toString(), QString("float64"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test5")).size(), 0);
    ctx = context->findContextAt(CursorInRevision(2, 30)); //second case
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test1")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test2")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test3")).first()->abstractType()->toString(), QString("function (int) string"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test5")).first()->abstractType()->toString(), QString("rune"));
    ctx = context->findContextAt(CursorInRevision(3, 30)); //third case
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test1")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test2")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test3")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test5")).size(), 0);
    ctx = context->findContextAt(CursorInRevision(4, 30)); //fourth case
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test1")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test2")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test3")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test5")).size(), 0);
    ctx = context->findContextAt(CursorInRevision(5, 30)); //fifth case
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test1")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test2")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test3")).first()->abstractType()->toString(), QString("int"));
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test4")).size(), 0);
    QCOMPARE(ctx->findDeclarations(QualifiedIdentifier("test5")).size(), 0);
}

void TestDuchain::test_funcparams_data()
{
    QTest::addColumn<QString>("params");
    QTest::addColumn<QString>("result");

    QTest::newRow("zero params") << "()" << "function () ";
    QTest::newRow("simple param") << "(a int)" << "function (int) ";
    QTest::newRow("unnamed param") << "(int)" << "function (int) ";
    QTest::newRow("unknown param") << "(unknown)" << "function (unknown) ";
    QTest::newRow("complex param") << "([]int)" << "function (int[]) ";
    QTest::newRow("complex params") << "(*int, [10]rune)" << "function (int*, rune[]) ";
    QTest::newRow("two separate params") << "(a int, b float32)" << "function (int, float32) ";
    QTest::newRow("two combined params") << "(a, b rune)" << "function (rune, rune) ";
    QTest::newRow("two complex params") << "(a, b chan <- rune)" << "function (chan <- rune, chan <- rune) ";
    QTest::newRow("three params") << "(_, c, d float32)" << "function (float32, float32, float32) ";
    QTest::newRow("three separate params") << "(a []uint8, b *rune, c interface{})" << "function (uint8[], rune*, interface{}) ";
    QTest::newRow("return param") << "(test []mytype) *byte" << "function (main::mytype[]) byte*";
    QTest::newRow("multiple return params") << "(chan mytype) (b, e string)" << "function (chan main::mytype) (string, string)";
    QTest::newRow("multiple return params") << "(<- chan func (string)) (mytype, mytype)"
                << "function (<- chan function (string) ) (main::mytype, main::mytype)";
    QTest::newRow("three types") << "(a, b, int)" << "function (a, b, int) ";
    QTest::newRow("three types 2") << "(a, b, []int)" << "function (a, b, int[]) ";
    QTest::newRow("three types 3") << "(a, _ int, z float32) bool" << "function (int, int, float32) bool";
    QTest::newRow("return func type") << "(n int) func(p *T)" << "function (int) function (T*) ";
    QTest::newRow("variadic param") << "(a ...int) mytype" << "function (int[]) main::mytype";
    QTest::newRow("variadic param 2") << "(a, b int, z float64, opt ...interface{}) (success bool)"
                << "function (int, int, float64, interface{}[]) bool";
}

void TestDuchain::test_funcparams()
{
    QFETCH(QString, params);
    QFETCH(QString, result);
    QString code(QString("package main; type mytype int; func main%1 {  }").arg(params));
    DUContext* context = getPackageContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    Declaration* decl = context->findDeclarations(QualifiedIdentifier("main::main")).first();
    QVERIFY(decl);
    go::GoFunctionType::Ptr func = decl->abstractType().cast<go::GoFunctionType>();
    QVERIFY(func);
    QCOMPARE(func->toString(), result);
}

void TestDuchain::test_literals_data()
{
    QTest::addColumn<QString>("expr");
    QTest::addColumn<QString>("type");

    QTest::newRow("basic literal") << "1" << "int";
    QTest::newRow("basic literal 2") << ".2" << "float64";
    QTest::newRow("basic literal 3") << "1E6i" << "complex128";
    QTest::newRow("basic literal 4") << "\'y\'" << "rune";
    QTest::newRow("basic literal 5") << "\"str\"" << "string";
    QTest::newRow("named type literal") << "mytype{2}" << "main::mytype";
    QTest::newRow("named type conversion") << "mytype(2)" << "main::mytype"; //not call
    QTest::newRow("named type conversion 2") << "(mytype)(2)" << "main::mytype";//not paren expression with resolve
    QTest::newRow("named type literal 2") << "unknown{2}" << "unknown";
    QTest::newRow("struct type literal") << "struct { a int }{2}" << "struct { a int }";
    QTest::newRow("struct type literal 2") << "struct { f func(); b rune }{f:main, b:3}" << "struct { f func(); b rune }";
    QTest::newRow("struct type conversion") << "struct { mytype }(100)" << "struct { mytype }";
    QTest::newRow("struct type access") << "struct { c rune }{}.c" << "rune";
    QTest::newRow("array type literal") << "[10]int{1, 2, 3}" << "int[]";
    QTest::newRow("slice type conversion") << "[]mytype(anotherslice)" << "main::mytype[]";
    QTest::newRow("slice special form") << "[...]float32{-1., 0., 1.}" << "float32[]";
    QTest::newRow("map type literal") << "map[int]rune{}" << "map[int]rune";
    QTest::newRow("map type conversion") << "(map[int]rune)(f)" << "map[int]rune";
    QTest::newRow("map type conversion access") << "(map[int]rune)(f)[0]" << "rune";
    QTest::newRow("func type literal") << "func() int { return 0; }" << "function () int";
    QTest::newRow("func type literal 2") << "func(a, b mytype) (b bool) { c := 2; return true; }" << "function (main::mytype, main::mytype) bool";
    QTest::newRow("func type literal 3") << "func() int { var b int; }" << "function () int";
    QTest::newRow("func type conversion") << "(func())(main) " << "function () ";
    QTest::newRow("func type conversion 2") << "func() (i int)(main) " << "function () int";
    QTest::newRow("func type call") << "func(f []int) float64 {} ( []int{1, 2} )" << "float64";
    QTest::newRow("pointer") << "*mytype(&v) " << "main::mytype*";
    QTest::newRow("pointer type conversion") << "(*mytype)(v) " << "main::mytype*";
    QTest::newRow("pointer type conversion") << "(*unnamed)(v) " << "unnamed*";
    QTest::newRow("interface type conversion") << "interface{}(main) " << "interface{}";
    QTest::newRow("interface type conversion 2") << "interface{ Read(a int) string }(Reader) " << "interface{ Read(a int) string }";
    QTest::newRow("chan") << "<-chan int(c) " << "int";
    QTest::newRow("chan conversion") << "(<-chan int)(c) " << "<- chan int";
    QTest::newRow("chan conversion 2") << "chan<- mytype(tt) " << "chan <- main::mytype";
}

void TestDuchain::test_literals()
{
    QFETCH(QString, expr);
    QFETCH(QString, type);
    QString code(QString("package main; type mytype int; func main() { test := %1 }").arg(expr));
    DUContext* context = getMainContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    auto decls = context->findDeclarations(QualifiedIdentifier("test"));
    QCOMPARE(decls.size(), 1);
    QCOMPARE(decls.first()->abstractType()->toString(), type);
}


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

DUContext* getMainContext(const QString& code)
{
    DUContext* package = getPackageContext(code);
    if(!package)
	return 0;
    DUChainReadLocker lock;
    auto decls = package->findDeclarations(QualifiedIdentifier("main"));
    if(decls.size() == 0)
	return 0;
    AbstractFunctionDeclaration* function = dynamic_cast<AbstractFunctionDeclaration*>(decls.first());
    if(!function)
	return 0;
    return function->internalFunctionContext();
}


#include "testduchain.moc"
