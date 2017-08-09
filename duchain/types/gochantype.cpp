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

#include "gochantype.h"

#include <language/duchain/types/typeregister.h>

using namespace KDevelop;

namespace go
{
REGISTER_TYPE(GoChanType);

GoChanType::GoChanType(const GoChanType& rhs)
  : KDevelop::AbstractType(copyData<GoChanType>(*rhs.d_func()))
{
}

GoChanType::GoChanType(GoChanTypeData& data)
  : KDevelop::AbstractType(data)
{
}

GoChanType::GoChanType()
  : KDevelop::AbstractType(createData<GoChanType>())
{
}

void GoChanType::setValueType(AbstractType::Ptr type)
{
    d_func_dynamic()->valueType = type->indexed();
}

void GoChanType::setKind(uint kind)
{
    d_func_dynamic()->kind = kind;
}

AbstractType::Ptr GoChanType::valueType()
{
    return d_func()->valueType.abstractType();
}

uint GoChanType::kind()
{
    return d_func()->kind;
}

QString GoChanType::toString() const
{
    QString typeName;
    if(d_func()->kind == ChanKind::Receive)
        typeName = "<- chan";
    else if(d_func()->kind == ChanKind::Send)
        typeName = "chan <-";
    else
        typeName = "chan";
    typeName = typeName + " "+d_func()->valueType.abstractType()->toString();
    return typeName;
}

KDevelop::AbstractType* GoChanType::clone() const
{
    return new GoChanType(*this);
}

uint GoChanType::hash() const
{
    uint hash = 6 * KDevelop::AbstractType::hash();
    hash += 6 * d_func()->valueType.hash();
    hash += 6 * d_func()->kind;
    return hash;
}

void GoChanType::accept0(TypeVisitor *v) const
{
    v->visit(this);
}

bool GoChanType::equals(const AbstractType* rhs) const
{
    if(this == rhs)
        return true;

    if(!AbstractType::equals(rhs))
        return false;

    Q_ASSERT( fastCast<const GoChanType*>(rhs) );

    const GoChanType* type = static_cast<const GoChanType*>(rhs);

    return d_func()->valueType == type->d_func()->valueType && d_func()->kind == type->d_func()->kind;
}

}
