/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef GOLANGFUNCTIONDEFINITION_H
#define GOLANGFUNCTIONDEFINITION_H

#include <language/duchain/functiondefinition.h>

#include "kdevgoduchain_export.h"

namespace go {

class GoFunctionDefinitionData : public KDevelop::FunctionDefinitionData
{
public:
    GoFunctionDefinitionData() : KDevelop::FunctionDefinitionData()
    {
    }

    GoFunctionDefinitionData(const GoFunctionDefinitionData& rhs) : KDevelop::FunctionDefinitionData(rhs), returnContext(rhs.returnContext)
    {
    }

    KDevelop::IndexedDUContext returnContext;
};

class KDEVGODUCHAIN_EXPORT GoFunctionDefinition : public KDevelop::FunctionDefinition
{
public:
    GoFunctionDefinition(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    GoFunctionDefinition(GoFunctionDefinitionData& data);

    void setReturnArgsContext(KDevelop::DUContext* context);

    KDevelop::DUContext* returnArgsContext() const;

    enum {
        Identity = 122
    };
private:
    DUCHAIN_DECLARE_DATA(GoFunctionDefinition);

};

}

#endif