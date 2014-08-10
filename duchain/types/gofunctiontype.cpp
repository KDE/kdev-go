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

#include "gofunctiontype.h"

#include <language/duchain/types/typeregister.h>

using namespace KDevelop;

namespace go
{
    
REGISTER_TYPE(GoFunctionType);
    
DEFINE_LIST_MEMBER_HASH(GoFunctionTypeData, m_returnArgs, IndexedType)

GoFunctionType::GoFunctionType(GoFunctionTypeData& data)
  : FunctionType(data)
{
}

GoFunctionType::GoFunctionType(const GoFunctionType& rhs)
  : FunctionType(copyData<GoFunctionType>(*rhs.d_func()))
{
}

GoFunctionType::GoFunctionType()
  : FunctionType(createData<GoFunctionType>())
{
}

//TODO cleanup this code

void GoFunctionType::addReturnArgument(AbstractType::Ptr arg)
{
    Q_ASSERT(arg);
    d_func_dynamic()->m_returnArgsList().append(arg->indexed());
    //kDebug() << " size check: "<< d_func()->m_returnArgs.size();
    //makeDynamic();
    //kDebug() << "size check: " <<  d_func()->m_returnArguments.size();
}
    
QList<AbstractType::Ptr> GoFunctionType::returnArguments() const
{
    QList<AbstractType::Ptr> ret;
    FOREACH_FUNCTION(const IndexedType& arg , d_func()->m_returnArgs)
	ret << arg.abstractType();
    return ret;
    //QList<AbstractType::Ptr> list;
    //for(const AbstractType::Ptr& type : d_func()->m_returnArguments)
	//list.append(type);
    //return list;
}
    
QString GoFunctionType::toString() const
{
    //return FunctionType::toString();
    QString output = "function (";
    auto args = arguments();
    for(const AbstractType::Ptr& type : args)
    {
	 output = output.append(type.isNull() ? "<no type>" : type->toString());
	 if(args.back() != type)
	    output = output.append(", ");
    }
    output = output.append(") ");
    //auto rargs = d_func()->m_returnArgs;
    auto rargs = returnArguments();
    //kDebug() << "size check again: " << d_func()->m_returnArgs.size();
    if(rargs.size() == 0)
	return output;
    if(rargs.size() == 1)
    {
	if(rargs.first().isNull())
	    return output.append("<no type>");
	//kDebug() << rargs.first().abstractType()->toString();
	return output.append(rargs.first()->toString());
    }
    
    output = output.append("(");
    foreach(const AbstractType::Ptr& type,  rargs)
    {
	//auto type = idx.abstractType();
	output = output.append(type.isNull() ? "<no type>" : type->toString());
	if(rargs.back() != type)
	    output = output.append(", ");
    }
    return output.append(")");
}
    
KDevelop::AbstractType* GoFunctionType::clone() const
{
    return new GoFunctionType(*this);
}

uint GoFunctionType::hash() const
{
   /* KDevHash hash(6 * KDevelop::FunctionType::hash());
    //for(const IndexedType& idx : d_func()->m_returnArgsList())
    FOREACH_FUNCTION(const IndexedType& idx, d_func()->m_returnArgs)
	hash << idx.hash();
    return hash;*/
    
    
    uint h = 6 * FunctionType::hash();
    //for ( uint i = 0; i < d_func()->m_returnArgsSize(); i++ ) {
    FOREACH_FUNCTION(const IndexedType& idx, d_func()->m_returnArgs) {
        //h += d_func()->m_returnArgs()[i].hash();
	h += idx.hash();
    }
    return h;
}

bool GoFunctionType::equals(const AbstractType* rhs) const
{
    if ( this == rhs ) {
        return true;
    }
    if ( ! KDevelop::FunctionType::equals(rhs) ) {
        return false;
    }
    Q_ASSERT(fastCast<GoFunctionType*>(rhs) );
    const GoFunctionType* _rhs = static_cast<const GoFunctionType*>(rhs);
    TYPE_D(GoFunctionType);
    
    if ( d->m_returnArgsSize() != _rhs->d_func()->m_returnArgsSize())
	return false;
    
    for(unsigned int a = 0; a < d->m_returnArgsSize(); ++a)
	if(d->m_returnArgs()[a] != _rhs->d_func()->m_returnArgs()[a])
	return false;
    
    return true;
}
    
}