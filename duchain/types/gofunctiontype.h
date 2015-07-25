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

#ifndef GOLANGFUNCTYPE_H
#define GOLANGFUNCTYPE_H

#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/typesystemdata.h>
//#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/indexedtype.h>
#include <duchain/kdevgoduchain_export.h>

using namespace KDevelop;

namespace go {

DECLARE_LIST_MEMBER_HASH(GoFunctionTypeData, m_returnArgs, IndexedType)

class GoFunctionTypeData : public KDevelop::FunctionTypeData
{
public:
    GoFunctionTypeData() : KDevelop::FunctionTypeData()
    {
	initializeAppendedLists(m_dynamic);
    }

    GoFunctionTypeData(const GoFunctionTypeData& rhs) : KDevelop::FunctionTypeData(rhs)
    {
	//foreach( KDevelop::IndexedType type, rhs.m_returnArgs)
		//m_returnArgs.append(type);
	initializeAppendedLists(m_dynamic);
	copyListsFrom(rhs);
    }


    START_APPENDED_LISTS_BASE(GoFunctionTypeData, AbstractTypeData);

    //APPENDED_LIST_FIRST(GoFunctionTypeData, IndexedType, m_returnArgs);
    APPENDED_LIST(GoFunctionTypeData, IndexedType, m_returnArgs, m_arguments);

    END_APPENDED_LISTS(GoFunctionTypeData, m_returnArgs);
};

class KDEVGODUCHAIN_EXPORT GoFunctionType : public KDevelop::FunctionType
{
public:
    typedef KDevelop::TypePtr<GoFunctionType> Ptr;
    
    GoFunctionType();
    
    GoFunctionType(GoFunctionTypeData& data);
    
    GoFunctionType(const GoFunctionType& rhs);
    
    virtual QString toString() const;
    
    virtual KDevelop::AbstractType* clone() const;

    virtual uint hash() const;
    
    void addReturnArgument(AbstractType::Ptr arg);
    
    QList<AbstractType::Ptr> returnArguments() const;
    
    virtual bool equals(const KDevelop::AbstractType* rhs) const;

    enum {
        Identity = 79
    };

    enum {
	VariadicArgument=1<<12
    };

  typedef GoFunctionTypeData Data;
  typedef KDevelop::FunctionType BaseType;
  
protected:
    TYPE_DECLARE_DATA(GoFunctionType);
    
};

}

namespace KDevelop
{

template<>
inline go::GoFunctionType* fastCast<go::GoFunctionType*>(AbstractType* from) {
    if ( !from || from->whichType() != AbstractType::TypeFunction ) {
        return 0;
    } else {
        return dynamic_cast<go::GoFunctionType*>(from);
    }
}

}

#endif