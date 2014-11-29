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

#include "parsertest.h"

#include <QTest>
#include "parser/golexer.h"
#include "parser/goparser.h"
#include "parser/godebugvisitor.h"
#include "parser/gotokentext.h"
#include "parsesession.h"


QTEST_MAIN(go::ParserTest)
namespace go {
ParserTest::ParserTest() {}

void ParserTest::testKeyWords()
{
    QByteArray code = "break default func interface select case defer go map struct \
      chan else goto package switch const fallthrough if range type continue for \
      import return var";
    KDevPG::QByteArrayIterator iter(code);
    Lexer lexer(iter);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_BREAK);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_DEFAULT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_FUNC);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_INTERFACE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_SELECT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_CASE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_DEFER);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_GO);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MAP);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_STRUCT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_CHAN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_ELSE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_GOTO);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_PACKAGE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_SWITCH);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_CONST);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_FALLTHROUGH);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_IF);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RANGE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_TYPE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_CONTINUE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_FOR);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_IMPORT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RETURN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_VAR);
}

void ParserTest::testOperators()
{
    QByteArray code = "+ & += &= && == != ( ) - | -= |= || < <= [ ] * ^ *= ^= <- > >= { } \
    / << /= <<= ++ = := , ; % >> %= >>= -- ! ... . : &^ &^=";
    KDevPG::QByteArrayIterator iter(code);
    Lexer lexer(iter);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_PLUS);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_AMPERSAND);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_PLUSEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_AMPEREQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LOGICALAND);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_ISEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_ISNOTEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LPAREN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RPAREN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MINUS);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_BITWISEOR);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MINUSEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_OREQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LOGICALOR);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LESS);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LESSOREQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LBRACKET);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RBRACKET);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_STAR);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_HAT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MULTIPLYEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_XOREQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LEFTCHAN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_GREATER);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_GREATEROREQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LBRACE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RBRACE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_DIVIDE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LEFTSHIFT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_DIVIDEEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_LEFTSHIFTEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_PLUSPLUS);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_ASSIGN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_AUTOASSIGN);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_COMMA);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_SEMICOLON);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MOD);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RIGHTSHIFT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MODEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RIGHTSHIFTEQUAL);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_MINUSMINUS);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_BANG);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_TRIPLEDOT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_DOT);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_COLON);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_AMPERXOR);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_AMPERXOREQUAL);
}


void ParserTest::testRunes()
{
    //@TODO stuff like ''' shouldn't actually be accepted, try fixing rune lexer rule in go.g
    QByteArray code = "'\\'   '\"'   '~'   '_'  '0'  'a'  '\''   'щ'  '''  ";
    KDevPG::QByteArrayIterator iter(code);
    Lexer lexer(iter);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    QVERIFY( lexer.read().kind == TokenTypeWrapper::Token_RUNE);
    
}

void prepareParser(const QByteArray& code, go::Parser** parser, go::Lexer** lexer)
{
    KDevPG::TokenStream tokenStream;
    *parser = new go::Parser(); 
    (*parser)->setMemoryPool(new KDevPG::MemoryPool());
    KDevPG::QByteArrayIterator iter(code);
    *lexer = new go::Lexer(iter);
    (*parser)->setTokenStream(*lexer);
}

void ParserTest::testBasicTypes()
{
    QByteArray code = "struct { array [5][2]string; slice []*int; \
	  x, y int \n A *[]int \n F func() \n T1 \n *T2 \n P.T3; \
	  *P.T4; afk func(a, b int, z float64, opt ...interface{}) (success bool); \
	  afk2 func(int, int) bool; afk3 func(n int) func(p *T); }";
   //QByteArray code = "int";
    go::Parser parser;
    KDevPG::MemoryPool pool;
    parser.setMemoryPool(&pool);
    KDevPG::QByteArrayIterator iter(code);
    go::Lexer lexer(iter);
    parser.setTokenStream(&lexer);
    //parser.rewind(0);
    //while((kind = lexer.read().kind) != TokenTypeWrapper::Token_EOF)
	//qDebug() << tokenText(kind);
    //}
    
    parser.rewind(0);
    go::TypeAst* ast;
    bool result=parser.parseType(&ast);
    QVERIFY(result);
    //go::DebugVisitor visitor(&lexer, code);
    //visitor.visitNode(ast);
    QString content = code;
    const KDevPG::ListNode <go::FieldDeclAst*> *node = ast->complexType->structType->fieldDeclSequence->front();
    //qDebug() << code.mid(lexer.at(node->element->startToken).begin, lexer.at(node->element->endToken).end - lexer.at(node->element->startToken).begin+1);
    QVERIFY( code.mid(lexer.at(node->element->startToken).begin, lexer.at(node->element->endToken).end - lexer.at(node->element->startToken).begin+1) 
      == "array [5][2]string");
  
}


void ParserTest::testIfClause()
{
    QByteArray code = "if a:=(A{2}); a==(A{2}) { a += A{1}; }\n \
		  if ID(f()) { g(); }\n \
		  if ([]int)(A)==hello {} \n \
		  if (a)(A{3}) {} \n \
		  if ([]a)(A{3}) {} \n \
		  if x := f(); x < y { abc; def(); } \n \
		  if ([]a)(A{3}); (a)(A{f}) {} \n";
   //QByteArray code = "int";
    go::Parser parser;
    KDevPG::MemoryPool pool;
    parser.setMemoryPool(&pool);
    KDevPG::QByteArrayIterator iter(code);
    go::Lexer lexer(iter);
    parser.setTokenStream(&lexer);
    parser.rewind(0);
    go::StatementsAst* ast;
    bool result=parser.parseStatements(&ast);
    QVERIFY(result);
    //go::DebugVisitor visitor(&lexer, code);
    //visitor.visitNode(ast);
}

void ParserTest::testFuncTypes()
{
    QByteArray code = "func () {} \n \
	      func(a) {} \n \
	      func(a,) {} \n \
	      func(a int,) {}\n \ 
	      func(a, b []int){} \n \
	      func(*char, [2]int, f chan float64){} \n \
	      func(nums ...int, ){} \n \ "; 
	      
   //QByteArray code = "int";
    go::Parser parser;
    KDevPG::MemoryPool pool;
    parser.setMemoryPool(&pool);
    KDevPG::QByteArrayIterator iter(code);
    go::Lexer lexer(iter);
    parser.setTokenStream(&lexer);
    parser.rewind(0);
    go::StatementsAst* ast;
    bool result=parser.parseStatements(&ast);
    QVERIFY(result);
    //go::DebugVisitor visitor(&lexer, code);
    //visitor.visitNode(ast);
}

void ParserTest::testForRangeLoop()
{
    QString code = "package main; func main() { str := \"string\"; for range str { c := 2 } } ";
    ParseSession session(code.toUtf8(), 0, true);
    QVERIFY(session.startParsing());
}



//add this tests:
//go func(ch chan<- bool) {for {sleep(10); ch <- true; }}(c);

//select {
//  case i1 = <-c1:
//    print("asdas", i3, "from c3\n");
//  case c2 <- i2:
//	f(); g();
 
}

#include  "parsertest.moc"