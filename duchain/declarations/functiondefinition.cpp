/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "functiondefinition.h"
#include <language/duchain/duchainregister.h>

using namespace KDevelop;

namespace go {

REGISTER_DUCHAIN_ITEM(GoFunctionDefinition);

GoFunctionDefinition::GoFunctionDefinition(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : FunctionDefinition(range, context)
{

}

GoFunctionDefinition::GoFunctionDefinition(GoFunctionDefinitionData& data) : FunctionDefinition(data)
{

}

void GoFunctionDefinition::setReturnArgsContext(KDevelop::DUContext* context)
{
    d_func_dynamic()->returnContext = context;
}


DUContext* GoFunctionDefinition::returnArgsContext() const
{
    return d_func()->returnContext.context();
}

}