/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef BUILDJOB_H
#define BUILDJOB_H

#include <outputview/outputexecutejob.h>

class GoBuildJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT
public:

    GoBuildJob(QObject* parent, QString command, QUrl buildDir, QString resultDir);
    QStringList commandLine() const override;
private:
    QString m_command;
    QString m_output;
};
#endif // BUILDJOB_H

