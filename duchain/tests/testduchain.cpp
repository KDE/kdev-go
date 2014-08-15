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

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testhelpers.h>

QTEST_KDEMAIN(TestDuchain, NoGUI);

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
}


void TestDuchain::builtinFunctions()
{
    QFETCH(QString, expression);
    QFETCH(QString, type);

    QString code(QString("package main; type mytype int; func main() { testvar := %1; }").arg(expression));

    DUContext* context = getMainContext(code);
    QVERIFY(context);
    DUChainReadLocker lock;
    auto decls = context->findDeclarations(QualifiedIdentifier("testvar"));
    QCOMPARE(decls.size(), 1);
    Declaration* decl = decls.first();
    AbstractType::Ptr result = decl->abstractType();

    QCOMPARE(result->toString(), type);
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
