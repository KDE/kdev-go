/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "buildsystem.h"

#include "builder.h"
#include "executabletargetitem.h"
#include "utils.h"
#include "builddirchooser.h"
#include "preferences.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <kpluginfactory.h>
#include <project/helper.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(BuildSystemFactory, "buildsystem.json", registerPlugin<GoBuildSystem>(); )

GoBuildSystem::GoBuildSystem( QObject *parent, const QVariantList& args )
: KDevelop::AbstractFileManagerPlugin( "gobuildsystem", parent ), m_builder(new GoBuilder())
{
    Q_UNUSED(args)
    setXMLFile( "buildsystem.rc" );

}

GoBuildSystem::~GoBuildSystem()
{
}

IProjectBuilder* GoBuildSystem::builder() const
{
    return m_builder;
}

Path::List GoBuildSystem::includeDirectories(KDevelop::ProjectBaseItem*) const
{
    return {};
}

Path::List GoBuildSystem::frameworkDirectories(KDevelop::ProjectBaseItem*) const
{
    return {};
}

QHash<QString,QString> GoBuildSystem::defines(KDevelop::ProjectBaseItem*) const
{
    return {};
}

QString GoBuildSystem::extraArguments(KDevelop::ProjectBaseItem*) const
{
    return {};
}

ProjectTargetItem* GoBuildSystem::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED(target)
    Q_UNUSED(parent)
    return nullptr;
}

bool GoBuildSystem::addFilesToTarget(const QList< ProjectFileItem* > &files, ProjectTargetItem* parent)
{
    Q_UNUSED( files )
    Q_UNUSED( parent )
    return false;
}

bool GoBuildSystem::removeTarget(KDevelop::ProjectTargetItem *target)
{
    Q_UNUSED( target )
    return false;
}

bool GoBuildSystem::removeFilesFromTargets(const QList< ProjectFileItem* > &targetFiles)
{
    Q_UNUSED( targetFiles )
    return false;
}

bool GoBuildSystem::hasBuildInfo(KDevelop::ProjectBaseItem* item) const
{
    Q_UNUSED(item);
    return false;
}

Path GoBuildSystem::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    auto project = item->project();
    ProjectFolderItem *folder = nullptr;
    do {
        folder = dynamic_cast<ProjectFolderItem*>(item);
        item = item->parent();
    } while (!folder && item);

    if(folder) {
        return folder->path();
    }

    return project->path();
}

QList<ProjectTargetItem*> GoBuildSystem::targets(KDevelop::ProjectFolderItem*) const
{
    return {};
}

KDevelop::ProjectFolderItem * GoBuildSystem::createFolderItem(KDevelop::IProject* project, const KDevelop::Path& path, KDevelop::ProjectBaseItem* parent)
{
    ProjectBuildFolderItem *item = new KDevelop::ProjectBuildFolderItem(project, path, parent);
    new GoExecutableTargetItem(item, item->folderName());

    return item;
}

int GoBuildSystem::perProjectConfigPages() const
{
    return 1;
}

KDevelop::ConfigPage* GoBuildSystem::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions &options, QWidget *parent)
{
    if (number == 0)
    {
        return new GoPreferences(this, options, parent);
    }
    return nullptr;
}

KDevelop::ProjectFolderItem *GoBuildSystem::import(KDevelop::IProject *project)
{
    auto buildDir = Go::currentBuildDir(project);
    if(buildDir.isEmpty())
    {
        auto newBuildDir = Path(project->path().parent(), project->name()+"-build");
        GoBuildDirChooser buildDirChooser;
        buildDirChooser.setBuildFolder(newBuildDir);
        if(buildDirChooser.exec())
        {
            newBuildDir = buildDirChooser.buildFolder();
        }
        Go::setCurrentBuildDir(project, newBuildDir);
    }
    return AbstractFileManagerPlugin::import(project);
}

#include "buildsystem.moc"
