/* KDevelop gometalinter support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "job.h"

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <shell/problem.h>

#include <QApplication>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <language/editor/documentrange.h>

namespace GoMetaLinter
{


Job::Job(const QUrl &workingDirectory, const QString &path, QObject *parent)
    : KDevelop::OutputExecuteJob(parent)
    , m_timer(new QElapsedTimer)
{
    setJobName(i18n("Go Meta Linter Analysis"));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);

    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    setWorkingDirectory(workingDirectory);
    QStringList commandLine = {"gometalinter", "--aggregate", "--sort=path", path};
    *this << commandLine;
    m_projectRootPath = KDevelop::Path(workingDirectory);
}

Job::~Job()
{
    doKill();
}

void Job::postProcessStdout(const QStringList& lines)
{
    static const auto problemRegex = QRegularExpression(QStringLiteral("^([^:]*):([^:]*):([^:]*):([^:]*): ([^:]*)$"));

    QRegularExpressionMatch match;

    QVector<KDevelop::IProblem::Ptr> problems;

    foreach (const QString & line, lines) {
        match = problemRegex.match(line);
        if (match.hasMatch()) {
            KDevelop::IProblem::Ptr problem(new KDevelop::DetectedProblem(i18n("Go Meta Linter")));
            if(match.captured(4) == QStringLiteral("warning"))
            {
                problem->setSeverity(KDevelop::IProblem::Warning);
            }
            if(match.captured(4) == QStringLiteral("error"))
            {
                problem->setSeverity(KDevelop::IProblem::Error);
            }
            problem->setDescription(match.captured(5));
            KDevelop::DocumentRange range;
            range.document = KDevelop::IndexedString(KDevelop::Path(m_projectRootPath, match.captured(1)).toLocalFile());
            range.setBothLines(match.captured(2).toInt() - 1);
            if(!match.captured(3).isEmpty())
            {
                range.setBothColumns(match.captured(3).toInt() - 1);
            }
            problem->setFinalLocation(range);
            problems.append(problem);
        }
    }

    emit problemsDetected(problems);

    if (status() == KDevelop::OutputExecuteJob::JobStatus::JobRunning) {
        KDevelop::OutputExecuteJob::postProcessStdout(lines);
    }
}

void Job::start()
{
    m_timer->restart();
    KDevelop::OutputExecuteJob::start();
}

void Job::childProcessError(QProcess::ProcessError e)
{
    QString message;

    switch (e) {
    case QProcess::FailedToStart:
        message = i18n("Failed to start Go Meta Linter from \"%1\".", commandLine()[0]);
        break;

    case QProcess::Crashed:
        if (status() != KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
            message = i18n("Go Meta Linter crashed.");
        }
        break;

    case QProcess::Timedout:
        message = i18n("Go Meta Linter process timed out.");
        break;

    case QProcess::WriteError:
        message = i18n("Write to Go Meta Linter process failed.");
        break;

    case QProcess::ReadError:
        message = i18n("Read from Go Meta Linter process failed.");
        break;

    case QProcess::UnknownError:
        break;
    }

    if (!message.isEmpty()) {
        KMessageBox::error(qApp->activeWindow(), message, i18n("Go Meta Linter Error"));
    }

    KDevelop::OutputExecuteJob::childProcessError(e);
}

}
