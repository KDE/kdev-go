/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef UTILS_H
#define UTILS_H

#include "gobuildsystem_export.h"

#include <util/path.h>
#include <interfaces/iproject.h>

namespace Go
{
    /**
     * @returns the current build dir for the given project or an empty url if none
     * has been set by the user.
     */
    KDEVGOBUILDSYSTEM_EXPORT KDevelop::Path currentBuildDir(KDevelop::IProject* project);

    /**
     * @returns the current build output for the given project item
     */
    KDEVGOBUILDSYSTEM_EXPORT KDevelop::Path buildOutputFile(KDevelop::ProjectBaseItem *item);

    /**
     * Sets the build dir for the given project
     */
    KDEVGOBUILDSYSTEM_EXPORT void setCurrentBuildDir(KDevelop::IProject* project, const KDevelop::Path& path);
}

#endif // UTILS_H
