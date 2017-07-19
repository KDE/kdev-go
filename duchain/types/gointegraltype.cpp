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

#include "gointegraltype.h"

#include <language/duchain/types/typeregister.h>

using namespace KDevelop;

namespace go{

REGISTER_TYPE(GoIntegralType);  
    
GoIntegralType::GoIntegralType(const GoIntegralType& rhs)
  : IntegralType(copyData<GoIntegralType>(*rhs.d_func()))
{
}

GoIntegralType::GoIntegralType(GoIntegralTypeData& data)
  : IntegralType(data)
{
}

GoIntegralType::GoIntegralType(uint type)
  : IntegralType(createData<GoIntegralType>())
{
  setDataType(type);
  setModifiers(ConstModifier);
}


QString GoIntegralType::toString() const
{
    
    TYPE_D(GoIntegralType);

    QString name;

    switch (d->m_dataType) {
    case TypeUint8:
        name="uint8";
        break;
    case TypeUint16:
        name="uint16";
        break;
    case TypeUint32:
        name="uint32";
        break;
    case TypeUint64:
        name="uint64";
        break;
    case TypeInt8:
        name="int8";
        break;
    case TypeInt16:
        name="int16";
        break;
    case TypeInt32:
        name="int32";
        break;
    case TypeInt64:
        name="int64";
        break;
    case TypeFloat32:
        name="float32";
        break;
    case TypeFloat64:
        name="float64";
        break;
    case TypeComplex64:
        name="complex64";
        break;
    case TypeComplex128:
        name="complex128";
        break;
    case TypeRune:
        name="rune";
        break;
    case TypeUint:
        name="uint";
        break;
    case TypeInt:
        name="int";
        break;
    case TypeUintptr:
        name="uintptr";
        break;
    case TypeString:
        name="string";
        break;
    case TypeBool:
        name="bool";
        break;
    case TypeByte:
        name="byte";
        break;
    }

    return /*AbstractType::toString() + */name;
}


KDevelop::AbstractType* GoIntegralType::clone() const
{
    return new GoIntegralType(*this);
}

uint GoIntegralType::hash() const
{
    return 4 * KDevelop::IntegralType::hash();
}

bool GoIntegralType::equals(const KDevelop::AbstractType* rhs) const
{
    if( this == rhs ) {
        return true;
    }

    if ( !IntegralType::equals(rhs) ) {
        return false;
    }

    Q_ASSERT( fastCast<const GoIntegralType*>(rhs) );

    const GoIntegralType* type = static_cast<const GoIntegralType*>(rhs);

    return d_func()->m_dataType == type->d_func()->m_dataType;
}


}