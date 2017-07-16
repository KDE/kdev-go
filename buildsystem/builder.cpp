/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "builder.h"

#include "buildjob.h"
#include "buildsystem.h"
#include "utils.h"

#include <project/projectmodel.h>

KJob* GoBuilder::build( KDevelop::ProjectBaseItem *item )
{
    return createJobForAction(item, QStringLiteral("build"));
}

KJob* GoBuilder::clean( KDevelop::ProjectBaseItem *item )
{
    return createJobForAction(item, QStringLiteral("clean"));
}

KJob* GoBuilder::install(KDevelop::ProjectBaseItem *item, const QUrl &installPath)
{
    Q_UNUSED(installPath)
    return createJobForAction(item, QStringLiteral("install"));
}

KJob* GoBuilder::createJobForAction(KDevelop::ProjectBaseItem *item, const QString &action) const
{
    auto bsm = item->project()->buildSystemManager();
    auto buildDir = bsm->buildDirectory(item);
    auto job = new GoBuildJob(nullptr, action, buildDir.toUrl(), Go::buildOutputFile(item).toLocalFile());
    job->setWorkingDirectory(buildDir.toUrl());
    return job;
}
