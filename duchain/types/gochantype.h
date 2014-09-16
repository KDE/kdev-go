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

#ifndef GOLANGCHANTYPE_H
#define GOLANGCHANTYPE_H

#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/typesystem.h>
#include <language/duchain/types/indexedtype.h>

#include "duchain/goduchainexport.h"

namespace go
{

class GoChanTypeData : public KDevelop::AbstractTypeData
{
public:
    GoChanTypeData()
        : KDevelop::AbstractTypeData()
    {
        kind = 0;
    }
    GoChanTypeData( const GoChanTypeData& rhs )
        : KDevelop::AbstractTypeData(rhs),  valueType(rhs.valueType), kind(rhs.kind)
    {
    }

    KDevelop::IndexedType valueType;
    uint kind; //channel kind(0 = "chan", 1 = "<-chan", 2 = "chan <-")
};

class KDEVGODUCHAIN_EXPORT GoChanType: public KDevelop::AbstractType
{
public:
    typedef TypePtr<GoChanType> Ptr;

    /// Default constructor
    GoChanType();
    /// Copy constructor. \param rhs type to copy
    GoChanType(const GoChanType& rhs);
    /// Constructor using raw data. \param data internal data.
    GoChanType(GoChanTypeData& data);

    void setValueType(AbstractType::Ptr type);
    
    void setKind(uint kind);

    AbstractType::Ptr valueType();
    
    uint kind();

    virtual QString toString() const;

    virtual KDevelop::AbstractType* clone() const;

    virtual uint hash() const;

    virtual bool equals(const AbstractType* rhs) const;

    void accept0(KDevelop::TypeVisitor *v) const;

    enum ChanKind{
        SendAndReceive=0,
        Receive=1,
        Send=2
    };
    
    enum {
        Identity = 106
    };

    typedef GoChanTypeData Data;
    typedef KDevelop::AbstractType BaseType;

protected:
    TYPE_DECLARE_DATA(GoChanType);
};

}

namespace KDevelop
{

template<>
inline go::GoChanType* fastCast<go::GoChanType*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeAbstract ) {
        return 0;
    } else {
        return dynamic_cast<go::GoChanType*>(from);
    }
}

}


#endif