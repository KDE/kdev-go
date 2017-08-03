/* KDevelop Go codecompletion support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef GOLANGTYPEMATCH_H
#define GOLANGTYPEMATCH_H

#include <language/codecompletion/codecompletioncontext.h>
#include <language/codecompletion/codecompletionitem.h>

#include "kdevgocompletion_export.h"
#include <language/duchain/types/abstracttype.h>

namespace go
{

class KDEVGOCOMPLETION_EXPORT TypeMatch
{
public:
    TypeMatch();

    TypeMatch(const KDevelop::AbstractType::Ptr &m_singleType, const QList<KDevelop::AbstractType::Ptr> &m_multipleTypes);

    KDevelop::AbstractType::Ptr singleType() const;
    QList<KDevelop::AbstractType::Ptr> multipleTypes() const;

    void setSingleType(const KDevelop::AbstractType::Ptr &singleType);
    void setMultipleTypes(const QList<KDevelop::AbstractType::Ptr> &multipleTypes);

private:
    KDevelop::AbstractType::Ptr m_singleType;
    QList<KDevelop::AbstractType::Ptr> m_multipleTypes;
};

}

#endif //GOLANGTYPEMATCH_H
