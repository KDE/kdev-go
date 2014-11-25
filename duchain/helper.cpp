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

#include <QReadLocker>
#include <QProcess>

namespace go
{

QList<QString> Helper::m_CachedSearchPaths;

QList< QString > Helper::getSearchPaths(QUrl document)
{
    QList<QString> paths;
    if(document != QUrl())
    {//try to find path automatically for opened documents
        QDir currentDir(document.adjusted(QUrl::RemoveFilename).path());
        //qCDebug(Go) << currentDir.dirName();
        while(currentDir.exists() && currentDir.dirName() != "src")
            if(!currentDir.cdUp())
                break;
        if(currentDir.exists() && currentDir.dirName() == "src")
            paths.append(currentDir.absolutePath());
    }

    if(Helper::m_CachedSearchPaths.empty())
    {
        //check $GOPATH env var
        QByteArray result = qgetenv("GOPATH");
        if(!result.isEmpty())
        {
            QDir path(result);
            if(path.exists() && path.cd("src") && path.exists())
                m_CachedSearchPaths.append(path.absolutePath());
        }
        //then check $GOROOT
        //these days most people don't set GOROOT manually
        //instead go tool can find correct value for GOROOT on its own
        //in order for this to work go exec must be in $PATH
        QProcess p;
        p.start("go env GOROOT");
        p.waitForFinished();
        result = p.readAllStandardOutput();
        if(result.endsWith("\n"))
            result.remove(result.length()-1, 1);
        if(!result.isEmpty())
        {
            //since Go 1.4 stdlib packages are stored in $GOROOT/src/
            //but we also support old layout $GOROOT/src/pkg/
            QDir path = QDir(result);
            if(path.exists() && path.cd("src") && path.exists())
            {
                m_CachedSearchPaths.append(path.absolutePath());
                if(path.cd("pkg") && path.exists())
                    m_CachedSearchPaths.append(path.absolutePath());
            }
        }
    }
    paths.append(m_CachedSearchPaths);
    return paths;
}


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