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

#include "functiondeclaration.h"
#include <language/duchain/duchainregister.h>

using namespace KDevelop;

namespace go {
  
REGISTER_DUCHAIN_ITEM(GoFunctionDeclaration);

GoFunctionDeclaration::GoFunctionDeclaration(const go::GoFunctionDeclaration& rhs): 
			    FunctionDeclaration(*new GoFunctionDeclarationData(*rhs.d_func()))
{

}

GoFunctionDeclaration::GoFunctionDeclaration(const RangeInRevision& range, DUContext* context): 
			    FunctionDeclaration(*new GoFunctionDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context) {
        setContext(context);
    }
}

GoFunctionDeclaration::GoFunctionDeclaration(go::GoFunctionDeclarationData& data): FunctionDeclaration(data)
{

}

GoFunctionDeclaration::GoFunctionDeclaration(go::GoFunctionDeclarationData& data, const RangeInRevision& range): 
								FunctionDeclaration(data, range)
{

}

Declaration* GoFunctionDeclaration::clonePrivate() const
{
    return new GoFunctionDeclaration(*this);
}


QString GoFunctionDeclaration::toString() const
{
    return KDevelop::FunctionDeclaration::toString();
    //return QString("test func declaration");
}

void GoFunctionDeclaration::setReturnArgsContext(DUContext* context)
{
    d_func_dynamic()->returnContext = context;
}

DUContext* GoFunctionDeclaration::returnArgsContext() const
{
    return d_func()->returnContext.context();
}

}