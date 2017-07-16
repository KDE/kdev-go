/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "executabletargetitem.h"

#include "utils.h"

GoExecutableTargetItem::GoExecutableTargetItem(KDevelop::ProjectFolderItem* parent, const QString& name)
        : KDevelop::ProjectExecutableTargetItem(parent->project(), name, parent)
{}

QUrl GoExecutableTargetItem::builtUrl() const
{
    return Go::buildOutputFile(parent()).toUrl();
}

QUrl GoExecutableTargetItem::installedUrl() const
{
    return QUrl();
}
