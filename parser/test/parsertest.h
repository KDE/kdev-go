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

#ifndef GOPARSERTEST
#define GOPARSERTEST

#include <QTest>
#include <QObject>

namespace go {

class Lexer;
class AstNode;

class ParserTest : public QObject
{
  Q_OBJECT
public:
  ParserTest();
  
private slots:
  void testKeyWords();
  void testOperators();
  void testRunes();
  void testNumbers();
  void testNumbers_data();
  void testCommentsAreIgnored();
  void testCommentsAreIgnored_data();
  void testMultiLineStrings();
  void testMultiLineStrings_data();
  void testBasicTypes();
  void testIfClause();
  void testFuncTypes();
  void testForRangeLoop();
  void testForSingleConditionalLoop();
  void testForWithForClauseLoop();
  void testForWithEmptySingleConditionLoop();
  void testShortVarDeclaration();
  void testShortVarDeclaration_data();
  void testEmptyLabeledStmt();
  void testMapKeyLiteralValue(); //Go 1.5 feature
private:
  QByteArray getCodeFromNode(const QByteArray &code, go::Lexer *lexer, go::AstNode *node);
};

}

#endif