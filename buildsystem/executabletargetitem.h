/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef EXECUTABLETARGETITEM_H
#define EXECUTABLETARGETITEM_H

#include <project/projectmodel.h>

class GoExecutableTargetItem : public KDevelop::ProjectExecutableTargetItem
{
public:
    GoExecutableTargetItem(KDevelop::ProjectFolderItem* parent, const QString& name);

    QUrl builtUrl() const override;
    QUrl installedUrl() const override;
};


#endif // EXECUTABLETARGETITEM_H
