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

#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/pointertype.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>

#include "expressionvisitor.h"
#include "helper.h"
#include "duchaindebug.h"

using namespace KDevelop;


DeclarationBuilder::DeclarationBuilder(ParseSession* session, bool forExport) : m_export(forExport), m_preBuilding(false), m_lastTypeComment(), m_lastConstComment()
{
    setParseSession(session);
}

KDevelop::ReferencedTopDUContext DeclarationBuilder::build(const KDevelop::IndexedString& url, go::AstNode* node, const KDevelop::ReferencedTopDUContext& updateContext_)
{
  qCDebug(DUCHAIN) << "DeclarationBuilder start";
  ReferencedTopDUContext updateContext(updateContext_);
  if(!m_preBuilding)
  {
      qCDebug(DUCHAIN) << "Running prebuilder";
      DeclarationBuilder preBuilder(m_session, m_export);
      preBuilder.m_preBuilding = true;
      updateContext = preBuilder.build(url, node, updateContext);
  }
  return DeclarationBuilderBase::build(url, node, updateContext);
}

void DeclarationBuilder::startVisiting(go::AstNode* node)
{
    {
        DUChainWriteLocker lock;
        topContext()->clearImportedParentContexts();
        topContext()->updateImportsCache();
    }

    return DeclarationBuilderBase::startVisiting(node);
}

void DeclarationBuilder::visitVarSpec(go::VarSpecAst* node)
{
    if(node->type)
    {//if type is supplied we don't visit expressions
        declareVariablesWithType(node->id, node->idList, node->type, false);
    }else if(node->expression)
    {
        declareVariables(node->id, node->idList, node->expression, node->expressionList, false);
    }
}

void DeclarationBuilder::visitShortVarDecl(go::ShortVarDeclAst* node)
{
    declareVariables(node->id, node->idList, node->expression, node->expressionList, false);
}

void DeclarationBuilder::declareVariablesWithType(go::IdentifierAst* id, go::IdListAst* idList, go::TypeAst* type, bool declareConstant)
{
    m_contextIdentifier = identifierForNode(id);
    visitType(type);
    if(!lastType())
        injectType(AbstractType::Ptr(new IntegralType(IntegralType::TypeNone)));
    lastType()->setModifiers(declareConstant ? AbstractType::ConstModifier : AbstractType::NoModifiers);
    if(identifierForNode(id).toString() != "_")
    {
        declareVariable(id, lastType());
    }
    if(declareConstant) m_constAutoTypes.append(lastType());

    if(idList)
    {
        auto iter = idList->idSequence->front(), end = iter;
        do
        {
            if(identifierForNode(iter->element).toString() != "_")
            {
                declareVariable(iter->element, lastType());
            }
            if(declareConstant)
                m_constAutoTypes.append(lastType());
            iter = iter->next;
        }
        while (iter != end);
    }
}


void DeclarationBuilder::declareVariables(go::IdentifierAst* id, go::IdListAst* idList, go::ExpressionAst* expression,
                                            go::ExpressionListAst* expressionList, bool declareConstant)
{
    m_contextIdentifier = identifierForNode(id);
    QList<AbstractType::Ptr> types;
    if(!expression)
        return;
    go::ExpressionVisitor exprVisitor(m_session, currentContext(), this);
    exprVisitor.visitExpression(expression);
    visitExpression(expression);
    Q_ASSERT(exprVisitor.lastTypes().size() != 0);
    if(!expressionList)
        types = exprVisitor.lastTypes();
    else
    {
        types.append(exprVisitor.lastTypes().first());
        auto iter = expressionList->expressionsSequence->front(), end = iter;
        do
        {
            exprVisitor.clearAll();
            exprVisitor.visitExpression(iter->element);
            Q_ASSERT(exprVisitor.lastTypes().size() != 0);
            types.append(exprVisitor.lastTypes().first());
            iter = iter->next;
        }
        while (iter != end);
    }

    if(types.size() == 0)
        return;
    for(AbstractType::Ptr& type : types)
    {
        if(type != nullptr)
        {
            type->setModifiers(declareConstant ? AbstractType::ConstModifier : AbstractType::NoModifiers);
        }
    }
    if(declareConstant)
        m_constAutoTypes = types;

    declareVariable(id, types.first());

    if(idList)
    {
        int typeIndex = 1;
        auto iter = idList->idSequence->front(), end = iter;
        do
        {
            if(typeIndex >= types.size()) //not enough types to declare all variables
                return;
            declareVariable(iter->element, types.at(typeIndex));
            iter = iter->next;
            typeIndex++;
        }
        while (iter != end);
    }
}

void DeclarationBuilder::declareVariable(go::IdentifierAst* id, const AbstractType::Ptr& type)
{
    auto identifier = identifierForNode(id);
    auto declaration = go::getDeclaration(identifier, currentContext(), false);
    auto wasDeclaredInCurrentContext = declaration && declaration.data()->range() != editorFindRange(id, 0);
    if(identifier.toString() != "_" && !wasDeclaredInCurrentContext)
    {
        if(type->modifiers() & AbstractType::ConstModifier)
        {
            setComment(m_lastConstComment);
        }
        DUChainWriteLocker lock;
        Declaration* dec = openDeclaration<Declaration>(identifierForNode(id), editorFindRange(id, 0));
        dec->setType<AbstractType>(type);
        dec->setKind(Declaration::Instance);
        closeDeclaration();
    }
}


void DeclarationBuilder::visitConstDecl(go::ConstDeclAst* node)
{
    m_constAutoTypes.clear();
    m_lastConstComment = m_session->commentBeforeToken(node->startToken);
    //adding const declaration code, just like in GoDoc
    m_lastConstComment.append(m_session->textForNode(node).toUtf8());
    go::DefaultVisitor::visitConstDecl(node);
    m_lastConstComment = QByteArray();
}


void DeclarationBuilder::visitConstSpec(go::ConstSpecAst* node)
{
    if(node->type)
    {
        declareVariablesWithType(node->id, node->idList, node->type, true);
    }else if(node->expression)
    {
        declareVariables(node->id, node->idList, node->expression, node->expressionList, true);
    }else
    {//this can only happen after a previous constSpec with some expressionList
        //in this case identifiers assign same types as previous constSpec(http://golang.org/ref/spec#Constant_declarations)
        if(m_constAutoTypes.size() == 0)
            return;
        {
            declareVariable(node->id, m_constAutoTypes.first());
        }

        if(node->idList)
        {
            int typeIndex = 1;
            auto iter = node->idList->idSequence->front(), end = iter;
            do
            {
                if(typeIndex >= m_constAutoTypes.size()) //not enough types to declare all constants
                    return;

                declareVariable(iter->element, m_constAutoTypes.at(typeIndex));
                iter = iter->next;
                typeIndex++;
            }
            while (iter != end);
        }
    }
}

void DeclarationBuilder::visitFuncDeclaration(go::FuncDeclarationAst* node)
{
    buildFunction(node->signature, node->body, node->funcName, m_session->commentBeforeToken(node->startToken-1));
}

void DeclarationBuilder::visitPrimaryExpr(go::PrimaryExprAst *node)
{
    if(node->signature && !node->convArg) // func type literal and not conversion.
    {
        buildFunction(node->signature, node->body);
    }
    else
    {
        DeclarationBuilderBase::visitPrimaryExpr(node);
    }
}

void DeclarationBuilder::visitMethodDeclaration(go::MethodDeclarationAst* node)
{

    go::GoFunctionDeclaration* functionDeclaration = nullptr;
    QualifiedIdentifier typeIdentifier;
    auto containerType = go::getMethodRecvTypeIdentifier(node->methodRecv);

    {
        DUChainWriteLocker lock;
        typeIdentifier = identifierForNode(containerType);
        auto typeDeclaration = go::getTypeDeclaration(typeIdentifier, currentContext());
        auto toBeInjectedContext = typeDeclaration ? typeDeclaration->internalContext() : nullptr;
        if(!typeDeclaration || !typeDeclaration->internalContext())
        {
            openContext(node->methodName, node->methodName, DUContext::ContextType::Class, typeIdentifier);
            if(typeDeclaration)
            {
                typeDeclaration->setInternalContext(currentContext());
            }
            toBeInjectedContext = currentContext();
            closeContext();
        }

        injectContext(toBeInjectedContext);

        auto range = RangeInRevision(currentContext()->range().start, currentContext()->range().start);
        functionDeclaration = openDeclaration<go::GoFunctionDeclaration>(identifierForNode(node->methodName).last(), range);
        functionDeclaration->setAutoDeclaration(true);
        closeDeclaration();

        closeInjectedContext();
    }

    QualifiedIdentifier identifier;
    {
        DUChainReadLocker lock;
        identifier = functionDeclaration->qualifiedIdentifier();
    }

    openContext(node, editorFindRange(node, 0), DUContext::ContextType::Class, typeIdentifier);
    DUChainWriteLocker lock;
    auto functionDefinition = buildMethod(node->signature, node->body, node->methodName, functionDeclaration, m_session->commentBeforeToken(node->startToken-1), identifier);
    functionDeclaration->setType<go::GoFunctionType>(functionDefinition->type<go::GoFunctionType>());
    functionDeclaration->setKind(Declaration::Instance);
    lock.unlock();

    if(node->methodRecv->type && functionDefinition->internalContext())
    {
        //declare method receiver variable('this' or 'self' analog in Go)
        openContext(functionDefinition->internalContext());
        buildTypeName(node->methodRecv->type);
        if(node->methodRecv->star!= -1)
        {
            PointerType* ptype = new PointerType();
            ptype->setBaseType(lastType());
            injectType(PointerType::Ptr(ptype));
        }
        DUChainWriteLocker n;
        functionDefinition->setDeclaration(functionDeclaration);
        auto methodReceiverName = node->methodRecv->nameOrType;
        Declaration* thisVariable = openDeclaration<Declaration>(identifierForNode(methodReceiverName).last(),
                                                                 editorFindRange(methodReceiverName, 0));
        thisVariable->setAbstractType(lastType());
        closeDeclaration();
        closeContext();
    }
    closeContext();
}

void DeclarationBuilder::visitTypeSpec(go::TypeSpecAst* node)
{
    //first try setting comment before type name
    //if it doesn't exists, set comment before type declaration
    QByteArray comment = m_session->commentBeforeToken(node->startToken);
    if(comment.size() == 0)
        comment = m_lastTypeComment;
    setComment(comment);
    ClassDeclaration* decl;
    {
        DUChainWriteLocker lock;
        decl = openDeclaration<ClassDeclaration>(identifierForNode(node->name), editorFindRange(node->name, 0));
        //decl->setKind(Declaration::Namespace);
        decl->setKind(Declaration::Type);
        //force direct here because otherwise DeclarationId will mess up actual type declaration and method declarations
        //TODO perhaps we can do this with specialization or additional identity?
        decl->setAlwaysForceDirect(true);
    }
    m_contextIdentifier = identifierForNode(node->name);
    visitType(node->type);
    DUChainWriteLocker lock;
    //qCDebug(DUCHAIN) << lastType()->toString();
    decl->setType(lastType());
    auto structType = fastCast<StructureType*>(lastType().data());
    if(structType)
    {
        structType->setDeclaration(decl);
    }
    if(!lastContext())
    {
        openContext(node->name, DUContext::ContextType::Class, identifierForNode(node->name));
        closeContext();
    }
    decl->setInternalContext(lastContext());
    if(node->type && node->type->complexType && node->type->complexType->structType && node->type->complexType->structType->fieldDeclSequence)
    {
        auto sequence = node->type->complexType->structType->fieldDeclSequence;
        for(int i = 0; i<sequence->count(); ++i)
        {
            auto anonymousField = sequence->at(i)->element->anonFieldStar;
            if(anonymousField)
            {
                visitTypeName(anonymousField->typeName);

                StructureType::Ptr baseClassType = lastType().cast<StructureType>();
                if(baseClassType)
                {
                    auto baseClassDeclaration = baseClassType->declaration(topContext());
                    if(baseClassDeclaration && baseClassDeclaration->internalContext())
                    {
                        decl->internalContext()->addImportedParentContext(baseClassType->declaration(topContext())->internalContext());
                    }
                }
            }
            auto varId = sequence->at(i)->element->varid;
            if(varId && !sequence->at(i)->element->type)
            {
                buildTypeName(varId);

                StructureType::Ptr baseClassType = lastType().cast<StructureType>();
                if(baseClassType)
                {
                    auto baseClassDeclaration = baseClassType->declaration(topContext());
                    if(baseClassDeclaration && baseClassDeclaration->internalContext())
                    {
                        decl->internalContext()->addImportedParentContext(baseClassType->declaration(topContext())->internalContext());
                    }
                }
            }
        }
    }
    closeDeclaration();
    //qCDebug(DUCHAIN) << "Type" << identifierForNode(node->name) << " exit";
}

void DeclarationBuilder::visitImportSpec(go::ImportSpecAst* node)
{
    //prevent recursive imports
    //without preventing recursive imports. importing standart go library(2000+ files) takes minutes and sometimes never stops
    //thankfully go import mechanism doesn't need recursive imports(I think)
    //if(m_export)
        //return;
    QString import(identifierForIndex(node->importpath->import).toString());
    QList<ReferencedTopDUContext> contexts = m_session->contextForImport(import);
    if(contexts.empty())
        return;
 
    //usually package name matches directory, so try searching for that first
    QualifiedIdentifier packageName(import.mid(1, import.length()-2));
    bool firstContext = true;
    for(const ReferencedTopDUContext& context : contexts)
    {
        //don't import itself
        if(context.data() == topContext())
            continue;
        DeclarationPointer decl = go::checkPackageDeclaration(packageName.last(), context);
        if(!decl && firstContext)
        {
            decl = go::getFirstDeclaration(context); //package name differs from directory, so get the real name
            if(!decl)
                continue;
            DUChainReadLocker lock;
            packageName = decl->qualifiedIdentifier();
        }
        if(!decl) //contexts belongs to a different package
            continue;
        
        DUChainWriteLocker lock;
        if(firstContext) //only open declarations once per import(others are redundant)
        {
            setComment(decl->comment());
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
        }
        topContext()->addImportedParentContext(context.data());
        firstContext = false;
    }
    DUChainWriteLocker lock;
}

void DeclarationBuilder::visitSourceFile(go::SourceFileAst* node)
{
    setComment(m_session->commentBeforeToken(node->startToken));
    DUChainWriteLocker lock;
    Declaration* packageDeclaration = openDeclaration<Declaration>(identifierForNode(node->packageClause->packageName), editorFindRange(node->packageClause->packageName, 0));
    packageDeclaration->setKind(Declaration::Namespace);
    openContext(node, editorFindRange(node, 0), DUContext::Namespace, identifierForNode(node->packageClause->packageName));
    
    packageDeclaration->setInternalContext(currentContext());
    lock.unlock();
    m_thisPackage = identifierForNode(node->packageClause->packageName);
    //import package this context belongs to
    importThisPackage();
    importBuiltins();

    go::DefaultVisitor::visitSourceFile(node);
    closeContext();
    closeDeclaration();
}

void DeclarationBuilder::importThisPackage()
{
    QList<ReferencedTopDUContext> contexts = m_session->contextForThisPackage(document());
    if(contexts.empty())
        return;

    for(const ReferencedTopDUContext& context : contexts)
    {
        if(context.data() == topContext())
            continue;
        //import only contexts with the same package name
        DeclarationPointer decl = go::checkPackageDeclaration(m_thisPackage.last(), context);
        if(!decl)
            continue;
        //if our package doesn't have comment, but some file in out package does, copy it
        if(currentDeclaration<Declaration>()->comment().size() == 0 && decl->comment().size() != 0)
            currentDeclaration<Declaration>()->setComment(decl->comment());

        DUChainWriteLocker lock;
        //TODO Since package names are identical duchain should find declarations without namespace alias, right?
        
        //NamespaceAliasDeclaration* import = openDeclaration<NamespaceAliasDeclaration>(QualifiedIdentifier(globalImportIdentifier()), RangeInRevision());
        //import->setKind(Declaration::NamespaceAlias);
        //import->setImportIdentifier(packageName); //this needs to be actual package name
        //closeDeclaration();
        topContext()->addImportedParentContext(context.data());
    }
    DUChainWriteLocker lock;
}

void DeclarationBuilder::importBuiltins()
{
    QString builtinFile = go::Helper::getBuiltinFile();
    if(builtinFile.isEmpty() || IndexedString(builtinFile) == document())
        return;
    ReferencedTopDUContext builtins = go::Helper::getBuiltinContext();
    if(!builtins)
    {//schedule this file for reparsing, if builtins were not parsed yet
        m_session->rescheduleThisFile();
    }else
    {
        DUChainWriteLocker lock;
        //import builtins
        NamespaceAliasDeclaration* decl = openDeclaration<NamespaceAliasDeclaration>(QualifiedIdentifier(globalImportIdentifier()), RangeInRevision());
        decl->setKind(Declaration::NamespaceAlias);
        decl->setImportIdentifier(QualifiedIdentifier("_builtins"));
        closeDeclaration();
        topContext()->addImportedParentContext(builtins);
    }
}

void DeclarationBuilder::visitForStmt(go::ForStmtAst* node)
{
    openContext(node, editorFindRange(node, 0), DUContext::Other); //wrapper context
    if(node->range != -1 && node->autoassign != -1)
    {//manually infer types
        go::ExpressionVisitor exprVisitor(m_session, currentContext(), this);
        exprVisitor.visitRangeClause(node->rangeExpression);
        auto types = exprVisitor.lastTypes();
        if(!types.empty())
        {
            declareVariable(identifierAstFromExpressionAst(node->expression), types.first());
            if(types.size() > 1 && node->expressionList)
            {
                int typeIndex = 1;
                auto iter = node->expressionList->expressionsSequence->front(), end = iter;
                do
                {
                    if(typeIndex >= types.size()) //not enough types to declare all variables
                        break;
                    declareVariable(identifierAstFromExpressionAst(iter->element), types.at(typeIndex));
                    iter = iter->next;
                    typeIndex++;
                }
                while (iter != end);
            }
        }
    }
    DeclarationBuilderBase::visitForStmt(node);
    closeContext();
}

void DeclarationBuilder::visitSwitchStmt(go::SwitchStmtAst* node)
{
    openContext(node, editorFindRange(node, 0), DUContext::Other); //wrapper context
    if(node->typeSwitchStatement && node->typeSwitchStatement->typeSwitchGuard)
    {
        go::TypeSwitchGuardAst* typeswitch = node->typeSwitchStatement->typeSwitchGuard;
        go::ExpressionVisitor expVisitor(m_session, currentContext(), this);
        expVisitor.visitPrimaryExpr(typeswitch->primaryExpr);
        if(!expVisitor.lastTypes().empty())
        {
            declareVariable(typeswitch->ident, expVisitor.lastTypes().first());
            m_switchTypeVariable = identifierForNode(typeswitch->ident);
        }
    }
    DeclarationBuilderBase::visitSwitchStmt(node);
    closeContext(); //wrapper context
    m_switchTypeVariable.clear();
}

void DeclarationBuilder::visitTypeCaseClause(go::TypeCaseClauseAst* node)
{
    openContext(node, editorFindRange(node, 0), DUContext::Other);
    const KDevPG::ListNode<go::TypeAst*>* typeIter = 0;
    if(node->typelistSequence)
        typeIter = node->typelistSequence->front();
    if(node->defaultToken == -1 && typeIter && typeIter->next == typeIter)
    {//if default is not specified and only one type is listed
        //we open another declaration of listed type
        visitType(typeIter->element);
        lastType()->setModifiers(AbstractType::NoModifiers);
        DUChainWriteLocker lock;
        if(lastType()->toString() != "nil" && !m_switchTypeVariable.isEmpty())
        {//in that case we also don't open declaration
            Declaration* decl = openDeclaration<Declaration>(m_switchTypeVariable, editorFindRange(typeIter->element, 0));
            decl->setAbstractType(lastType());
            closeDeclaration();
        }
    }
    go::DefaultVisitor::visitTypeCaseClause(node);
    closeContext();
}

void DeclarationBuilder::visitExprCaseClause(go::ExprCaseClauseAst* node)
{
    openContext(node, editorFindRange(node, 0), DUContext::Other);
    go::DefaultVisitor::visitExprCaseClause(node);
    closeContext();
}

void DeclarationBuilder::visitCommCase(go::CommCaseAst* node)
{
    if(node->autoassign != -1)
    {
        go::ExpressionVisitor expVisitor(m_session, currentContext(), this);
        expVisitor.visitExpression(node->recvExp);
        auto types = expVisitor.lastTypes();
        if(types.size() == 2) //expression must be a receive operator, returning two values
        {
            declareVariable(identifierAstFromExpressionAst(node->sendOrRecv), types.first());
            if(node->expressionList)
            {
                auto secondVariable = node->expressionList->expressionsSequence->front();
                declareVariable(identifierAstFromExpressionAst(secondVariable->element), types.at(1));
            }
        }
    }
    DeclarationBuilderBase::visitCommCase(node);
}

void DeclarationBuilder::visitCommClause(go::CommClauseAst* node)
{
    openContext(node, editorFindRange(node, 0), DUContext::Other); //wrapper context
    DeclarationBuilderBase::visitCommClause(node);
    closeContext();
}



void DeclarationBuilder::visitTypeDecl(go::TypeDeclAst* node)
{
    m_lastTypeComment = m_session->commentBeforeToken(node->startToken);
    go::DefaultVisitor::visitTypeDecl(node);
    m_lastTypeComment = QByteArray();
}


go::GoFunctionDeclaration* DeclarationBuilder::declareFunction(go::IdentifierAst* id, const go::GoFunctionType::Ptr& type,
                                                               DUContext* paramContext, DUContext* retparamContext, const QByteArray& comment, DUContext* bodyContext)
{
    setComment(comment);
    DUChainWriteLocker lock;
    auto dec = openDefinition<go::GoFunctionDeclaration>(identifierForNode(id), editorFindRange(id, 0));
    dec->setType<go::GoFunctionType>(type);
    dec->setKind(Declaration::Instance);
    dec->setInternalContext(bodyContext);

    if(bodyContext)
    {
        if(paramContext)
        {
            bodyContext->addImportedParentContext(paramContext);
            dec->setInternalFunctionContext(paramContext);
        }
        if(retparamContext)
        {
            bodyContext->addImportedParentContext(retparamContext);
            dec->setReturnArgsContext(retparamContext);
        }
    }

    closeDeclaration();
    return dec;
}

go::GoFunctionDefinition* DeclarationBuilder::declareMethod(go::IdentifierAst *id, const go::GoFunctionType::Ptr &type,
                                                      DUContext *paramContext, DUContext *retparamContext,
                                                      const QByteArray &comment, DUContext *bodyContext,
                                                      go::GoFunctionDeclaration *declaration,
                                                      const QualifiedIdentifier &identifier)
{
    setComment(comment);
    DUChainWriteLocker lock;
    auto dec = openDefinition<go::GoFunctionDefinition>(identifier, editorFindRange(id, 0));
    dec->setType<go::GoFunctionType>(type);
    dec->setKind(Declaration::Instance);
    dec->setInternalContext(bodyContext);

    if(bodyContext)
    {
        if(paramContext)
        {
            bodyContext->addImportedParentContext(paramContext);
            dec->setInternalFunctionContext(paramContext);
        }
        if(retparamContext)
        {
            bodyContext->addImportedParentContext(retparamContext);
            dec->setReturnArgsContext(retparamContext);
        }
    }
    closeDeclaration();
    return dec;
}

go::GoFunctionDeclaration* DeclarationBuilder::buildFunction(go::SignatureAst* node, go::BlockAst* block,
                                                             go::IdentifierAst* name, const QByteArray& comment)
{
    DUContext* bodyContext = nullptr;
    if(block)
    {
        visitBlock(block);
        bodyContext = lastContext();
    }
    DUContext* returnArgsContext = nullptr;
    DUContext* parametersContext = nullptr;
    auto type = parseSignature(node, true, &parametersContext, &returnArgsContext, identifierForNode(name), comment);
    return declareFunction(name, type, parametersContext, returnArgsContext, comment, bodyContext);
}

go::GoFunctionDefinition* DeclarationBuilder::buildMethod(go::SignatureAst *node, go::BlockAst *block,
                                                          go::IdentifierAst *name, go::GoFunctionDeclaration *declaration,
                                                          const QByteArray &comment, const QualifiedIdentifier &identifier)
{
    DUContext* bodyContext = nullptr;
    if(block)
    {
        visitBlock(block);
        bodyContext = lastContext();
    }
    DUContext* parametersContext = nullptr;
    DUContext* returnArgsContext = nullptr;
    auto type = parseSignature(node, true, &parametersContext, &returnArgsContext, identifier, comment);
    return declareMethod(name, type, parametersContext, returnArgsContext, comment, bodyContext, declaration, identifier);
}
