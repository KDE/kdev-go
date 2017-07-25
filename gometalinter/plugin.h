/* KDevelop gometalinter support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef GOMETALINTER_PLUGIN_H
#define GOMETALINTER_PLUGIN_H

#include "problemmodel.h"
#include "job.h"

#include <interfaces/iplugin.h>
#include <interfaces/iproject.h>
#include <outputview/outputexecutejob.h>

namespace GoMetaLinter
{

class Plugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit Plugin(QObject *parent, const QVariantList&);
    ~Plugin() override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;
    void run(KDevelop::IProject* project, const QString &path);
    bool isRunning();
private:
    void updateActions();
    void projectClosed(KDevelop::IProject* project);
    void killJob();
    void result(KJob* job);
    void run(bool checkProject);

    KDevelop::IProject* m_currentProject;

    QAction* m_menuActionFile;
    QAction* m_menuActionProject;
    QAction* m_contextActionFile;
    QAction* m_contextActionProject;
    QAction* m_contextActionProjectItem;

    ProblemModel* m_model;
    Job* m_job;
};

}


#endif //KDEVGOPLUGIN_PLUGIN_H
