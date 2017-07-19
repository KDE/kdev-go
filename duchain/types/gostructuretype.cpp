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

#include "gostructuretype.h"

#include <language/duchain/types/typeregister.h>
#include <language/duchain/ducontext.h>

using namespace KDevelop;

namespace go
{
REGISTER_TYPE(GoStructureType);

GoStructureType::GoStructureType(const GoStructureType& rhs)
  : KDevelop::AbstractType(copyData<GoStructureType>(*rhs.d_func()))
{
}

GoStructureType::GoStructureType(GoStructureTypeData& data)
  : KDevelop::AbstractType(data)
{
}

GoStructureType::GoStructureType()
  : KDevelop::AbstractType(createData<GoStructureType>())
{
}

QString GoStructureType::toString() const
{
    if(d_func()->m_prettyName.index() == 0)
        return "structure type";
    return d_func()->m_prettyName.str();
}

void GoStructureType::setContext(DUContext* context)
{
    d_func_dynamic()->m_context = context;
}

DUContext* GoStructureType::context()
{
    return d_func()->m_context.data();
}


KDevelop::AbstractType* GoStructureType::clone() const
{
    return new GoStructureType(*this);
}

void GoStructureType::accept0(TypeVisitor *v) const
{
  v->visit (this);
}

uint GoStructureType::hash() const
{
    uint hash = 6 * KDevelop::AbstractType::hash();
    hash += 6 * d_func()->m_context.hash();
    hash += 6 * d_func()->m_prettyName.hash();
    return hash;
}

bool GoStructureType::equals(const AbstractType* rhs) const
{
    if(this == rhs)
        return true;
    
    if(!AbstractType::equals(rhs))
        return false;
    
    Q_ASSERT( fastCast<const GoStructureType*>(rhs) );

    const GoStructureType* type = static_cast<const GoStructureType*>(rhs);
    
    if(d_func()->m_context.topContextIndex() != type->d_func()->m_context.topContextIndex())
        return false;
    
    if(d_func()->m_context.localIndex() != type->d_func()->m_context.localIndex())
        return false;
    
    if(d_func()->m_context.context() != type->d_func()->m_context.context())
        return false;
    
    //if(d_func()->m_context.data()->localScopeIdentifier() != d_func()->m_context.data()->localScopeIdentifier())
        //return false;
    return true;
}

void GoStructureType::setPrettyName(QString name)
{
    if(name.size() > 40)
    {
        name = name.left(39);
        name.append("...");
    }
    d_func_dynamic()->m_prettyName = IndexedString(name);
}


}
