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

#ifndef GOLANGSTRUCTTYPE_H
#define GOLANGSTRUCTTYPE_H

#include <language/duchain/types/structuretype.h>
#include <language/duchain/indexedducontext.h>
#include <language/duchain/types/typesystem.h>
#include <serialization/indexedstring.h>

#include "kdevgoduchain_export.h"


namespace go
{

class GoStructureTypeData : public KDevelop::StructureTypeData
{
public:
    GoStructureTypeData()
        : KDevelop::StructureTypeData()
    {
        m_type=0;
    }
    GoStructureTypeData( const GoStructureTypeData& rhs )
        : KDevelop::StructureTypeData(rhs), m_context(rhs.m_context), m_prettyName(rhs.m_prettyName), m_type(rhs.m_type)
    {
    }

    KDevelop::IndexedDUContext m_context;
    KDevelop::IndexedString m_prettyName;
    uint m_type;
};


class KDEVGODUCHAIN_EXPORT GoStructureType: public KDevelop::StructureType
{
public:
    typedef KDevelop::TypePtr<GoStructureType> Ptr;

    /// Default constructor
    GoStructureType();
    /// Copy constructor. \param rhs type to copy
    GoStructureType(const GoStructureType& rhs);
    /// Constructor using raw data. \param data internal data.
    GoStructureType(GoStructureTypeData& data);

    void setContext(KDevelop::DUContext* context);
    
    KDevelop::DUContext* context();
    
    void setPrettyName(QString name);
    
    void setStructureType() { d_func_dynamic()->m_type = 0; }
    void setInterfaceType() { d_func_dynamic()->m_type = 1; }
    
    
    void accept0(KDevelop::TypeVisitor *v) const;

    virtual QString toString() const;

    virtual KDevelop::AbstractType* clone() const;

    virtual uint hash() const;
    
    virtual bool equals(const AbstractType* rhs) const;

    enum {
        Identity = 104 
    };

  typedef GoStructureTypeData Data;
  typedef KDevelop::StructureType BaseType;

protected:
    TYPE_DECLARE_DATA(GoStructureType);
};

}

namespace KDevelop
{

template<>
inline go::GoStructureType* fastCast<go::GoStructureType*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeAbstract ) {
        return 0;
    } else {
        return dynamic_cast<go::GoStructureType*>(from);
    }
}

}


#endif