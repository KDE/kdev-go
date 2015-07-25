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

#ifndef GOLANGFUNCDECL_H
#define GOLANGFUNCDECL_H

#include <language/duchain/functiondeclaration.h>

#include "kdevgoduchain_export.h"

namespace go {

class GoFunctionDeclarationData : public KDevelop::FunctionDeclarationData
{
public:
    GoFunctionDeclarationData() : KDevelop::FunctionDeclarationData()
    {
    }
    
    GoFunctionDeclarationData(const GoFunctionDeclarationData& rhs) : KDevelop::FunctionDeclarationData(rhs), returnContext(rhs.returnContext)
    {
    }
    
    KDevelop::IndexedDUContext returnContext;
};

//typedef KDevelop::FunctionDeclarationData GoFunctionDeclarationData;
    
    
class KDEVGODUCHAIN_EXPORT GoFunctionDeclaration : public KDevelop::FunctionDeclaration
{
public:
    
    GoFunctionDeclaration(const GoFunctionDeclaration& rhs);
    GoFunctionDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    GoFunctionDeclaration(GoFunctionDeclarationData& data);
    GoFunctionDeclaration(GoFunctionDeclarationData& data, const KDevelop::RangeInRevision& range);
    
    
    virtual QString toString() const;
    
    void setReturnArgsContext(KDevelop::DUContext* context);
    
    KDevelop::DUContext* returnArgsContext() const;
    
    enum {
	Identity = 121 
    };
    
    virtual KDevelop::Declaration* clonePrivate() const;
    
private:
    DUCHAIN_DECLARE_DATA(GoFunctionDeclaration);
    
};

}

#endif