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

#include "helper.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>

namespace go
{

DeclarationPointer getDeclaration(QualifiedIdentifier id, DUContext* context, bool searchInParent)
{
    DUChainReadLocker lock;
    if(context)
    {
	auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
	for(Declaration* decl: declarations)
	{
	    //import declarations are just decorations and need not be returned
	    if(decl->kind() == Declaration::Import)
		continue;
	    return DeclarationPointer(decl);
	}
    }
    return DeclarationPointer();
}

DeclarationPointer getTypeOrVarDeclaration(QualifiedIdentifier id, DUContext* context, bool searchInParent)
{
    DUChainReadLocker lock;
    if(context)
    {
	auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
	for(Declaration* decl : declarations)
	{
	    if((decl->kind() == Declaration::Import) || (decl->kind() == Declaration::Namespace) || (decl->kind() == Declaration::NamespaceAlias))
		continue; 
	    return DeclarationPointer(decl);
	}
    }
    return DeclarationPointer();
}

DeclarationPointer getTypeDeclaration(QualifiedIdentifier id, DUContext* context, bool searchInParent)
{
    DUChainReadLocker lock;
    if(context)
    {
	auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
	for(Declaration* decl : declarations)
	{
	    //TODO change this to just decl->kind() != Declaration::Type
	    if((decl->kind() == Declaration::Import) || (decl->kind() == Declaration::Namespace) 
		|| (decl->kind() == Declaration::NamespaceAlias) || (decl->kind() == Declaration::Instance))
		continue; 
	    return DeclarationPointer(decl);
	}
    }
    return DeclarationPointer();
}

QList< Declaration* > getDeclarations(QualifiedIdentifier id, DUContext* context, bool searchInParent)
{
    DUChainReadLocker lock;
    if(context)
    {
	QList<Declaration*> decls;
	auto declarations = context->findDeclarations(id, CursorInRevision(INT_MAX, INT_MAX));
	for(Declaration* decl: declarations)
	{
	    if(decl->kind() == Declaration::Import)
		continue;
	    decls << decl;
	}
	return decls;
    }
    return QList<Declaration*>();
}


DeclarationPointer getFirstDeclaration(DUContext* context, bool searchInParent)
{
    DUChainReadLocker lock;
    auto declarations = context->allDeclarations(CursorInRevision::invalid(), context->topContext(), searchInParent);
    if(declarations.size()>0)
	return DeclarationPointer(declarations.first().first);
    return DeclarationPointer();
}

DeclarationPointer checkPackageDeclaration(Identifier id, TopDUContext* context)
{
    DUChainReadLocker lock;
    auto declarations = context->findLocalDeclarations(id);
    if(declarations.size() > 0)
        return DeclarationPointer(declarations.first());
    return DeclarationPointer();
}


}