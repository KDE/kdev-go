/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "utils.h"

#include <kconfiggroup.h>
#include <project/projectmodel.h>
#include <interfaces/icore.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/iplugincontroller.h>

using namespace KDevelop;

namespace Config
{
    static const QString buildDirPathKey = QStringLiteral("Build Directory Path");
    static const QString groupName = QStringLiteral("Go");
}

namespace Go
{

KConfigGroup baseGroup( KDevelop::IProject* project )
{
    return project ? project->projectConfiguration()->group( Config::groupName ) : KConfigGroup();
}

KDevelop::Path currentBuildDir(KDevelop::IProject *project)
{
    return KDevelop::Path(baseGroup(project).readEntry(Config::buildDirPathKey, QString()));
}

void setCurrentBuildDir(KDevelop::IProject *project, const KDevelop::Path& path)
{
    baseGroup(project).writeEntry(Config::buildDirPathKey, path.toLocalFile());
}

KDevelop::Path buildOutputFile(KDevelop::ProjectBaseItem *item)
{
    auto buildDir = currentBuildDir(item->project());

    auto project = item->project();
    ProjectFolderItem *folder = nullptr;

    if(!(folder = dynamic_cast<ProjectFolderItem*>(item)))
    {
        folder = dynamic_cast<ProjectFolderItem*>(item->parent());
    }

    auto folderPath = project->path().relativePath(folder->path());
    auto result = Path(buildDir.cd(folderPath), folder->folderName());

    return result;
}

}
