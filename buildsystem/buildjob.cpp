/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "buildjob.h"

#include <outputview/outputfilteringstrategies.h>

using namespace KDevelop;

GoBuildJob::GoBuildJob(QObject* parent, QString command, QUrl buildDir, QString resultDir) : OutputExecuteJob(parent), m_command(command), m_output(resultDir)
{
    setStandardToolView(IOutputView::BuildView);
    setFilteringStrategy(new CompilerFilterStrategy(buildDir.toString()));
    setWorkingDirectory(buildDir);
    setProperties(KDevelop::OutputExecuteJob::NeedWorkingDirectory | KDevelop::OutputExecuteJob::DisplayStderr | KDevelop::OutputExecuteJob::IsBuilderHint);
}

QStringList GoBuildJob::commandLine() const
{
    return {"go", m_command, "-o", m_output};
}
