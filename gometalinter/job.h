/* KDevelop gometalinter support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef GOMETALINTER_JOB_H
#define GOMETALINTER_JOB_H

#include <interfaces/iproblem.h>
#include <outputview/outputexecutejob.h>
#include <util/path.h>

class QElapsedTimer;

namespace GoMetaLinter
{

class Job : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    explicit Job(const QUrl &workingDirectory, const QString &path, QObject *parent = nullptr);
    ~Job() override;

    void start() override;

Q_SIGNALS:
    void problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems);

protected slots:
    void postProcessStdout(const QStringList& lines) override;

    void childProcessError(QProcess::ProcessError processError) override;

protected:
    QScopedPointer<QElapsedTimer> m_timer;

    KDevelop::Path m_projectRootPath;
};

}

#endif
