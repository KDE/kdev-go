----------------------------------------------------------------------------
--    Copyright 2014 by Pavel Petrushkov <onehundredof@gmail.com>
--    This program is free software; you can redistribute it and/or modify
--    it under the terms of the GNU Library General Public License as
--    published by the Free Software Foundation; either version 2 of the
--    License, or (at your option) any later version.
--
--    This program is distributed in the hope that it will be useful,
--    but WITHOUT ANY WARRANTY; without even the implied warranty of
--    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--    GNU General Public License for more details.
--
--    You should have received a copy of the GNU Library General Public
--    License along with this program; if not, write to the
--    Free Software Foundation, Inc.,
--    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
-----------------------------------------------------------------------------

--This grammar is based on official Go language specification http://golang.org/ref/spec
--All first/first and first/follow conflicts are hidden behind try/rollbacks and LA's
--I was trying to minimize usage of try/rollbacks, so some rules are very ugly for this reason
--Before changing grammar consider contacting me first at onehundredof@gmail.com, or at least
--run parser tests and test it manually on Go standart library

--@Warning because this grammar uses some manual flags(inIfClause and inSwitchTypeClause) be careful with changing try/rollback order, 
--because when parser fails and rollbacks to catch statement flags may not be reset

[: namespace KDevelop {
    class DUContext;
    }
:]

%token_stream Lexer;

%input_encoding "UTF-8"
%input_stream "KDevPG::QByteArrayIterator"
%lexer_bits_header "QDebug"
%parser_bits_header "QDebug"
%parser_declaration_header "language/duchain/duchain.h"

%export_macro "KDEVGOPARSER_EXPORT"
%export_macro_header "goparserexport.h"


%ast_extra_members
[:
    KDevelop::DUContext* ducontext;
:]

%parserclass (private declaration)
[:
   struct ParserState {
    };
    ParserState m_state;

    qint64 lparenCount=0;
    bool inIfClause = false;
    bool inSwitchTypeClause = false;
:]


%token IDENT, INTEGER, FLOAT, COMPLEX, RUNE, STRING, TEST;;


%token BREAK ("break"), CASE ("case"), CHAN ("chan"), CONST ("const"), CONTINUE ("continue"),
    DEFAULT ("default"), DEFER ("defer"), ELSE ("else"), FALLTHROUGH ("fallthrough"), 
    FOR ("for"), FUNC ("func"), GO ("go"), GOTO ("goto"), IF ("if"), IMPORT ("import"),
    INTERFACE ("interface"), MAP ("map"), PACKAGE ("package"), RANGE ("range"), 
    RETURN ("return"), SELECT ("select"), STRUCT ("struct"), SWITCH("switch"), 
    TYPE ("type"), VAR ("var");;
    
%token PLUS ("+"), AMPERSAND ("&"), PLUSEQUAL("+="), AMPEREQUAL ("&="), LOGICALAND ("&&"), ISEQUAL ("=="), ISNOTEQUAL ("!="), LPAREN ("("), 
     RPAREN (")"), MINUS ("-"), BITWISEOR ("|"), MINUSEQUAL ("-="), OREQUAL ("|="), LOGICALOR ("||"), LESS ("<"), LESSOREQUAL ("<="), 
     LBRACKET ("["), RBRACKET ("]"), STAR ("*"), HAT ("^"), MULTIPLYEQUAL ("*="), XOREQUAL ("^="), LEFTCHAN ("<-"), GREATER (">"), 
     GREATEROREQUAL (">="), LBRACE ("{"), RBRACE ("}"), DIVIDE ("/"), LEFTSHIFT ("<<"), DIVIDEEQUAL ("/="), LEFTSHIFTEQUAL ("<<="), 
     PLUSPLUS ("++"), ASSIGN ("="), AUTOASSIGN (":="), COMMA (","), SEMICOLON (";"), MOD ("%"), RIGHTSHIFT (">>"), MODEQUAL ("%="), 
     RIGHTSHIFTEQUAL (">>="), MINUSMINUS ("--"), BANG("!"), TRIPLEDOT ("..."), DOT ("."), COLON (":"), AMPERXOR ("&^"), AMPERXOREQUAL ("&^=");;
     
%lexer -> 
  
------------------------------------------------------------
--comments

-- one liner
  "//"[.^\n]*				[: /*locationTable()->newline(lxCURR_IDX);*/  :]	;
  
-- multi line
"/*"  [: /*std::cout << "entering comment\n";*/ lxSET_RULE_SET(incomment); :] ;

--broken
-- "/*"[.^"/*"]*"*/"			[: std::cout << "multiline comment\n"; :]	;
-- /\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/       [: /*std::cout << "multiline comment\n";*/ :] 	;

------------------------------------------------------------
--keywords
"break"    		BREAK;
"case"        		CASE; 
"chan"        		CHAN; 
"const"       		CONST; 
"continue"    		CONTINUE; 
"default"     		DEFAULT; 
"defer"       		DEFER; 
"else"        		ELSE; 
"fallthrough" 		FALLTHROUGH; 
"for"         		FOR;
"func"        		FUNC; 
"go"          		GO; 
"goto"        		GOTO; 
"if"          		IF; 
"import"      		IMPORT; 
"interface"   		INTERFACE; 
"map"          		MAP;
"package"     		PACKAGE; 
"range"       		RANGE; 
"return"      		RETURN; 
"select"		SELECT; 
"struct"		STRUCT;
"switch"		SWITCH;
"type"			TYPE;
"var"			VAR;
 
------------------------------------------------------------------
-- operators 
"+" PLUS;
"&" AMPERSAND;
"+=" PLUSEQUAL;
"&=" AMPEREQUAL;
"&&" LOGICALAND;
"==" ISEQUAL;
"!=" ISNOTEQUAL;
"(" LPAREN;
")" RPAREN;
"-" MINUS;
"|" BITWISEOR;
"-=" MINUSEQUAL;
"|=" OREQUAL;
"||" LOGICALOR;
"<" LESS;
"<=" LESSOREQUAL;
"[" LBRACKET;
"]" RBRACKET;
"*" STAR;
"^" HAT;
"*=" MULTIPLYEQUAL;
"^=" XOREQUAL;
"<-" LEFTCHAN;
">" GREATER;
">=" GREATEROREQUAL;
"{" LBRACE;

--@TODO Go language specification allows omitting semicolons before closing ")" or "}"
--so expressions like struct{ x, y float64 } are legal
--I'm not sure how exactly these rules should be applied so for now I just
--insert semicolon in the same conditions as newline
--another example is { for {} } or  {return "", 0 }
"}"  [: int prevkind = at(size()-1).kind;  if(prevkind == Token_IDENT || prevkind == Token_INTEGER || prevkind == Token_FLOAT
			    || prevkind == Token_COMPLEX || prevkind == Token_RUNE || prevkind == Token_STRING
			    || prevkind == Token_BREAK || prevkind == Token_CONTINUE || prevkind == Token_FALLTHROUGH
			    || prevkind == Token_RETURN || prevkind == Token_PLUSPLUS || prevkind == Token_MINUSMINUS
			    || prevkind == Token_RPAREN || prevkind == Token_RBRACKET || prevkind == Token_RBRACE)
				lxTOKEN(SEMICOLON);   :]	RBRACE;



"/" DIVIDE;
"<<" LEFTSHIFT;
"/=" DIVIDEEQUAL;
"<<=" LEFTSHIFTEQUAL;
"++" PLUSPLUS;
"=" ASSIGN;
":=" AUTOASSIGN;
"," COMMA;
";" SEMICOLON;
"%" MOD;
">>" RIGHTSHIFT;
"%=" MODEQUAL;
">>=" RIGHTSHIFTEQUAL;
"--" MINUSMINUS;
"!" BANG;
"..." TRIPLEDOT;
"." DOT;
":" COLON;
"&^" AMPERXOR;
"&^=" AMPERXOREQUAL;

--identifier---------------------------------------------------
 ({alphabetic}|"_")({alphabetic}|[0-9]|"_")*		IDENT;
--integers-----------------------------------------------------
 ([1-9][0-9]*)|("0"[0-7]*)|("0"("x"|"X")[0-9a-fA-F]+)		INTEGER;
--floats-------------------------------------------------------
  (("e"|"E")("+"|"-")[0-9]+)|(("e"|"E")[0-9]+)   -> exponent;

  ([0-9]+"."[0-9]*) | ([0-9]+"."[0-9]*{exponent}) | ([0-9]+{exponent})
      | ("." [0-9]+) | ("."[0-9]+{exponent}) 	 -> float_literal;
  {float_literal} 						FLOAT;
--complex numbers-------------------------------------------------------
  ([0-9]+ | {float_literal})"i"					COMPLEX;
--rune literals---------------------------------------------------------
--apparently '\\' should be parsed as rune which we will have to write as "\\\\"

  --for some reason these runes don't work without this rules
  ("\'\\\\\'") | ("\'\\\'\'") | ("\'\\\"\'")	RUNE;
  
  ("\\"[0-7][0-7][0-7])
     | ("\\x"[0-9a-fA-F][0-9a-fA-F]) 			->	bbyte_value;
  ("\\u"[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]) |
    ("\\U"[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]
      [0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]) |
     -- ("\\a")|("\\b")|("\\f")|("\\n")|("\\r")|("\\t")|("\\v")|("\\\\")|("\\\'")|("\\\"") | ({alphabetic} | {num} | [.^\'] )		-> unicode_value;
     ("\\a")|("\\b")|("\\f")|("\\n")|("\\r")|("\\t")|("\\v")|("\\\\")|("\\\'")|("\\\"") | ({alphabetic} | {num} | . )		-> unicode_value;
     
--because of problems with utf-8 add this rule for now
--disregard that, utf8 works fine
[.^\']*		-> utf8helper;

  "\'"({bbyte_value}|{unicode_value})"\'"				RUNE;
--  "\'"({utf8helper})"\'"				RUNE;

--strings----------------------------------------------------------------
--This works well
( [.^(\" | \\)] | \\.)	->string_internal;
-- ( [.^(\` | \\)] | \\.)	->string_internal2; --I'm not sure if you can escape backticks
  ("\"" ({string_internal})* "\"")  			 STRING;
  "\`" [: lxSET_RULE_SET(instring); :]                 STRING;

  
  
--broken
--  ("\""[.^\"]*"\"") | ("\`"[.^\`]*"\`")							STRING;
--  ("\"" ([.^\"] | "\\\"")* "\"") | ("\`" ([.^\`] | "\\\`" ) *"\`")				STRING;
-- @TODO I'm not really sure but in src/cmd/go/main.go there are `\` and `/` expressions
-- Does this mean you can't escape backtick? 
--This regexp below failed to recognize strings like "\\"
--  ("\"" ([.^\"] | "\\\"")* "\"") | ("\`" [.^\`] *"\`")					STRING;

--blanks, tabs and newlines----------------------------------------------
 [\ \t]+                     [: /* blank */ :] ;

-- Go language specification requires inserting semicolons when met this conditions:
			   
 \n			[:  locationTable()->newline(lxCURR_IDX);  
			    if(size() >= 1) { int prevkind = at(size()-1).kind; /*qint64 prevline, prevcol; endPosition(size()-1,&prevline, &prevcol); */
			  /*std::cout << "new line detected:" << at(size()-1).kind << prevline << std::endl;*/
			  if(prevkind == Token_IDENT || prevkind == Token_INTEGER || prevkind == Token_FLOAT
			    || prevkind == Token_COMPLEX || prevkind == Token_RUNE || prevkind == Token_STRING
			    || prevkind == Token_BREAK || prevkind == Token_CONTINUE || prevkind == Token_FALLTHROUGH
			    || prevkind == Token_RETURN || prevkind == Token_PLUSPLUS || prevkind == Token_MINUSMINUS
			    || prevkind == Token_RPAREN || prevkind == Token_RBRACKET || prevkind == Token_RBRACE)
			  lxRETURN(SEMICOLON); } :]	;
			  
--output TEST on an unknown symbol so Lexer wouldn't crash
.	TEST;
 ;

--special rule set for multilined comments
%lexer "incomment" ->
 "*/"	[: /*std::cout << "leaving comment\n";*/ lxSET_RULE_SET(start); :] 	;
 
 \n			[: locationTable()->newline(lxCURR_IDX); :]	;

 .	;
 ;
%lexer "instring" ->
 \n            [: locationTable()->newline(lxCURR_IDX); :]     ;
 "\`"          [: lxSET_RULE_SET(start); :] ;
 .         ;
 ;

------------------------------------------------------------------------- 
------------------------------------------------------------------------- 
------------------------------------------------------------------------- 
----------------------GRAMMAR-------------------------------------------- 
------------------------------------------------------------------------- 
------------------------------------------------------------------------- 
 
sourceFile=sourceFile
-> start;;
 
--Types-----------------------------------------------------------------
------------------------------------------------------------------------
------------------------------------------------------------------------

--resolving FIRST/FIRST conflict in typeName: IDENT and qualifiedIdent (e.g. math.complex) intersect
--type_resolve is part of qualifiedIdent
DOT fullName=identifier | 0
->type_resolve;;

name=identifier type_resolve=type_resolve
-> typeName;;

typeName=typeName
| parenType=parenType
| complexType=complexType
-> type;;

LPAREN type=type RPAREN
-> parenType;;

--separation of typeName, parenType and complexType is desired for parameterList
( arrayOrSliceType=arrayOrSliceType | structType=structType | pointerType=pointerType | functionType=functionType
| interfaceType=interfaceType | mapType=mapType | chanType=chanType )
-> complexType;;


--Array and Slice-------------------------------------------------------------------

--resolving FIRST/FIRST conflict in arrayType/sliceType (both start with "[")
LBRACKET arrayOrSliceResolve=arraySliceResolve
-> arrayOrSliceType;;

RBRACKET slice=type						-- <- this is slice
| expression=expression RBRACKET array=type			-- <- this is array
-> arraySliceResolve;;

--Struct---------------------------------------------------------------------------

STRUCT LBRACE (#fieldDecl=fieldDecl SEMICOLON)* RBRACE
-> structType;;

--to resolve first/first between field declaration and anonymous field:
STAR anonFieldStar=anonymousField (tag=tag | 0)
| varid=identifier ( (idList=idList | 0) type=type | DOT fullname=identifier | 0) (tag=tag | 0)
-> fieldDecl;;

typeName=typeName
-> anonymousField;; --anonymous field starting with "*"

STRING
-> tag;;

(COMMA #id=identifier)+
-> idList;;

STAR type=type
->pointerType;;

--Function Type-----------------------------------------------------
--parsing parameter list is a bit weird, because Go allows putting a comma after all parameters,
--like so: func(a int, b float64,)
--so writing just "(identifier type) @ COMMA" won't work
--another problem is anonymous parameters like: func(int, float64) and combined parameter names: func(a, b int)
--when we encounter first identifier it is unclear whether it would be a parameter name
--or parameter type, we will have to decide that later

FUNC signature=signature
-> functionType;;

parameters=parameters (result=result | 0)
-> signature;;

parameters=parameters | typeName=typeName | complexType=complexType
-> result;;

LPAREN (parameter=parameter (COMMA (#parameterList=parameter | 0))* | 0 ) RPAREN
->parameters;;

complexType=complexType
| parenType=parenType
| TRIPLEDOT unnamedvartype=type
| idOrType=identifier ( type=type | TRIPLEDOT vartype=type | DOT fulltype=identifier | 0 )
->parameter;;

--Interfaces------------------------------------------------------------

INTERFACE LBRACE (#methodSpec=methodSpec SEMICOLON)* RBRACE
-> interfaceType;;

--		  	Read(a)			fmt.Reader	    Read  <-interface name
methodName=identifier (signature=signature | DOT fullName=identifier | 0)
->methodSpec;;

--Map Type--------------------------------------------------------------

MAP LBRACKET keyType=type RBRACKET elemType=type
-> mapType;;

--Channel Type----------------------------------------------------------

--Actually grammar here is ambiguous
--e.g. chan <- chan. Is that channel to sending channel or receiving channel to channel?
--language associates leftmost <- hence the order of tokens in the next rule:
CHAN (send=LEFTCHAN | 0 ) rtype=type
| LEFTCHAN CHAN stype=type
-> chanType;;



--Declarations----------------------------------------------------------
------------------------------------------------------------------------
------------------------------------------------------------------------

( declaration=declaration | FUNC (funcDecl=funcDeclaration | methodDecl=methodDeclaration) )
-> topLevelDeclaration;;

( constDecl=constDecl | typeDecl=typeDecl | varDecl=varDecl )
-> declaration;;

--Const Declarations----------------------------------------------------
CONST ( constSpec=constSpec | LPAREN (#constSpecList=constSpec SEMICOLON)* RPAREN )
-> constDecl;;

id=identifier (idList=idList | 0) ((type=type | 0) ASSIGN expression=expression (expressionList=expressionList | 0) | 0)
-> constSpec;;

--Type Declarations-----------------------------------------------------

TYPE ( typeSpec=typeSpec | LPAREN (#typeSpecList=typeSpec SEMICOLON)* RPAREN )
-> typeDecl;;

name=identifier type=type
-> typeSpec;;

--Var Declarations------------------------------------------------------
VAR ( varSpec=varSpec | LPAREN (#varSpecList=varSpec SEMICOLON)*  RPAREN)
-> varDecl;;

id=identifier (idList=idList | 0) (type=type (ASSIGN expression=expression ( expressionList=expressionList | 0 ) | 0)
| ASSIGN expression=expression ( expressionList=expressionList | 0 ) )
-> varSpec;;

--Func Declaration-------------------------------------------------------

funcName=identifier signature=signature (body=block | 0)
-> funcDeclaration;;

--Method Declaration-----------------------------------------------------
methodRecv=methodRecv methodName=identifier signature=signature (body=block | 0)
->methodDeclaration;; 

LPAREN ( nameOrType=identifier (star=STAR | 0) (type=identifier | 0) | star=STAR ptype=identifier )  RPAREN
->methodRecv;;


--Expressions---------------------------------------------------------------------------
----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------

--Operand-------------------------------------------------------------------------------

--@Warning Original Go Language specification contains operand rules(see http://golang.org/ref/spec#Operands), 
--but I decided to go with potentially horrible decision:
--eliminate operand rules and do one huge rule in Primary Expression
--Idea here is that operand only used by primary expression and it has 
--many overlaps(first/first) with primary expression, so it's better
--to look at everything in primary expression itself rather than trying LA and rollbacks

--Basic literals(integers, strings...)

integer=INTEGER
| flt=FLOAT
| cpx=COMPLEX
| rune=RUNE
| string=STRING
-> basicLit;;

--what is this?
--id=IDENT type_resolve=type_resolve
--->operandName;;

--Composite Literals

--@Warning @TODO semicolon is not in language specification
--I added it because I insert semicolon before closing }
--because language specification allows omitting it(see SEMICOLON lexer rule)
LBRACE (element=element | 0) (COMMA (#elements=element | 0))* (SEMICOLON | 0) RBRACE
-> literalValue;;

--if you look at language specification this may be confusing
--point here is that when parser sees IDENT(or Expression)
--it cannot decide without LA if it's key or value
--so we just save whatever there is and when traversing AST later
--we decide what is what by looking at value node
keyOrValue=keyOrValue (COLON value=value | 0 ) 
| literalValue=literalValue				--in this case it's just value
-> element;;

indexOrValue=expression
-> keyOrValue;;

expValue=expression
| literalValue=literalValue
-> value;;

--Primary Expressions-------------------------------------------------------

--if after looking for some time at next rule you want to ask why couldn't I just write type instead of all type redeclarations,
--that's because literals of some types are impossible(like interface)

--first/first with parenthesized expression and type conversion resolved with try/rollback
--about inIfClause and lparenCount see ifStmt rule
--unmatched lparenCount are there becase of try/rollback

--TODO should I add lparenCount++ after LBRACKET in array/slice conversion/literal?
--that may mess up things like: if [mystruct{3}]int{10} { } 

id=identifier ( ?[: !inIfClause || (inIfClause && lparenCount > 0) :] literalValue=literalValue 
  |  LPAREN [: if(inIfClause) lparenCount++; :] callOrBuiltinParam=callOrBuiltinParam [: if(inIfClause) lparenCount--; :] RPAREN | 0)
   ( primaryExprResolve=primaryExprResolve | 0 ) 			--named literal
| basicLit=basicLit ( primaryExprResolve=primaryExprResolve | 0 ) --(primaryExprResolve=primaryExprResolve | 0)	not sure why I wrote this twice	--basic literal
| structType=structType ( literalValue=literalValue | convArg=conversionArgument )	(primaryExprResolve=primaryExprResolve | 0)	--struct conversion/literal
| LBRACKET ( arrayOrSliceResolve=arraySliceResolve  (literalValue=literalValue | convArg=conversionArgument)
 | TRIPLEDOT RBRACKET element=type literalValue=literalValue )(primaryExprResolve=primaryExprResolve | 0)				--array/slice conversion/literal
| mapType=mapType ( literalValue=literalValue | convArg=conversionArgument )(primaryExprResolve=primaryExprResolve | 0)	--map type conversion/literal
| FUNC signature=signature (body=block | convArg=conversionArgument )(primaryExprResolve=primaryExprResolve | 0) 		--func type conversion/literal
| pointerType=pointerType convArg=conversionArgument (primaryExprResolve=primaryExprResolve | 0) 		--pointer type conversion
| interfaceType=interfaceType convArg=conversionArgument  (primaryExprResolve=primaryExprResolve | 0)		--interface type conversion
| chanType=chanType convArg=conversionArgument (primaryExprResolve=primaryExprResolve | 0)			--chan type conversion
| try/rollback(LPAREN [: if(inIfClause) lparenCount++; :] expression=expression [: if(inIfClause) lparenCount--; :] RPAREN ) 		--@TODO is this useless?(change order)
 catch([: if(inIfClause) lparenCount--; :] parenType=parenType convArg=conversionArgument ) (primaryExprResolve=primaryExprResolve | 0)		--parenthesized type conversion
-> primaryExpr;;

LPAREN [: if(inIfClause) lparenCount++; :] expression=expression (COMMA | 0) [: if(inIfClause) lparenCount--; :] RPAREN
->conversionArgument;;


--allow first argument to be 'type' so builtin functions could work
--TripleDot is to allow special form of append call: append(s1, s2...)
try/rollback(expression=expression) catch(type=type) (COMMA (#expressions=expression | 0))* (tripleDot=TRIPLEDOT | 0)
| 0
->callOrBuiltinParam;;

--@TODO apparently not only in append there can be such construction
--e.g. go/cmd/go/test.go:926 exec.Command(args[0], args[1:]...)
expression=expression (COMMA (#expressions=expression | 0))* (tripleDot=TRIPLEDOT | 0)
| 0
->callParam;;


--first is selector/type typeAssertion
--second is array/slice index
--inSwitchTypeClause is true if we are looking for type switch statement ( switch x.(type) {} )
--in which case we must prevent primaryExpression from eating up extra DOT (which will fail later anyway)
--another bug is complex-named literals like package.typename{}
--the way we deal with that is add literalValue to this primaryExprResolve rule
--inIfClause makes sure rule isn't invoked when parsing if clauses
( [: if(inSwitchTypeClause && LA(1).kind==Token_DOT && LA(2).kind==Token_LPAREN && LA(3).kind==Token_TYPE) 
{ (*yynode)->endToken = tokenStream->index() - 2; return true;} :]
DOT (selector=identifier | LPAREN typeAssertion=type RPAREN ) (primaryExprResolve=primaryExprResolve | 0 ) )
--increase lParenCounter within brackets to allow literals within brackets in if clauses
| index=LBRACKET [: if(inIfClause) lparenCount++; :] ( ( low=expression (colon=COLON | 0) | colon=COLON ) ( high=expression (COLON max=expression | 0) | 0 ))
[: if(inIfClause) lparenCount--; :] RBRACKET (primaryExprResolve=primaryExprResolve | 0)
| LPAREN [: if(inIfClause) lparenCount++; :] callParam=callParam [: if(inIfClause) lparenCount--; :] RPAREN (primaryExprResolve=primaryExprResolve | 0)
| [: if(inIfClause && lparenCount <= 0) { (*yynode)->endToken = tokenStream->index() - 2; return true;} :] literalValue=literalValue 
    (primaryExprResolve=primaryExprResolve | 0) 	--@TODO this isn't in language specification. It's a way to deal with complex-named literals e.g. "a.b{1}"
-> primaryExprResolve;;


--Expression------------------------------------------------------------------------
------------------------------------------------------------------------------------

unaryExpression=unaryExpression ( binary_op=binary_op expression=expression | 0)
-> expression;;

try/rollback( primaryExpr=primaryExpr | unary_op=unary_op unaryExpression=unaryExpression )
catch (unsafe_unary_op=unsafe_unary_op unaryExpression=unaryExpression )
-> unaryExpression;;

plus=PLUS | minus=MINUS | bang=BANG | hat=HAT | ampersand=AMPERSAND
-> unary_op;;

--unsafe here means it's conflicting with primaryExpression and may trigger rollback
star=STAR | leftchan=LEFTCHAN
-> unsafe_unary_op;;

logicalor=LOGICALOR | logicaland=LOGICALAND | rel_op=rel_op | add_op=add_op | mul_op=mul_op
-> binary_op;;

isequal=ISEQUAL | isnotequal=ISNOTEQUAL | less=LESS | lessorequal=LESSOREQUAL | greater=GREATER | greaterorequal=GREATEROREQUAL
-> rel_op;;

plus=PLUS | minus=MINUS | bitwiseor=BITWISEOR | hat=HAT
-> add_op;;

star=STAR | divide=DIVIDE | mod=MOD | leftshift=LEFTSHIFT | rightshift=RIGHTSHIFT | ampersand=AMPERSAND | amperxor=AMPERXOR
-> mul_op;;


--Statements-----------------------------------------------------------------
------------------------------------------------------------------------
------------------------------------------------------------------------
LBRACE (statements=statements | 0) RBRACE
-> block;;

(#statement=statement SEMICOLON)*
-> statements;;


--conflicts:
--FIRST/FIRST between simpleStmt.expression and labeledStmt (IDENT) - resolved by try/rollback
--@TODO consider resolving ^ this conflict with LA for better performance
--FIRST/FIRST between simpleStmt.expression and shortVarDecl (IDENT) - resolved bt try/rollback

--@Warning there is a dangerous bug with the try/rollback usage
--if try/rollback expression succedes catch rule won't be evaluated even if parser fails later
--this means if catch expression is sub-expression of try/rollback expression then 
--we are likely to hit this bug
--example: a simple IDENT is a valid simpleStmt, so if we have a labeledStmt in the catch
--we will parse a simpleStmt and then fail looking for ";" instead of ":"

declaration=declaration
| ifStmt=ifStmt
| switchStmt=switchStmt
| forStmt=forStmt
| goStmt=goStmt
| selectStmt=selectStmt
| ( returnStmt=returnStmt | breakStmt=breakStmt | continueStmt=continueStmt | gotoStmt=gotoStmt)
| ( fallthroughStmt=fallthroughStmt | deferStmt=deferStmt )
| block=block
| try/rollback(labeledStmt=labeledStmt)
  catch(simpleStmt=simpleStmt)
| 0				--Empty statement is part of language specification
-> statement;;

label=identifier COLON statement=statement
-> labeledStmt;;

try/rollback(shortVarDecl=shortVarDecl)
  catch( expression=expression ( simpleStmtResolve=simpleStmtResolve | 0 ))
-> simpleStmt;;

LEFTCHAN expression=expression
| increment=PLUSPLUS
| decrement=MINUSMINUS
| ( assigneeList=expressionList | 0 ) assign_op=assign_op expression=expression ( assignmentList=expressionList | 0 )
->simpleStmtResolve;;

(COMMA #expressions=expression)+
->expressionList;;

--Language specification says (add_op | mul_op ) ASSIGN
--which I think is incorrect since we specifically for this case have things like PLUSEQUAL and so on...
--and obviously lexer will match these tokens rather than PLUS and EQUAL separately
( plusequal=PLUSEQUAL | minusequal=MINUSEQUAL | orequal=OREQUAL | xorequal=XOREQUAL )
| ( multiplyequal=MULTIPLYEQUAL | divideequal=DIVIDEEQUAL | modequal=MODEQUAL | leftshiftequal=LEFTSHIFTEQUAL )
| ( rightshigtequal=RIGHTSHIFTEQUAL | amperequal=AMPEREQUAL | amperxorequal=AMPERXOREQUAL | assign=ASSIGN )
-> assign_op;;

--ShortVarDecl-------------------------------------------------------------
id=identifier (idList=idList | 0) autoassign=AUTOASSIGN expression=expression (expressionList=expressionList | 0)
-> shortVarDecl;;

--If Statement------------------------------------------------------------

--in this and few next rules there should be a conflict between expecting condition(expression) or simple statement and then condition
--it can't be resolved with LA because it's unknown where statement will start differing from expression
--properly we should distinguish it with try/rollback or some smart rule
--but I decided to parse whatever comes first as a simple statement since expression is also a simpleStmt
--then if there is a ";" we parse actual condtion and parse block if not
--that allows for succesful parsing of structures not supported by language
--but I think this small inconsistency is worth preformance we gain from not using try/rollback(or is it not?)

--About the thing with all this lparenCounts :
--When parsing If (and some others) statements there appears parsing ambiguity with lbraces
--when we see opening brace after identifier it is unclear if it's composite literal or start of a body block
--therefore language specification recommends putting composite literals in if clauses in parenthesis
--see details at http://golang.org/ref/spec#Composite_literals

--old way
--ifToken=IF try/rollback( [: lparenCount=0; inIfClause=true; :] simpleStmt=simpleStmt SEMICOLON expression=expression
--[: lparenCount=0; inIfClause=false; :] ifblock=block)
--catch([: lparenCount=0; inIfClause=true; :] expression=expression
--[: lparenCount=0; inIfClause=false; :] ifblock=block)
--( elseToken=ELSE (ifStmt=ifStmt | elseblock=block) | 0 )
---> ifStmt;;

ifToken=IF  [: lparenCount=0; inIfClause=true; :] stmtOrCondition=simpleStmt 
( SEMICOLON condition=expression | 0)
[: lparenCount=0; inIfClause=false; :] ifblock=block
( elseToken=ELSE (ifStmt=ifStmt | elseblock=block) | 0 )
-> ifStmt;;

--Switch Statement-------------------------------------------------------------

try/rollback(exprSwitchStatement=exprSwitchStatement)
catch(typeSwitchStatement=typeSwitchStatement)
-> switchStmt;;

SWITCH [: lparenCount=0; inIfClause=true; :] ( stmtOrCondition=simpleStmt | 0)
( SEMICOLON (condition=expression | 0) | 0)
[: lparenCount=0; inIfClause=false; :] LBRACE #exprCaseClause=exprCaseClause* RBRACE
->exprSwitchStatement;;

( caseToken=CASE expression=expression (expressionList=expressionList |0) | defaultToken=DEFAULT )
COLON statements=statements
->exprCaseClause;;

SWITCH try/rollback( [: lparenCount=0; inIfClause=true; :] simpleStmt=simpleStmt SEMICOLON typeSwitchGuard=typeSwitchGuard
[: lparenCount=0; inIfClause=false; :] LBRACE #typeCaseClause=typeCaseClause* RBRACE)
catch( [: lparenCount=0; inIfClause=true; :]  typeSwitchGuard=typeSwitchGuard
[: lparenCount=0; inIfClause=false; :] LBRACE #typeCaseClause=typeCaseClause* RBRACE)
->typeSwitchStatement;;

(?[: (LA(1).kind==Token_IDENT && LA(2).kind==Token_AUTOASSIGN) :] ident=identifier AUTOASSIGN | 0)
[: inSwitchTypeClause=true; :] primaryExpr=primaryExpr [: inSwitchTypeClause=false; :] DOT LPAREN TYPE RPAREN
->typeSwitchGuard;;

( caseToken=CASE (#typelist=type @ COMMA) | defaultToken=DEFAULT)
COLON statements=statements
->typeCaseClause;; 

--For Statement----------------------------------------------------------------------


--try/rollback is here because range clause incorrectly parses as statement
FOR try/rollback( [: lparenCount=0; inIfClause=true; :] ( initStmtOrCondition=simpleStmt | 0)
( semicolon=SEMICOLON (condition=expression | 0) SEMICOLON ( postStmt=simpleStmt | 0) --c-style for statement
 | 0 )  [: lparenCount=0; inIfClause=false; :] block=block)
catch( [: lparenCount=0; inIfClause=true; :] expression=expression (expressionList=expressionList | 0) (ASSIGN | autoassign=AUTOASSIGN)
range=RANGE rangeExpression=expression [: lparenCount=0; inIfClause=false; :] block=block) 			--range for statement
-- block=block
-> forStmt;;

--go statements-------------------------------------------------------------------

GO expression=expression
-> goStmt;;

--select statements----------------------------------------------------------------

SELECT LBRACE #commClause=commClause* RBRACE
-> selectStmt;;

commCase=commCase COLON statements=statements
-> commClause;;

defaultToken=DEFAULT
| CASE sendOrRecv=expression
( (expressionList=expressionList | 0) (ASSIGN | AUTOASSIGN) recvExp=expression
 | LEFTCHAN sendExp=expression | 0 )
 -> commCase;;

--Return statement-----------------------------------------------------------------
RETURN ( expression=expression (expressionList=expressionList | 0) | 0)
-> returnStmt;;

--Break, continue, goto, fallthrough, defer statement-----------------------------------------------------------------
BREAK (label=identifier | 0)
-> breakStmt;;

CONTINUE (label=identifier | 0)
-> continueStmt;;

GOTO label=identifier
-> gotoStmt;;

fallthrough=FALLTHROUGH
-> fallthroughStmt;;

DEFER expression=expression
-> deferStmt;;


--Packages------------------------------------------------------------------
---------------------------------------------------------------------------
---------------------------------------------------------------------------

packageClause=packageClause SEMICOLON (#importDecl=importDecl SEMICOLON)* (#topDeclarations=topLevelDeclaration SEMICOLON)*
-> sourceFile;;

PACKAGE packageName=identifier
-> packageClause;;

IMPORT ( importSpec=importSpec | LPAREN (#importSpecs=importSpec SEMICOLON)* RPAREN)
-> importDecl;;

(dot=DOT | packageName=identifier | 0) importpath=importpath
-> importSpec;;

id=IDENT
-> identifier;;

--to find importpath range easier
import=STRING
->importpath;;

[:
namespace go 
{

void Parser::expectedSymbol(int /*expectedSymbol*/, const QString& name)
{
  qDebug() << "Expected: " << name;
  //qDebug() << "Expected symbol: " << expectedSymbol;
}
void Parser::expectedToken(int /*expected*/, qint64 /*where*/, const QString& name)
{
  qDebug() << "Expected token: " << name;
}

Parser::ParserState* Parser::copyCurrentState()
{
    return new ParserState();
}

void Parser::restoreState( Parser::ParserState* state)
{
}

}

:]