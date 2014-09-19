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

#include "typebuilder.h"

#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/structuretype.h>

#include "types/gointegraltype.h"
#include "types/gostructuretype.h"
#include "types/gomaptype.h"
#include "types/gochantype.h"
#include "helper.h"

using namespace KDevelop;

namespace go
{

void TypeBuilder::visitTypeName(go::TypeNameAst* node)
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
            DUChainReadLocker lock;
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

void TypeBuilder::visitArrayOrSliceType(go::ArrayOrSliceTypeAst* node)
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

void TypeBuilder::visitPointerType(go::PointerTypeAst* node)
{
    PointerType* type = new PointerType();
    visitType(node->type);
    type->setBaseType(lastType());
    injectType<PointerType>(PointerType::Ptr(type));
}

void TypeBuilder::visitStructType(go::StructTypeAst* node)
{
    openType<go::GoStructureType>(go::GoStructureType::Ptr(new go::GoStructureType));
    {
        DUChainWriteLocker lock;
        openContext(node, editorFindRange(node, 0), DUContext::ContextType::Class, m_contextIdentifier);
    }
    TypeBuilderBase::visitStructType(node);
    {
        DUChainWriteLocker lock;
        currentType<go::GoStructureType>()->setContext(currentContext());
        closeContext();
    }
    currentType<go::GoStructureType>()->setPrettyName(m_session->textForNode(node));
    currentType<go::GoStructureType>()->setStructureType();
    closeType();
}

void TypeBuilder::visitFieldDecl(go::FieldDeclAst* node)
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

    for(auto name : names)
    {
        declareVariable(name, lastType());
    }
}


void TypeBuilder::visitInterfaceType(go::InterfaceTypeAst* node)
{
    openType<go::GoStructureType>(go::GoStructureType::Ptr(new go::GoStructureType));
    //ClassDeclaration* decl;
    {
        DUChainWriteLocker lock;
        //decl = openDeclaration<ClassDeclaration>(QualifiedIdentifier(), RangeInRevision());
        openContext(node, editorFindRange(node, 0), DUContext::ContextType::Class, m_contextIdentifier);
    }

    TypeBuilderBase::visitInterfaceType(node);
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

void TypeBuilder::visitMethodSpec(go::MethodSpecAst* node)
{
    if(node->signature)
    {
        parseSignature(node->signature, true, node->methodName);
    }else{
        visitTypeName(typeNameFromIdentifier(node->methodName, node->fullName));
        go::IdentifierAst* id = node->fullName ? node->fullName : node->methodName;
        {
            declareVariable(id, lastType());
        }
    }
}


void TypeBuilder::visitMapType(go::MapTypeAst* node)
{
    go::GoMapType* type = new go::GoMapType();
    visitType(node->keyType);
    type->setKeyType(lastType());
    visitType(node->elemType);
    type->setValueType(lastType());

    injectType(AbstractType::Ptr(type));
}

void TypeBuilder::visitChanType(go::ChanTypeAst* node)
{
    visitType(node->rtype ? node->rtype : node->stype);
    go::GoChanType::Ptr type(new go::GoChanType());
    if(node->stype)
        type->setKind(go::GoChanType::Receive);
    else if(node->send != -1)
        type->setKind(go::GoChanType::Send);
    else
        type->setKind(go::GoChanType::SendAndReceive);
    DUChainReadLocker lock;
    type->setValueType(lastType());
    injectType(type);
}

void TypeBuilder::visitFunctionType(go::FunctionTypeAst* node)
{
    parseSignature(node->signature, false);
}

void TypeBuilder::visitParameter(go::ParameterAst* node)
{
    //parameter grammar rule is written in such a way that full types won't be parsed automatically
    //so we do it manually(see go.g:parameter)
    //also if type is just identifier it is impossible to say right now whether it is really a type
    //or a identifier. This will be decided in parseParameres method
    if(node->idOrType && node->fulltype)
        visitTypeName(typeNameFromIdentifier(node->idOrType, node->fulltype));
    TypeBuilderBase::visitParameter(node);
}


go::GoFunctionDeclaration* TypeBuilder::parseSignature(go::SignatureAst* node, bool declareParameters, go::IdentifierAst* name)
{
    go::GoFunctionType::Ptr type(new go::GoFunctionType());
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
        return declareFunction(name, type, parametersContext, returnArgsContext);
    }
    return 0;
}

void TypeBuilder::parseParameters(go::ParametersAst* node, bool parseArguments, bool declareParameters)
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
        {
            function->setModifiers(go::GoFunctionType::VariadicArgument);
            ArrayType* atype = new ArrayType();
            atype->setElementType(lastType());
            injectType(AbstractType::Ptr(atype));
        }
        if(!param->complexType && !param->parenType && !param->unnamedvartype &&
            !param->type && !param->vartype && !param->fulltype)
            paramNames.append(param->idOrType); //we only have an identifier
        else
        {
            addArgumentHelper(function, lastType(), parseArguments);
            //if we have a parameter name(but it's not part of fullname) open declaration
            if(param->idOrType && !param->fulltype && declareParameters)
                declareVariable(param->idOrType, lastType());
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
                {
                    function->setModifiers(go::GoFunctionType::VariadicArgument);
                    ArrayType* atype = new ArrayType();
                    atype->setElementType(lastType());
                    injectType(AbstractType::Ptr(atype));
                }
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
                        if(declareParameters) declareVariable(id, lastType());
                    }
                    addArgumentHelper(function, lastType(), parseArguments);
                    if(declareParameters) declareVariable(param->idOrType, lastType());
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

void TypeBuilder::addArgumentHelper(go::GoFunctionType::Ptr function, AbstractType::Ptr argument, bool parseArguments)
{
    DUChainWriteLocker lock;
    if(argument)
    {
        if(parseArguments)
            function->addArgument(argument);
        else
            function->addReturnArgument(argument);
    }
}


go::TypeNameAst* TypeBuilder::typeNameFromIdentifier(go::IdentifierAst* id, go::IdentifierAst* fullname)
{
    //TODO handle memory leaks
    //(create buildTypeName(id, fullName) method and call it from visitTypeName and all places
    //this method gets called from)
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


AbstractType::Ptr TypeBuilder::buildType(go::TypeAst* node)
{
    visitType(node);
    return lastType();
}

AbstractType::Ptr TypeBuilder::buildType(go::IdentifierAst* node, go::IdentifierAst* fullname)
{
    visitTypeName(typeNameFromIdentifier(node, fullname));
    return lastType();
}


}