/* KDevelop gometalinter support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "plugin.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/icore.h>
#include <language/interfaces/editorcontext.h>
#include <project/projectmodel.h>
#include <util/jobstatus.h>

#include <KCoreAddons/KPluginFactory>
#include <KI18n/KLocalizedString>
#include <kactioncollection.h>
#include <QtCore/QMimeDatabase>

K_PLUGIN_FACTORY_WITH_JSON(GoMetaLinterFactory, "gometalinter.json", registerPlugin<GoMetaLinter::Plugin>();)

namespace GoMetaLinter
{

Plugin::Plugin(QObject *parent, const QVariantList&)
    : IPlugin(QStringLiteral("gometalinter"), parent),
      m_currentProject(nullptr),
      m_model(new ProblemModel(this)),
      m_job(nullptr)
{
    setXMLFile(QStringLiteral("gometalinter.rc"));

    auto analyzeFile = [this](){ run(false); };
    auto analyzeProject = [this](){ run(true); };

    m_menuActionFile = new QAction(i18n("Analyze Current File with Go Meta Linter"), this);
    connect(m_menuActionFile, &QAction::triggered, this, analyzeFile);
    actionCollection()->addAction(QStringLiteral("gometalinter_file"), m_menuActionFile);

    m_menuActionProject = new QAction(i18n("Analyze Current Project with Go Meta Linter"), this);
    connect(m_menuActionProject, &QAction::triggered, this, analyzeProject);
    actionCollection()->addAction(QStringLiteral("gometalinter_project"), m_menuActionProject);

    auto documentController = core()->documentController();
    connect(documentController, &KDevelop::IDocumentController::documentClosed, this, &Plugin::updateActions);
    connect(documentController, &KDevelop::IDocumentController::documentActivated, this, &Plugin::updateActions);

    auto projectController = core()->projectController();
    connect(projectController, &KDevelop::IProjectController::projectOpened, this, &Plugin::updateActions);
    connect(projectController, &KDevelop::IProjectController::projectClosed, this, &Plugin::projectClosed);

    m_contextActionFile = new QAction(i18n("Go Meta Linter"), this);
    connect(m_contextActionFile, &QAction::triggered, this, analyzeFile);

    m_contextActionProject = new QAction(i18n("Go Meta Linter"), this);
    connect(m_contextActionProject, &QAction::triggered, this, analyzeProject);

    m_contextActionProjectItem = new QAction(i18n("Go Meta Linter"), this);

    updateActions();
}

Plugin::~Plugin()
{
    killJob();
}

void Plugin::run(bool checkProject)
{
    auto path = checkProject ? m_currentProject->path().toUrl() : core()->documentController()->activeDocument()->url();
    run(m_currentProject, path.toLocalFile());
}

void Plugin::run(KDevelop::IProject* project, const QString &path)
{
    auto workingDirectory = project->path().toUrl();
    m_model->reset(project, path);

    m_job = new Job(workingDirectory, path);
    connect(m_job, &Job::problemsDetected, m_model, &ProblemModel::addProblems);
    connect(m_job, &Job::finished, this, &Plugin::result);

    core()->uiController()->registerStatus(new KDevelop::JobStatus(m_job, QStringLiteral("Go Meta Linter")));
    core()->runController()->registerJob(m_job);

    m_model->show();

    updateActions();
}

void Plugin::updateActions()
{
    m_currentProject = nullptr;
    m_menuActionFile->setEnabled(false);
    m_menuActionProject->setEnabled(false);

    KDevelop::IDocument* activeDocument = core()->documentController()->activeDocument();

    if (!activeDocument) {
        return;
    }

    m_currentProject = core()->projectController()->findProjectForUrl(activeDocument->url());

    m_menuActionFile->setEnabled(m_currentProject);
    m_menuActionProject->setEnabled(m_currentProject);
}

void Plugin::projectClosed(KDevelop::IProject *project)
{
    if (project != m_model->project()) {
        return;
    }

    killJob();
    m_model->reset();
}

void Plugin::killJob()
{
    if(isRunning())
    {
        m_job->kill(KJob::EmitResult);
    }
}

void Plugin::result(KJob*)
{
    if (!core()->projectController()->projects().contains(m_model->project())) {
        m_model->reset();
    } else {
        m_model->show();
    }

    m_job = nullptr;

    updateActions();
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    if (context->hasType(KDevelop::Context::EditorContext) && m_currentProject && !isRunning()) {
        auto eContext = dynamic_cast<KDevelop::EditorContext*>(context);
        QMimeDatabase db;
        const auto mime = db.mimeTypeForUrl(eContext->url());

        if (mime.name() == QLatin1String("text/x-go")) {
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, m_contextActionFile);
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, m_contextActionProject);
        }
    }

    if (context->hasType(KDevelop::Context::ProjectItemContext) && !isRunning()) {
        auto pContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (pContext->items().size() != 1) {
            return extension;
        }

        auto item = pContext->items().first();

        switch (item->type()) {
            case KDevelop::ProjectBaseItem::File:
            case KDevelop::ProjectBaseItem::Folder:
            case KDevelop::ProjectBaseItem::BuildFolder:
                break;

            default:
                return extension;
        }

        m_contextActionProjectItem->disconnect();
        connect(m_contextActionProjectItem, &QAction::triggered, this, [this, item](){
            run(item->project(), item->path().toLocalFile());
        });

        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, m_contextActionProjectItem);
    }

    return extension;
}

bool Plugin::isRunning()
{
    return m_job;
}

}

#include "plugin.moc"