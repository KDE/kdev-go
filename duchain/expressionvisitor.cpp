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

#include "expressionvisitor.h"

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include "types/gointegraltype.h"
#include "types/gostructuretype.h"
#include "types/gomaptype.h"
#include "types/gochantype.h"
#include "helper.h"
#include "duchaindebug.h"

using namespace KDevelop;

namespace go 
{

ExpressionVisitor::ExpressionVisitor(ParseSession* session, DUContext* context, DeclarationBuilder* builder) : 
					m_session(session), m_context(context), m_builder(builder)
{
}


QList< AbstractType::Ptr > ExpressionVisitor::lastTypes()
{
    return m_types;
}

void ExpressionVisitor::visitExpression(ExpressionAst* node)
{
    if(node->binary_op)
    {
        if( (node->binary_op->logicaland != -1) || (node->binary_op->logicalor != -1) || node->binary_op->rel_op )
        {//if we have relation operator then result will always be boolean
            //however we still visit subexpressions to build uses
            visitUnaryExpression(node->unaryExpression);
            visitExpression(node->expression);
            pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeBool)));
            return;
        }
    }
	    
    visitUnaryExpression(node->unaryExpression);
    if(node->binary_op)
    {
	//operands of binary operator should match unless one of them is untyped constant etc...(see golang specification)
	//TODO properly return types
	auto types = lastTypes();

	visitExpression(node->expression);

	popTypes();
	for(const AbstractType::Ptr &type : types)
	    addType(type);
    }
    
    //push unknown type if we failed to find anything else
    if(m_types.size() == 0)
	pushType(AbstractType::Ptr(new IntegralType(IntegralType::TypeNone)));
}

void ExpressionVisitor::visitUnaryExpression(UnaryExpressionAst* node)
{
    DefaultVisitor::visitUnaryExpression(node);
    if(node->unary_op || node->unsafe_unary_op )
    {
        if(lastTypes().size() == 0)
        {
            pushType(AbstractType::Ptr(new IntegralType(IntegralType::TypeNone)));
            return;
        }
        AbstractType::Ptr type = lastTypes().first();
        if(node->unsafe_unary_op && node->unsafe_unary_op->star != -1)
        {//dereferencing
            if(fastCast<PointerType*>(type.constData()))
            {
                PointerType::Ptr ptype(fastCast<PointerType*>(type.constData()));
                pushType(ptype->baseType());
            }
        }else if(node->unsafe_unary_op && node->unsafe_unary_op->leftchan != -1)
        {//chan retrieval
            if(fastCast<GoChanType*>(type.constData()))
            {
                GoChanType::Ptr ctype(fastCast<GoChanType*>(type.constData()));
                pushType(ctype->valueType());
                addType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeBool)));
            }
        }else if(node->unary_op && node->unary_op->ampersand != -1)
        {//taking an address
            PointerType* ptype = new PointerType();
            ptype->setBaseType(type);
            pushType(AbstractType::Ptr(ptype));
        }else if(node->unary_op && node->unary_op->bang != -1)
        {
            pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeBool)));
        }
    }
}

void ExpressionVisitor::visitPrimaryExpr(PrimaryExprAst* node)
{
    if(node->id)
    {
	//first try to handle builtin functions, so we can allow identifiers like "make"
	if(!handleBuiltinFunction(node)) 
	{
	    QualifiedIdentifier id(identifierForNode(node->id));
	    DeclarationPointer decl = go::getTypeOrVarDeclaration(id, m_context);
	    if(!decl)
	    {//first try to get declaration which has a real type
		//if failed - only at the beginning of a primary expression there can be namespace identifier(import)
		//reason for this is we have multiple declarations for each type, one of them is actual type declaration(Declaration::Type)
		//others are its method declarations(Declaration::Namespace)
		//we want to get real type declaration so we can access its internal context, not the method namespace
		decl = go::getDeclaration(id, m_context);
		if(!decl)
		{
                    DelayedType* unknown = new DelayedType();
                    unknown->setIdentifier(IndexedTypeIdentifier(identifierForNode(node->id)));
                    pushType(AbstractType::Ptr(unknown));
		    return;
		}
	    }
            m_declaration = decl;
	    //qCDebug(DUCHAIN) << "Expression Visitor for "<< id;
	
            //this handles stuff like mytype{}, mytype()
	    if((node->literalValue || node->callOrBuiltinParam) && decl->isTypeAlias())
	    {
		//type aliases are custom types
                pushUse(node->id, decl.data());
                StructureType* type = new StructureType();
                DUChainReadLocker lock;
                type->setDeclaration(decl.data());
                pushType(AbstractType::Ptr( type ));
	    }else if(node->callOrBuiltinParam)
	    {
		//TODO properly check arguments to handle overloads
		AbstractType::Ptr funcType = resolveTypeAlias(decl->abstractType());//to get actual function type from variables, storing functions
		if(fastCast<GoFunctionType*>(funcType.constData()))
		{
		    GoFunctionType::Ptr type(fastCast<GoFunctionType*>(funcType.constData()));
		    popTypes();
		    for(const AbstractType::Ptr& arg : type->returnArguments())
			addType(arg);
		    pushUse(node->id, decl.data());
		    visitCallOrBuiltinParam(node->callOrBuiltinParam);
                    //if function was called, last declaration will no longer be of this function
                    //this is needed to prevent wrong function calltips
                    m_declaration = DeclarationPointer();
		}
	    }else
	    {
		pushUse(node->id, decl.data());
		if(decl->kind() == Declaration::Namespace || decl->kind() == Declaration::NamespaceAlias)//import identifier
		{
		    StructureType* type = new StructureType();
		    DUChainReadLocker lock;
		    type->setDeclaration(decl.data());
		    pushType(AbstractType::Ptr(type));
		}else if(decl->kind() == Declaration::Instance)//variable
		{
		    pushType(decl->abstractType());
		}
	    } 
	}
    }else if(node->expression)
    {
        visitExpression(node->expression);
    }else
    {
        handleLiteralsAndConversions(node);
	//DefaultVisitor::visitPrimaryExpr(node);
    }
    if(node->primaryExprResolve)
        visitPrimaryExprResolve(node->primaryExprResolve);
}

void ExpressionVisitor::visitPrimaryExprResolve(PrimaryExprResolveAst* node)
{
   if(node->selector)
    {
	QList<AbstractType::Ptr> types = lastTypes();
	if(types.size() == 0)
	    return;
	//we will be looking for 2 types of declarations: namespace declarations and actual members
	//first type is methods assigned to custom type and imported declarations
	//if previous expression resulted in IdentifiedType(StructureType)
	//then it was variable of custom type, or custom type literal or import identifier
	//note that methods do NOT inherit from type to type
	//e.g. type mystruct int; type mystruct2 mystruct; func (m mystruct) method() {};
	//in this example mystruct2 will NOT have method method()
	
	//second type is field members of Go structs and interfaces(e.g. struct{ abc int}{10}.   or mystruct{10}.
	//										      ^		         ^
	// ^ - we are here
	//note that members are inherited(e.g. type mystruct struct { var1 int}; type mystruct2 mystruct; 
	//in this example mystruct2 will have member named var1
	//hope this explanation helps understand this code and some code in completion classes
	
	
	bool success=false;
	AbstractType::Ptr type = types.first();
	//evaluate pointers
	if(fastCast<PointerType*>(type.constData()))
	{
	    DUChainReadLocker lock;
	    PointerType::Ptr ptype(fastCast<PointerType*>(type.constData()));
	    if(ptype->baseType())
		type = ptype->baseType();
	}
	    
	if(fastCast<StructureType*>(type.constData()))
	{//we have to look for namespace declarations
	    DUChainReadLocker lock;
	    Declaration* declaration = fastCast<StructureType*>(type.constData())->declaration(m_context->topContext());
	    //if(decl->kind() == Declaration::Namespace || decl->kind() == Declaration::NamespaceAlias)
	    //{
		QualifiedIdentifier id(declaration->qualifiedIdentifier());
		id.push(identifierForNode(node->selector));
		lock.unlock();
		DeclarationPointer decl = getTypeOrVarDeclaration(id, m_context);
		if(decl)
		{
		    if(!handleComplexLiteralsAndConversions(node, decl.data())) //handle things like imp.mytype{2}
		    {
			pushUse(node->selector, decl.data());
			pushType(decl->abstractType());
		    }
		    m_declaration = decl;
		    success = true;
		}
	    //}
	}
	//this construction will descend through type hierarchy till it hits basic types
	//e.g. type mystruct struct{}; type mystruct2 mystruct; ...
	int count=0;
	if(!success) {
	    do {
		count++;
		GoStructureType::Ptr structure(fastCast<GoStructureType*>(type.constData()));
		if(structure)
		{//get members
		    DUContext* context = structure->context();
		    //qCDebug(DUCHAIN) << context->range() << m_context->range();
		    DeclarationPointer decl = getTypeOrVarDeclaration(identifierForNode(node->selector), context);
		    if(decl)
		    {
			DUChainReadLocker lock;
			pushUse(node->selector, decl.data());
			pushType(decl->abstractType());
                        m_declaration = decl;
		    }
		    break;
		}
		StructureType::Ptr identType(fastCast<StructureType*>(type.constData()));
		if(identType)
		{
		    DUChainReadLocker lock;
		    type = identType->declaration(m_context->topContext())->abstractType();
		}else 
		    break;
		
	    }while(type && count < 100); //if we descended 100 times and still didn't find anything - something is wrong
	}
    }else if(node->callParam)
    {
	//TODO properly check arguments to handle overloads
	if(lastTypes().size() == 0) return;
	AbstractType::Ptr funcType = resolveTypeAlias(lastTypes().first());//to get actual function type from variables, storing functions
	if(fastCast<GoFunctionType*>(funcType.constData()))
	{
	    GoFunctionType::Ptr type(fastCast<GoFunctionType*>(funcType.constData()));
	    popTypes();
	    for(const AbstractType::Ptr& arg : type->returnArguments())
		addType(arg);
	    //pushUse(node->id, decl.data());
	    visitCallParam(node->callParam);
	}
    }else if(node->index != -1)
    {//index expression
        if(lastTypes().size() == 0)
            return;
        AbstractType::Ptr type = popTypes().first();
        go::DefaultVisitor::visitPrimaryExprResolve(node); //build uses
        //slice expressions(e.g. a[low:high]) return slices and strings
        if(node->colon != -1)
        {
            if(fastCast<PointerType*>(type.constData()))
            {//pointer to arrays return slices in slice expressions
                PointerType::Ptr ptype(fastCast<PointerType*>(type.constData()));
                pushType(ptype->baseType());
                return;
            }
            pushType(type);
            return;
        }
        if(fastCast<GoIntegralType*>(type.constData()))
        {
            GoIntegralType::Ptr itype(fastCast<GoIntegralType*>(type.constData()));
            if(itype->dataType() == GoIntegralType::TypeString)
            {
                pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeByte)));
            }
        }else if(fastCast<ArrayType*>(type.constData()))
        {
            ArrayType::Ptr atype(fastCast<ArrayType*>(type.constData()));
            pushType(atype->elementType());
        }else if(fastCast<PointerType*>(type.constData()))
        {//pointers to array are automatically dereferenced
            PointerType::Ptr ptype(fastCast<PointerType*>(type.constData()));
            if(fastCast<ArrayType*>(ptype->baseType().constData()))
            {
                ArrayType::Ptr atype(fastCast<ArrayType*>(ptype->baseType().constData()));
                pushType(atype->elementType());
            }
        }else if(fastCast<GoMapType*>(type.constData()))
        {
            GoMapType::Ptr mtype(fastCast<GoMapType*>(type.constData()));
            //TODO check if expression and key type match, open a problem if not
            pushType(mtype->valueType());
            addType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeBool)));
        }else
        {
            //unrecognized index expression, return whatever type was before index
            pushType(type);
        }
    }else if(node->typeAssertion && m_builder)
    {
        m_builder->visitType(node->typeAssertion);
        pushType(m_builder->getLastType());
    }
    if(node->primaryExprResolve)
	visitPrimaryExprResolve(node->primaryExprResolve);
}

void ExpressionVisitor::visitCallOrBuiltinParam(CallOrBuiltinParamAst* node)
{
    //TODO properly check arguments
    //types of argument in function call will not affect type that expression will return
    //so just save it for now, and then push them
    auto types = popTypes();
    DefaultVisitor::visitCallOrBuiltinParam(node);
    popTypes();
    for(const AbstractType::Ptr &type : types)
	addType(type);
}

void ExpressionVisitor::visitCallParam(CallParamAst* node)
{
    auto types = popTypes();
    go::DefaultVisitor::visitCallParam(node);
    popTypes();
    for(const AbstractType::Ptr &type : types)
	addType(type);
}

void ExpressionVisitor::visitBasicLit(BasicLitAst* node)
{
    if(node->integer != -1)
	pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeInt)));
    else if(node->flt != -1)
	pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeFloat64)));
    else if(node->cpx != -1)
	pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeComplex128)));
    else if(node->rune != -1)
	pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeRune)));
    else if(node->string != -1)
	pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeString)));
}

void ExpressionVisitor::visitStructType(StructTypeAst* node)
{
    if(m_builder)
    {
        m_builder->visitStructType(node);
        pushType(m_builder->getLastType());
    }
}

void ExpressionVisitor::visitMapType(MapTypeAst* node)
{
    if(m_builder)
    {
        m_builder->visitMapType(node);
        pushType(m_builder->getLastType());
    }
}

void ExpressionVisitor::visitPointerType(PointerTypeAst* node)
{
    if(m_builder)
    {
        m_builder->visitPointerType(node);
        pushType(m_builder->getLastType());
    }
}

void ExpressionVisitor::visitInterfaceType(InterfaceTypeAst* node)
{
    if(m_builder)
    {
        m_builder->visitInterfaceType(node);
        pushType(m_builder->getLastType());
    }
}

void ExpressionVisitor::visitChanType(ChanTypeAst* node)
{
    if(m_builder)
    {
        m_builder->visitChanType(node);
        pushType(m_builder->getLastType());
    }
}

void ExpressionVisitor::visitParenType(ParenTypeAst* node)
{
    if(m_builder)
    {
        m_builder->visitParenType(node);
        if(node->type->typeName && fastCast<DelayedType*>(m_builder->getLastType().constData()))
        {//if we haven't found type it could be parenthesized function call '(funcname)(argument)'
            QualifiedIdentifier id(identifierForNode(node->type->typeName->name));
            if(node->type->typeName->type_resolve->fullName)
                id.push(identifierForNode(node->type->typeName->type_resolve->fullName));
	    DeclarationPointer decl = go::getTypeOrVarDeclaration(id, m_context);
            if(!decl)
                return;
            AbstractType::Ptr funcType = resolveTypeAlias(decl->abstractType());//to get actual function type from variables, storing functions
            if(fastCast<GoFunctionType*>(funcType.constData()))
            {
                GoFunctionType::Ptr type(fastCast<GoFunctionType*>(funcType.constData()));
                popTypes();
                for(const AbstractType::Ptr& arg : type->returnArguments())
                    addType(arg);
                pushUse(node->type->typeName->name, decl.data());
                m_declaration = decl;
                return;
            }
            //TODO there also can be builtin functions '(new)(int)'
        }
        pushType(m_builder->getLastType());
    }
}

void ExpressionVisitor::handleLiteralsAndConversions(PrimaryExprAst* node)
{
    if(!node)
        return;
    //build uses
    if(node->literalValue)
        visitLiteralValue(node->literalValue);
    if(node->convArg)
        visitConversionArgument(node->convArg);

    if(node->basicLit)
        visitBasicLit(node->basicLit);
    else if(node->structType)
        visitStructType(node->structType);
    else if(node->mapType)
        visitMapType(node->mapType);
    else if(node->signature)
    {
        if(m_builder)
        {
            m_builder->buildFunction(node->signature, node->body);
            pushType(m_builder->getLastType());
        }
    }
    else if(node->pointerType)
        visitPointerType(node->pointerType);
    else if(node->interfaceType)
        visitInterfaceType(node->interfaceType);
    else if(node->chanType)
        visitChanType(node->chanType);
    else if(node->parenType)
        visitParenType(node->parenType);
    else if(node->array != -1)
    {
        if(m_builder)
        {
            if(node->tripledot != -1)
            {
                m_builder->visitType(node->element);
            }else
            {
                if(node->arrayOrSliceResolve->array)
                    m_builder->visitType(node->arrayOrSliceResolve->array);
                else if(node->arrayOrSliceResolve->slice)
                    m_builder->visitType(node->arrayOrSliceResolve->slice);
            }
            ArrayType* array = new ArrayType();
            array->setElementType(m_builder->getLastType());
            pushType(AbstractType::Ptr(array));
        }
    }
}



void ExpressionVisitor::visitBlock(BlockAst* node)
{
}


void ExpressionVisitor::pushType(AbstractType::Ptr type)
{
    //if(type.isTypeAlias)
    m_types.clear();
    m_types.append(type);
}

AbstractType::Ptr ExpressionVisitor::resolveTypeAlias(AbstractType::Ptr type)
{
    if(fastCast<StructureType*>(type.constData()))
    {
	DUChainReadLocker lock;
	return fastCast<StructureType*>(type.constData())->declaration(m_context->topContext())->abstractType();
    }
    return type;
}


QList< AbstractType::Ptr > ExpressionVisitor::popTypes()
{
    QList<AbstractType::Ptr> types =lastTypes();
    m_types.clear();
    return types;
}

void ExpressionVisitor::addType(AbstractType::Ptr type)
{
    m_types.append(type);
}


void ExpressionVisitor::pushUse(IdentifierAst* node, Declaration* declaration)
{
    m_ids.append(node);
    m_declarations.append(DeclarationPointer(declaration));
}


QList< DeclarationPointer> ExpressionVisitor::allDeclarations()
{
    return m_declarations;
}

QList< IdentifierAst* > ExpressionVisitor::allIds()
{
    return m_ids;
}

void ExpressionVisitor::clearAll()
{
    m_declarations.clear();
    m_ids.clear();
    m_types.clear();
}

QualifiedIdentifier ExpressionVisitor::identifierForNode(IdentifierAst* node)
{
    if(!node)
	return QualifiedIdentifier();
    return QualifiedIdentifier(m_session->symbol(node->id));
}

bool ExpressionVisitor::handleComplexLiteralsAndConversions(PrimaryExprResolveAst* node, Declaration* decl)
{
    //we have to separately handle expressions like imp.mytype{3} because of the way grammar was written
    if(node->primaryExprResolve && (node->primaryExprResolve->literalValue || node->primaryExprResolve->callParam) && decl->isTypeAlias())
    {
	pushUse(node->selector, decl);
	StructureType* type = new StructureType();
	DUChainReadLocker lock;
	type->setDeclaration(decl);
	pushType(AbstractType::Ptr( type ));
	return true;
    }
    return false;
}

bool ExpressionVisitor::handleBuiltinFunction(PrimaryExprAst* node)
{
    if(!node->callOrBuiltinParam)
	return false;
    //we can't find types without builder
    if(!m_builder)
    {//but we can build uses anyway
	visitCallOrBuiltinParam(node->callOrBuiltinParam);
	return false;
    }
    if(node->callOrBuiltinParam)
    {
	//QualifiedIdentifier builtinFunction = identifierForNode(node->id);
	QString builtinFunction = identifierForNode(node->id).toString();
	if((builtinFunction == "make" || builtinFunction == "append") && node->callOrBuiltinParam->type)
	{//for make first argument must be slice, map or channel type
	    //TODO check types and open problem if they not what they should be
            m_builder->visitType(node->callOrBuiltinParam->type);
            AbstractType::Ptr type = m_builder->getLastType();
	    visitCallOrBuiltinParam(node->callOrBuiltinParam);
	    pushType(type);
	    return true;
	}else if(builtinFunction == "new")
	{
	    AbstractType::Ptr type;
	    if(!node->callOrBuiltinParam->type)
	    {//extract type name from expression
		ExpressionAst* exp = node->callOrBuiltinParam->expression;
		while(exp && exp->unaryExpression && exp->unaryExpression->primaryExpr)
		{
		    if(exp->unaryExpression->primaryExpr->expression)
		    {//type enclosed in parentheses
			exp = exp->unaryExpression->primaryExpr->expression;
			continue;
		    }
		    if(exp->unaryExpression->primaryExpr->id)
		    {
			IdentifierAst* fullname=0;
			if(exp->unaryExpression->primaryExpr->primaryExprResolve && exp->unaryExpression->primaryExpr->primaryExprResolve->selector)
			    fullname = exp->unaryExpression->primaryExpr->primaryExprResolve->selector;
                        m_builder->buildTypeName(exp->unaryExpression->primaryExpr->id, fullname);
                        type = m_builder->getLastType();
		    }
		    break;
		}
		if(!type) return false;
	    }else
	    {
                m_builder->visitType(node->callOrBuiltinParam->type);
                type = m_builder->getLastType();
	    }
	    visitCallOrBuiltinParam(node->callOrBuiltinParam);
	    if(type)
	    {
		PointerType* ptype = new PointerType();
		ptype->setBaseType(type);
		pushType(AbstractType::Ptr(ptype));
	    }
	    return true;
	}else if(builtinFunction == "cap" || builtinFunction == "copy" || builtinFunction == "len")
	{
	    visitCallOrBuiltinParam(node->callOrBuiltinParam);
	    pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeInt)));
            return true;
	}else if((builtinFunction == "imag" || builtinFunction == "real") && node->callOrBuiltinParam->expression)
	{
	    visitExpression(node->callOrBuiltinParam->expression);
	    auto types = popTypes();
	    if(types.size() != 1)
		return false;
	    AbstractType::Ptr type = resolveTypeAlias(types.first());
	    GoIntegralType::Ptr itype(fastCast<GoIntegralType*>(type.constData()));
	    if(itype)
	    {
		if(itype->dataType() == GoIntegralType::TypeComplex64)
		    pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeFloat32)));
		else
		    pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeFloat64)));
		return true;
	    }
	}else if(builtinFunction == "complex")
	{
	    visitCallOrBuiltinParam(node->callOrBuiltinParam);
	    pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeComplex128)));
	    return true;
	}else if(builtinFunction == "recover")
	{
	    visitCallOrBuiltinParam(node->callOrBuiltinParam);
	    GoStructureType* type = new GoStructureType();
	    type->setInterfaceType();
	    type->setContext(m_builder->newContext(RangeInRevision::invalid()));
	    type->setPrettyName("interface {}");
	    pushType(AbstractType::Ptr(type));
	    return true;
	}
	return false;
    }
    return false;
}

void ExpressionVisitor::visitTypeName(TypeNameAst* node)
{
    QualifiedIdentifier id(identifierForNode(node->name));
    if(node->type_resolve->fullName)
	id.push(identifierForNode(node->type_resolve->fullName));
    DeclarationPointer decl = getTypeDeclaration(id, m_context);
    if(decl)
    {
	if(node->type_resolve->fullName)
	{
	    //TODO can we do that more efficient?
	    DeclarationPointer package = getFirstDeclaration(decl->topContext());
	    pushUse(node->name, package.data());
	    pushUse(node->type_resolve->fullName, decl.data());
	}else
	    pushUse(node->name, decl.data());
    }
}

void ExpressionVisitor::visitRangeClause(ExpressionAst* node)
{
    visitExpression(node);
    if(lastTypes().size() == 0)
        return;
    AbstractType::Ptr type = popTypes().first();
    //go::DefaultVisitor::visitPrimaryExprResolve(node); //build uses
    //dereference pointers
    if(fastCast<PointerType*>(type.constData()))
    {
        PointerType::Ptr ptype = PointerType::Ptr(fastCast<PointerType*>(type.constData()));
        type = ptype->baseType();
    }
    //descend to underlying types through custom ones
    int recursionPrev = 0; //prevent recursion in case user defined circular types
    while(fastCast<StructureType*>(type.constData()) && recursionPrev < 100)
    {
        DUChainReadLocker lock;
        Declaration* declaration = fastCast<StructureType*>(type.constData())->declaration(m_context->topContext());
        if(!declaration || !declaration->abstractType())
            return;
        type = declaration->abstractType(); //underlying type
        recursionPrev++;
    }

    if(fastCast<GoIntegralType*>(type.constData()))
    {
        GoIntegralType::Ptr itype(fastCast<GoIntegralType*>(type.constData()));
        if(itype->dataType() == GoIntegralType::TypeString)
        {
            pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeInt)));
            addType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeRune)));
        }
    }else if(fastCast<ArrayType*>(type.constData()))
    {
        ArrayType::Ptr atype(fastCast<ArrayType*>(type.constData()));
        pushType(AbstractType::Ptr(new GoIntegralType(GoIntegralType::TypeInt)));
        addType(atype->elementType());
    }else if(fastCast<GoMapType*>(type.constData()))
    {
        GoMapType::Ptr mtype(fastCast<GoMapType*>(type.constData()));
        //TODO check if expression and key type match, open a problem if not
        pushType(mtype->keyType());
        addType(mtype->valueType());
    }else if(fastCast<GoChanType*>(type.constData()))
    {
        GoChanType::Ptr ctype(fastCast<GoChanType*>(type.constData()));
        pushType(ctype->valueType());
    }else
    {
        //unrecognized index expression, return whatever type was before index
        pushType(type);
    }
}



}
