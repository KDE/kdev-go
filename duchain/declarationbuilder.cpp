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

#include "declarationbuilder.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/namespacealiasdeclaration.h>

#include "types/gointegraltype.h"
#include "types/gostructuretype.h"
#include "expressionvisitor.h"
#include "helper.h"

using namespace KDevelop;


#include <kdebug.h>

DeclarationBuilder::DeclarationBuilder(ParseSession* session, bool forExport) : m_export(forExport), m_preBuilding(false)
{
    setParseSession(session);
}

KDevelop::ReferencedTopDUContext DeclarationBuilder::build(const KDevelop::IndexedString& url, go::AstNode* node, KDevelop::ReferencedTopDUContext updateContext)
{
  kDebug() << "DeclarationBuilder start";
  if(!m_preBuilding)
  {
      kDebug() << "Running prebuilder";
      DeclarationBuilder preBuilder(m_session, m_export);
      preBuilder.m_preBuilding = true;
      updateContext = preBuilder.build(url, node, updateContext);
  }
  return DeclarationBuilderBase::build(url, node, updateContext);
}

void DeclarationBuilder::startVisiting(go::AstNode* node)
{
    DUChainWriteLocker lock;
    topContext()->clearImportedParentContexts();
    
    return DeclarationBuilderBase::startVisiting(node);
}

void DeclarationBuilder::visitVarSpec(go::VarSpecAst* node)
{
    if(node->type)
    {//if type is supplied we don't visit expressions
	m_contextIdentifier = identifierForNode(node->id);
	visitType(node->type);
	if(!lastType())
	    injectType(AbstractType::Ptr(new IntegralType(IntegralType::TypeNone)));
	DUChainWriteLocker lock;
	Declaration* dec = openDeclaration<Declaration>(identifierForNode(node->id), editorFindRange(node->id, 0));
	//dec->setType<AbstractType>(lastType());
	dec->setType(lastType());
	dec->setKind(Declaration::Instance);
	closeDeclaration();
	
	if(node->idList)
	{
	    auto iter = node->idList->idSequence->front(), end = iter;
	    do
	    {
		Declaration* dec = openDeclaration<Declaration>(identifierForNode(iter->element), editorFindRange(iter->element, 0));
		dec->setType<AbstractType>(lastType());
		dec->setKind(Declaration::Instance);
		closeDeclaration();
		iter = iter->next;
	    }
	    while (iter != end);
	}
    }else if(node->expression)
    {
	declareVariables(node->id, node->idList, node->expression, node->expressionList);
    }
}

void DeclarationBuilder::visitShortVarDecl(go::ShortVarDeclAst* node)
{
    declareVariables(node->id, node->idList, node->expression, node->expressionList);
}

void DeclarationBuilder::declareVariables(go::IdentifierAst* id, go::IdListAst* idList, go::ExpressionAst* expression, go::ExpressionListAst* expressionList)
{
    QList<AbstractType::Ptr> types;
    if(expression)
    {
	go::ExpressionVisitor exprVisitor(m_session, currentContext(), this);
	exprVisitor.visitExpression(expression);
	types = exprVisitor.lastTypes();
	Q_ASSERT(types.size() != 0);
	injectType<AbstractType>(AbstractType::Ptr(types.first()));
    }else
	injectType<AbstractType>(AbstractType::Ptr());
    
    if(!lastType())
	injectType(AbstractType::Ptr(new IntegralType(IntegralType::TypeNone)));
    DUChainWriteLocker lock;
    Declaration* dec = openDeclaration<Declaration>(identifierForNode(id), editorFindRange(id, 0));
    dec->setType<AbstractType>(lastType());
    dec->setKind(Declaration::Instance);
    closeDeclaration();
    lock.unlock();
    
    //uglyness of this code stems from weird grammar. Consider rewriting it
    //what we are doing here is we assign types to idList
    //problem is Go function are allowed to return multiple values
    //so it's not just a simple iteration over two lists
    if(idList)
    {
	types.removeFirst();
	const KDevPG::ListNode<go::ExpressionAst*>* expr=0;
	if(expressionList)
	    expr = expressionList->expressionsSequence->front();
	auto iter = idList->idSequence->front(), end = iter;
	do
	{
	    if(types.size() != 0)
		injectType<AbstractType>(types.first());
	    else
	    {
		if(!expr) //not enough expressions to assign types
		    return;
		go::ExpressionVisitor exprVisitor(m_session, currentContext(), this);
		exprVisitor.visitExpression(expr->element);
		types = exprVisitor.lastTypes();
		Q_ASSERT(types.size() != 0);
		injectType<AbstractType>(types.first());
		expr = expr->next;
	    }
	    types.removeFirst();
	    
	    lock.lock();
	    Declaration* dec = openDeclaration<Declaration>(identifierForNode(iter->element), editorFindRange(iter->element, 0));
	    dec->setType<AbstractType>(lastType());
	    dec->setKind(Declaration::Instance);
	    closeDeclaration();
	    lock.unlock();
	    iter = iter->next;
	}
	while (iter != end);
    }
}

void DeclarationBuilder::visitFunctionType(go::FunctionTypeAst* node)
{
    parseSignature(node->signature, false);
}

void DeclarationBuilder::addArgumentHelper(go::GoFunctionType::Ptr function, AbstractType::Ptr argument, bool parseArguments)
{
    DUChainWriteLocker lock;
    if(argument)
	if(parseArguments)
	    function->addArgument(argument);
	else
	    function->addReturnArgument(argument);
}

void DeclarationBuilder::declareParameter(go::IdentifierAst* name, const AbstractType::Ptr& type)
{
    DUChainWriteLocker lock;
    Declaration* decl = openDeclaration<Declaration>(identifierForNode(name), editorFindRange(name, 0));
    decl->setType(type);
    decl->setKind(Declaration::Instance);
    closeDeclaration();
}


void DeclarationBuilder::parseParameters(go::ParametersAst* node, bool parseArguments, bool declareParameters)
{
    //code below is a bit ugly because of problems with parsing go parameter list(see details at parser/go.g:331)
    go::GoFunctionType::Ptr function;
    function = currentType<go::GoFunctionType>();
    if(node->parameter)
    {
	QList<go::IdentifierAst*> paramNames;
	go::ParameterAst* param=node->parameter;
	visitParameter(param);
	//variadic arguments
	if(param->unnamedvartype || param->vartype)
	    function->setModifiers(1000);
	if(!param->complexType && !param->parenType && !param->unnamedvartype && 
	    !param->type && !param->vartype && !param->fulltype)
	    paramNames.append(param->idOrType); //we only have an identifier
	else
	{
	    addArgumentHelper(function, lastType(), parseArguments);
	    //if we have a parameter name(but it's not part of fullname) open declaration
	    if(param->idOrType && !param->fulltype && declareParameters)
		declareParameter(param->idOrType, lastType());
	}
	
	if(node->parameterListSequence)
	{
	    auto elem = node->parameterListSequence->front();
	    while(true)
	    {
		go::ParameterAst* param=elem->element;
		visitParameter(param);
		//variadic arguments
		if(param->unnamedvartype || param->vartype)
		    function->setModifiers(1000);
		if(param->complexType || param->parenType || param->unnamedvartype || param->fulltype)
		{//we have a unnamed parameter list of types
		    AbstractType::Ptr lType = lastType();
		    for(auto id : paramNames)
		    {
			visitTypeName(typeNameFromIdentifier(id));
			addArgumentHelper(function, lastType(), parseArguments);
		    }
		    addArgumentHelper(function, lType, parseArguments);
		    paramNames.clear();
		}else if(!param->complexType && !param->parenType && !param->unnamedvartype && 
		    !param->type && !param->vartype && !param->fulltype)
		{//just another identifier
		    paramNames.append(param->idOrType);
		}else
		{//identifier with type, all previous identifiers are of the same type
		    for(auto id : paramNames)
		    {
			addArgumentHelper(function, lastType(), parseArguments);
			if(declareParameters) declareParameter(id, lastType());
		    }
		    addArgumentHelper(function, lastType(), parseArguments);
		    if(declareParameters) declareParameter(param->idOrType, lastType());
		    paramNames.clear();
		}
		if(elem->hasNext())
		    elem = elem->next;
		else break;
		
	    }
	    if(!paramNames.empty()) 
	    {//we have only identifiers which means they are all type names
		//foreach(auto id, paramNames)
		for(auto id : paramNames)
		{
		    visitTypeName(typeNameFromIdentifier(id));
		    addArgumentHelper(function, lastType(), parseArguments);
		}
		paramNames.clear();
	    }
	    
	}else if(!paramNames.empty())
	{
	    //one identifier that we have is a type
	    visitTypeName(typeNameFromIdentifier(param->idOrType));
	    addArgumentHelper(function, lastType(), parseArguments);
	}
    }
}

void DeclarationBuilder::visitParameter(go::ParameterAst* node)
{
    //parameter grammar rule is written in such a way that full types won't be parsed automatically
    //so we do it manually(see go.g:parameter)
    //also if type is just identifier it is impossible to say right now whether it is really a type
    //or a identifier. This will be decided in parseParameres method
    if(node->idOrType && node->fulltype)
	visitTypeName(typeNameFromIdentifier(node->idOrType, node->fulltype));
    DeclarationBuilderBase::visitParameter(node);
}

void DeclarationBuilder::visitTypeName(go::TypeNameAst* node)
{
    uint type = IntegralType::TypeNone;
    QualifiedIdentifier id = identifierForNode(node->name);
    QString name = id.toString();
    //Builtin types
    if(name == "uint8")
	type = go::GoIntegralType::TypeUint8;
    else if(name == "uint16")
	type = go::GoIntegralType::TypeUint16;
    else if(name == "uint32")
	type = go::GoIntegralType::TypeUint32;
    else if(name == "uint64")
	type = go::GoIntegralType::TypeUint64;
    else if(name == "int8")
	type = go::GoIntegralType::TypeUint8;
    else if(name == "int16")
	type = go::GoIntegralType::TypeInt16;
    else if(name == "int32")
	type = go::GoIntegralType::TypeInt32;
    else if(name == "int64")
	type = go::GoIntegralType::TypeInt64;
    else if(name == "float32")
	type = go::GoIntegralType::TypeFloat32;
    else if(name == "float64")
	type = go::GoIntegralType::TypeFloat64;
    else if(name == "complex64")
	type = go::GoIntegralType::TypeComplex64;
    else if(name == "complex128")
	type = go::GoIntegralType::TypeComplex128;
    else if(name == "rune")
	type = go::GoIntegralType::TypeRune;
    else if(name == "int")
	type = go::GoIntegralType::TypeInt;
    else if(name == "uint")
	type = go::GoIntegralType::TypeUint;
    else if(name == "uintptr")
	type = go::GoIntegralType::TypeUintptr;
    else if(name == "string")
	type = go::GoIntegralType::TypeString;
    else if(name == "bool")
	type = go::GoIntegralType::TypeBool;
    else if(name == "byte")
	type = go::GoIntegralType::TypeByte;
    
    if(type == IntegralType::TypeNone)
    {
	//in Go one can create variable of package type, like 'fmt fmt'
	//TODO support such declarations
	QualifiedIdentifier id(identifierForNode(node->name));
	if(node->type_resolve->fullName)
	    id.push(identifierForNode(node->type_resolve->fullName));
	DeclarationPointer decl = go::getTypeDeclaration(id, currentContext());
	if(decl)
	{
	    StructureType* type = new StructureType();
	    type->setDeclaration(decl.data());
	    injectType<AbstractType>(AbstractType::Ptr(type));
	    //kDebug() << decl->range();
	    return;
	}
	DelayedType* unknown = new DelayedType();
	unknown->setIdentifier(IndexedTypeIdentifier(id));
	injectType<AbstractType>(AbstractType::Ptr(unknown));
	return;
    }
    if(type != IntegralType::TypeNone)
    {
	injectType<AbstractType>(AbstractType::Ptr(new go::GoIntegralType(type)));
    }
}

void DeclarationBuilder::visitArrayOrSliceType(go::ArrayOrSliceTypeAst* node)
{
    if(node->arrayOrSliceResolve->array)
	visitType(node->arrayOrSliceResolve->array);
    else if(node->arrayOrSliceResolve->slice)
	visitType(node->arrayOrSliceResolve->slice);
    else //error
	injectType<AbstractType>(AbstractType::Ptr());
    
    //TODO create custom classes GoArrayType and GoSliceType
    //to properly distinguish between go slices and arrays
    ArrayType* array = new ArrayType();
    //kDebug() << lastType()->toString();
    array->setElementType(lastType());
    injectType<ArrayType>(ArrayType::Ptr(array));
}

void DeclarationBuilder::visitPointerType(go::PointerTypeAst* node)
{
    PointerType* type = new PointerType();
    visitType(node->type);
    type->setBaseType(lastType());
    injectType<PointerType>(PointerType::Ptr(type));
}

void DeclarationBuilder::visitStructType(go::StructTypeAst* node)
{
    openType<go::GoStructureType>(go::GoStructureType::Ptr(new go::GoStructureType));
    {
	DUChainWriteLocker lock;
	openContext(node, editorFindRange(node, 0), DUContext::ContextType::Class, m_contextIdentifier);
    }
    DeclarationBuilderBase::visitStructType(node);
    {
	DUChainWriteLocker lock;
	currentType<go::GoStructureType>()->setContext(currentContext());
	closeContext();
    }
    currentType<go::GoStructureType>()->setPrettyName(m_session->textForNode(node));
    currentType<go::GoStructureType>()->setStructureType();
    closeType();
}

void DeclarationBuilder::visitFieldDecl(go::FieldDeclAst* node)
{
    StructureType::Ptr structure = currentType<StructureType>();
    QList<go::IdentifierAst*> names;
    if(node->anonFieldStar)
    {
	PointerType* type = new PointerType();
	visitTypeName(node->anonFieldStar->typeName);
	type->setBaseType(lastType());
	go::IdentifierAst* id = node->anonFieldStar->typeName->type_resolve->fullName ? 
			    node->anonFieldStar->typeName->type_resolve->fullName :
			    node->anonFieldStar->typeName->name;
			    
	injectType<PointerType>(PointerType::Ptr(type));
	names.append(id);
    }else if(node->type)
    {
	visitType(node->type);
	names.append(node->varid);
	if(node->idList)
	{
	    auto elem = node->idList->idSequence->front();
	    while(true)
	    {
		names.append(elem->element);
		if(elem->hasNext())
		    elem = elem->next;
		else break;
	    }
	}
    }else
    {
	visitTypeName(typeNameFromIdentifier(node->varid, node->fullname));
	go::IdentifierAst* id = node->fullname ? node->fullname : node->varid;
	names.append(id);
    }
    
    DUChainWriteLocker lock;
    for(auto name : names)
    {
	Declaration* decl = openDeclaration<Declaration>(identifierForNode(name), editorFindRange(name, 0));
	decl->setAbstractType(lastType());
	closeDeclaration();
    }
}

void DeclarationBuilder::visitInterfaceType(go::InterfaceTypeAst* node)
{
    openType<go::GoStructureType>(go::GoStructureType::Ptr(new go::GoStructureType));
    //ClassDeclaration* decl;
    {
	DUChainWriteLocker lock;
	//decl = openDeclaration<ClassDeclaration>(QualifiedIdentifier(), RangeInRevision());
	openContext(node, editorFindRange(node, 0), DUContext::ContextType::Class, m_contextIdentifier);
    }

    DeclarationBuilderBase::visitInterfaceType(node);
    {
	DUChainWriteLocker lock;
	//decl->setInternalContext(currentContext());
	//decl->setClassType(ClassDeclarationData::Interface);
	currentType<go::GoStructureType>()->setContext(currentContext());
	closeContext();
	//closeDeclaration();
	//currentType<go::GoStructureType>()->setDeclaration(decl);
	//decl->setIdentifier(Identifier(QString("interface type")));
    }
    currentType<go::GoStructureType>()->setPrettyName(m_session->textForNode(node));
    currentType<go::GoStructureType>()->setInterfaceType();
    closeType();
}

void DeclarationBuilder::visitMethodSpec(go::MethodSpecAst* node)
{
    if(node->signature)
    {
	parseSignature(node->signature, true, node->methodName);
    }else{
	visitTypeName(typeNameFromIdentifier(node->methodName, node->fullName));
	go::IdentifierAst* id = node->fullName ? node->fullName : node->methodName;
	{
	    DUChainWriteLocker lock;
	    Declaration* decl = openDeclaration<Declaration>(identifierForNode(id), editorFindRange(id, 0));
	    decl->setAbstractType(lastType());
	    closeDeclaration();
	}
    }
}

go::TypeNameAst* DeclarationBuilder::typeNameFromIdentifier(go::IdentifierAst* id, go::IdentifierAst* fullname)
{
    go::TypeNameAst* newnode = new go::TypeNameAst();
    go::Type_resolveAst* res = new go::Type_resolveAst();
    newnode->kind = go::TypeNameAst::KIND;
    res->kind = go::Type_resolveAst::KIND;
    if(fullname)
	res->fullName = fullname;
    newnode->name = id;
    newnode->type_resolve = res;
    newnode->startToken = id->startToken;
    if(fullname)
	newnode->endToken = newnode->type_resolve->endToken;
    else 
	newnode->endToken = id->endToken;
    return newnode;
}

void DeclarationBuilder::visitFuncDeclaration(go::FuncDeclarationAst* node)
{
    go::GoFunctionDeclaration* decl = parseSignature(node->signature, true, node->funcName);
    
    if(!node->body)
	return;
    
    DUContext* bodyContext = openContext(node->body, DUContext::ContextType::Function, node->funcName);
    //import parameters into body context
    if(decl->internalContext()) currentContext()->addImportedParentContext(decl->internalContext());
    if(decl->returnArgsContext()) currentContext()->addImportedParentContext(decl->returnArgsContext());

 
    visitBlock(node->body);
    closeContext();
    {
	DUChainWriteLocker lock;
	decl->setInternalFunctionContext(bodyContext);
	decl->setKind(Declaration::Instance);
    }
}

void DeclarationBuilder::visitMethodDeclaration(go::MethodDeclarationAst* node)
{
    Declaration* declaration=0;
    if(node->methodRecv)
    {
	go::IdentifierAst* actualtype=0;
	if(node->methodRecv->ptype)
	    actualtype = node->methodRecv->ptype;
	else if(node->methodRecv->type)
	    actualtype = node->methodRecv->type;
	else 
	    actualtype = node->methodRecv->nameOrType;
	DUChainWriteLocker lock;
	declaration = openDeclaration<Declaration>(identifierForNode(actualtype), editorFindRange(actualtype, 0));
	declaration->setKind(Declaration::Namespace);
	openContext(node, editorFindRange(node, 0), DUContext::Namespace, identifierForNode(actualtype));
	declaration->setInternalContext(currentContext());
    }
    
    go::GoFunctionDeclaration* decl = parseSignature(node->signature, true, node->methodName);
    
    if(!node->body)
	return;

    DUContext* bodyContext = openContext(node->body, DUContext::ContextType::Function, node->methodName);

    //import parameters into body context
    if(decl->internalContext()) currentContext()->addImportedParentContext(decl->internalContext());
    if(decl->returnArgsContext()) currentContext()->addImportedParentContext(decl->returnArgsContext());
    
    if(node->methodRecv->type)
    {//declare method receiver variable('this' or 'self' analog in Go)
	visitTypeName(typeNameFromIdentifier(node->methodRecv->type));
	if(node->methodRecv->star!= -1)
	{
	    PointerType* ptype = new PointerType();
	    ptype->setBaseType(lastType());
	    injectType(PointerType::Ptr(ptype));
	}
	DUChainWriteLocker n;
	Declaration* thisVariable = openDeclaration<Declaration>(identifierForNode(node->methodRecv->nameOrType), editorFindRange(node->methodRecv->nameOrType, 0));
	thisVariable->setAbstractType(lastType());
	closeDeclaration();
    }
	
    visitBlock(node->body);
    closeContext();
    {
	DUChainWriteLocker lock;
	decl->setInternalFunctionContext(bodyContext);
	decl->setKind(Declaration::Instance);
    }
    
    closeContext();	//namespace
    closeDeclaration();	//namespace declaration
}


void DeclarationBuilder::visitMapType(go::MapTypeAst* node)
{
    //TODO create real mapType
    visitType(node->keyType);
    QString typeName("map["+lastType()->toString()+"]");
    visitType(node->elemType);
    typeName.append(lastType()->toString());
    
    DelayedType::Ptr type = DelayedType::Ptr(new DelayedType());
    openType<DelayedType>(type);
    type->setIdentifier(IndexedTypeIdentifier(typeName));
    closeType();
}

void DeclarationBuilder::visitChanType(go::ChanTypeAst* node)
{
    //TODO create real chan type
    visitType(node->rtype ? node->rtype : node->stype);
    DelayedType::Ptr type = DelayedType::Ptr(new DelayedType());
    openType<DelayedType>(type);
    type->setIdentifier(IndexedTypeIdentifier(QString("chan ") + lastType()->toString()));
    closeType();
}

void DeclarationBuilder::visitTypeSpec(go::TypeSpecAst* node)
{
    //kDebug() << "Type" << identifierForNode(node->name) << " enter";
    Declaration* decl;
    {
	DUChainWriteLocker lock;
	decl = openDeclaration<Declaration>(identifierForNode(node->name), editorFindRange(node->name, 0));
	//decl->setKind(Declaration::Namespace);
	decl->setKind(Declaration::Type);
	//force direct here because otherwise DeclarationId will mess up actual type declaration and method declarations
	//TODO perhaps we can do this with specialization or additional identity?
	decl->setAlwaysForceDirect(true);
    }
    m_contextIdentifier = identifierForNode(node->name);
    visitType(node->type);
    DUChainWriteLocker lock;
    kDebug() << lastType()->toString();
    decl->setType(lastType());
    
    decl->setIsTypeAlias(true);
    closeDeclaration();
    //kDebug() << "Type" << identifierForNode(node->name) << " exit";
}

void DeclarationBuilder::visitImportSpec(go::ImportSpecAst* node)
{
    //prevent recursive imports
    //without preventing recursive imports. importing standart go library(2000+ files) takes minutes and sometimes never stops
    //thankfully go import mechanism doesn't need recursive imports(I think)
    if(m_export)
	return;
    QString import(identifierForIndex(node->importpath->import).toString());
    QList<ReferencedTopDUContext> contexts = m_session->contextForImport(import);
    if(contexts.empty())
	return;
 
    for(const ReferencedTopDUContext& context : contexts)
    { //first declaration in top context must be package declaration
	DeclarationPointer decl = go::getFirstDeclaration(context);
	if(!decl)
	    continue;
	QualifiedIdentifier packageName(decl->qualifiedIdentifier());
	
	DUChainWriteLocker lock;
	if(node->packageName)
	{//create alias for package
	    QualifiedIdentifier id = identifierForNode(node->packageName);
	    NamespaceAliasDeclaration* decl = openDeclaration<NamespaceAliasDeclaration>(id, editorFindRange(node->importpath, 0));
	    decl->setKind(Declaration::NamespaceAlias);
	    decl->setImportIdentifier(packageName); //this needs to be actual package name
	    closeDeclaration();
	}else if(node->dot != -1)
	{//anonymous import
	    NamespaceAliasDeclaration* decl = openDeclaration<NamespaceAliasDeclaration>(QualifiedIdentifier(globalImportIdentifier()), 
											 editorFindRange(node->importpath, 0));
	    decl->setKind(Declaration::NamespaceAlias);
	    decl->setImportIdentifier(packageName); //this needs to be actual package name
	    closeDeclaration();
	}else
	{
	    Declaration* decl = openDeclaration<Declaration>(packageName, editorFindRange(node->importpath, 0));
	    decl->setKind(Declaration::Import);
	    closeDeclaration();
	}
	topContext()->addImportedParentContext(context.data());
    }
}

void DeclarationBuilder::visitSourceFile(go::SourceFileAst* node)
{
    Declaration* packageDeclaration = openDeclaration<Declaration>(identifierForNode(node->packageClause->packageName), editorFindRange(node->packageClause->packageName, 0));
    packageDeclaration->setKind(Declaration::Namespace);
    openContext(node, editorFindRange(node, 0), DUContext::Namespace, identifierForNode(node->packageClause->packageName));
    
    packageDeclaration->setInternalContext(currentContext());
    //import all files in current directory
    if(!m_export)
	importThisPackage();
    
    go::DefaultVisitor::visitSourceFile(node);
    closeContext();
    closeDeclaration();
}

void DeclarationBuilder::importThisPackage()
{
    if(m_export)
	return;
    QList<ReferencedTopDUContext> contexts = m_session->contextForThisPackage(document());
    if(contexts.empty())
	return;
    
    for(const ReferencedTopDUContext& context : contexts)
    {
	//first declaration in top context must be package declaration
	DeclarationPointer decl = go::getFirstDeclaration(context);
	if(!decl)
	    continue;
	QualifiedIdentifier packageName(decl->qualifiedIdentifier());
	
	DUChainWriteLocker lock;
	//TODO Since package names are identical duchain should find declarations without namespace alias, right?
	
	//NamespaceAliasDeclaration* import = openDeclaration<NamespaceAliasDeclaration>(QualifiedIdentifier(globalImportIdentifier()), RangeInRevision());
	//import->setKind(Declaration::NamespaceAlias);
	//import->setImportIdentifier(packageName); //this needs to be actual package name
	//closeDeclaration();
	topContext()->addImportedParentContext(context.data());
    }
}

go::GoFunctionDeclaration* DeclarationBuilder::parseSignature(go::SignatureAst* node, bool declareParameters, go::IdentifierAst* name)
{
    go::GoFunctionType::Ptr type = go::GoFunctionType::Ptr(new go::GoFunctionType());
    openType<go::GoFunctionType>(type); 
    
    DUContext* parametersContext;
    if(declareParameters) parametersContext = openContext(node->parameters, 
					       editorFindRange(node->parameters, 0), 
					       DUContext::ContextType::Function, 
					       name);
    
    parseParameters(node->parameters, true, declareParameters);
    if(declareParameters) closeContext();
    
    DUContext* returnArgsContext=0;
    
    if(node->result)
    {
	visitResult(node->result);
	if(node->result->parameters)
	{
	    if(declareParameters) returnArgsContext = openContext(node->result,
						editorFindRange(node->result, 0),
						DUContext::ContextType::Function,
					        name);
	    parseParameters(node->result->parameters, false, declareParameters);
	    if(declareParameters) closeContext();
	    
	}
	if(!node->result->parameters && lastType())
	    type->addReturnArgument(lastType());
    }
    closeType();
    
    if(declareParameters)
    {
	DUChainWriteLocker lock;
	go::GoFunctionDeclaration* dec = openDefinition<go::GoFunctionDeclaration>(identifierForNode(name), editorFindRange(name, 0));
	dec->setType<go::GoFunctionType>(type);
	//dec->setKind(Declaration::Type);
	dec->setKind(Declaration::Instance);
	dec->setInternalContext(parametersContext);
	if(returnArgsContext)
	    dec->setReturnArgsContext(returnArgsContext);
	//dec->setInternalFunctionContext(bodyContext);
	closeDeclaration();
	return dec;
    }
    return 0;
}

AbstractType::Ptr DeclarationBuilder::buildType(go::TypeAst* node)
{
    visitType(node);
    return lastType();
}

AbstractType::Ptr DeclarationBuilder::buildType(go::IdentifierAst* node, go::IdentifierAst* fullname)
{
    visitTypeName(typeNameFromIdentifier(node, fullname));
    return lastType();
}

