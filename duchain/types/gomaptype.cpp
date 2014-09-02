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

#include "gomaptype.h"

#include <language/duchain/types/typeregister.h>

using namespace KDevelop;

namespace go
{
REGISTER_TYPE(GoMapType);

GoMapType::GoMapType(const GoMapType& rhs)
  : KDevelop::AbstractType(copyData<GoMapType>(*rhs.d_func()))
{
}

GoMapType::GoMapType(GoMapTypeData& data)
  : KDevelop::AbstractType(data)
{
}

GoMapType::GoMapType()
  : KDevelop::AbstractType(createData<GoMapType>())
{
}

void GoMapType::setKeyType(AbstractType::Ptr type)
{
    d_func_dynamic()->keyType = type->indexed();
}

void GoMapType::setValueType(AbstractType::Ptr type)
{
    d_func_dynamic()->valueType = type->indexed();
}

AbstractType::Ptr GoMapType::keyType()
{
    return d_func()->keyType.abstractType();
}

AbstractType::Ptr GoMapType::valueType()
{
    return d_func()->valueType.abstractType();
}

QString GoMapType::toString() const
{
    QString typeName("map["+d_func()->keyType.abstractType()->toString()+"]");
    typeName.append(d_func()->valueType.abstractType()->toString());
    return typeName;
}

KDevelop::AbstractType* GoMapType::clone() const
{
    return new GoMapType(*this);
}

uint GoMapType::hash() const
{
    uint hash = 6 * KDevelop::AbstractType::hash();
    hash += 6 * d_func()->keyType.hash();
    hash += 6 * d_func()->valueType.hash();
    return hash;
}

void GoMapType::accept0(TypeVisitor *v) const
{
    v->visit(this);
}

bool GoMapType::equals(const AbstractType* rhs) const
{
    if(this == rhs)
        return true;

    if(!AbstractType::equals(rhs))
        return false;

    Q_ASSERT( fastCast<const GoMapType*>(rhs) );

    const GoMapType* type = static_cast<const GoMapType*>(rhs);

    if(d_func()->keyType != type->d_func()->keyType)
        return false;

    if(d_func()->valueType != type->d_func()->valueType)
        return false;

    return true;
}

}
