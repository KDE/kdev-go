/* KDevelop Go codecompletion support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "typematch.h"

namespace go
{

TypeMatch::TypeMatch() : m_singleType(nullptr), m_multipleTypes({})
{
}

TypeMatch::TypeMatch(const KDevelop::AbstractType::Ptr &m_singleType,
                     const QList<KDevelop::AbstractType::Ptr> &m_multipleTypes)
    : m_singleType(m_singleType), m_multipleTypes(m_multipleTypes)
{

}

KDevelop::AbstractType::Ptr TypeMatch::singleType() const
{
    return m_singleType;
}

QList<KDevelop::AbstractType::Ptr> TypeMatch::multipleTypes() const
{
    return m_multipleTypes;
}

void TypeMatch::setSingleType(const KDevelop::AbstractType::Ptr &singleType)
{
    m_singleType = singleType;
}

void TypeMatch::setMultipleTypes(const QList<KDevelop::AbstractType::Ptr> &multipleTypes)
{
    m_multipleTypes = multipleTypes;
}

}
