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

#ifndef GOLANGINTTYPE_H
#define GOLANGINTTYPE_H

#include <language/duchain/types/integraltype.h>

#include "goduchainexport.h"

namespace go {
    
typedef KDevelop::IntegralTypeData GoIntegralTypeData;

class KDEVGODUCHAIN_EXPORT GoIntegralType : public KDevelop::IntegralType
{
public:
   typedef TypePtr<GoIntegralType> Ptr; 
    
    /// Default constructor
    GoIntegralType(uint type = TypeNone);
    /// Copy constructor. \param rhs type to copy
    GoIntegralType(const GoIntegralType& rhs);
    /// Constructor using raw data. \param data internal data.
    GoIntegralType(GoIntegralTypeData& data);

    virtual KDevelop::AbstractType* clone() const;
    
    virtual QString toString() const;

    virtual bool equals(const KDevelop::AbstractType* rhs) const;

    virtual uint hash() const;
    
    enum GoIntegralTypes {
       TypeUint8=201,
       TypeUint16,
       TypeUint32,
       TypeUint64,
       TypeInt8,
       TypeInt16,
       TypeInt32,
       TypeInt64,
       TypeFloat32,
       TypeFloat64,
       TypeComplex64,
       TypeComplex128,
       TypeRune,
       TypeUint,
       TypeInt,
       TypeUintptr,
       TypeString,
       TypeBool,
       TypeByte
   };
   
    enum {
        ///TODO: is that value OK?
        Identity = 78 
    };
    
  //GoIntegralType(uint type = TypeNone) : IntegralType(type) {}
   
  typedef KDevelop::IntegralTypeData Data;
  typedef KDevelop::IntegralType BaseType;
   
protected:
    TYPE_DECLARE_DATA(GoIntegralType);

};

}


namespace KDevelop
{

template<>
inline go::GoIntegralType* fastCast<go::GoIntegralType*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeIntegral ) {
        return 0;
    } else {
        return dynamic_cast<go::GoIntegralType*>(from);
    }
}

}


#endif