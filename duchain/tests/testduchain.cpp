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

#include "testduchain.moc"
