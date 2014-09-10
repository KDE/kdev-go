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

#ifndef GOLANGMAPTYPE_H
#define GOLANGMAPTYPE_H

#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/typesystem.h>
#include <language/duchain/types/indexedtype.h>

#include "duchain/goduchainexport.h"

namespace go
{

class GoMapTypeData : public KDevelop::AbstractTypeData
{
public:
    GoMapTypeData()
        : KDevelop::AbstractTypeData()
    {
    }
    GoMapTypeData( const GoMapTypeData& rhs )
        : KDevelop::AbstractTypeData(rhs), keyType(rhs.keyType), valueType(rhs.valueType)
    {
    }

    KDevelop::IndexedType keyType;
    KDevelop::IndexedType valueType;
};

/**
 * I'm using custom map type insteadof MapType from KDevplatform because:
 *  - KDevplatform's MapType tied into UnsureType and stuff which is nod needed in kdev-go
 *  - Custom toString() implementation looks much better
 *  - This custom type is much simpler and therefore should be a little bit more efficient
 * If you have a convincing reason to use KDevplatform's MapType - do it, switching types 
 * shouldn't be hard. 
 */
class KDEVGODUCHAIN_EXPORT GoMapType: public KDevelop::AbstractType
{
public:
    typedef KDevelop::TypePtr<GoMapType> Ptr;

    /// Default constructor
    GoMapType();
    /// Copy constructor. \param rhs type to copy
    GoMapType(const GoMapType& rhs);
    /// Constructor using raw data. \param data internal data.
    GoMapType(GoMapTypeData& data);

    void setKeyType(AbstractType::Ptr type);

    void setValueType(AbstractType::Ptr type);

    AbstractType::Ptr keyType();

    AbstractType::Ptr valueType();

    virtual QString toString() const;

    virtual KDevelop::AbstractType* clone() const;

    virtual uint hash() const;

    virtual bool equals(const AbstractType* rhs) const;

    void accept0(KDevelop::TypeVisitor *v) const;

    enum {
        Identity = 105
    };

  typedef GoMapTypeData Data;
  typedef KDevelop::AbstractType BaseType;

protected:
    TYPE_DECLARE_DATA(GoMapType);
};

}

namespace KDevelop
{

template<>
inline go::GoMapType* fastCast<go::GoMapType*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeAbstract ) {
        return 0;
    } else {
        return dynamic_cast<go::GoMapType*>(from);
    }
}

}


#endif