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
#include "declarationbuilder.h"
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
    QVERIFY(session.startParsing());
    DeclarationBuilder builder(&session, false);
    ReferencedTopDUContext context =  builder.build(IndexedString(""), session.ast());
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



DUContext* getPackageContext(const QString& code)
{
    ParseSession session(code.toUtf8(), 0);
    if(!session.startParsing())
	return 0;
    DeclarationBuilder builder(&session, false);
    ReferencedTopDUContext context =  builder.build(IndexedString(""), session.ast());
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
