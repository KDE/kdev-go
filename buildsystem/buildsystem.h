/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef BUILDSYSTEM_H
#define BUILDSYSTEM_H

#include <project/abstractfilemanagerplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>

class GoBuildSystem : public KDevelop::AbstractFileManagerPlugin,
                          public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IBuildSystemManager )
public:

    GoBuildSystem(QObject* parent = nullptr, const QVariantList& args = QVariantList());

    ~GoBuildSystem() override;

    Features features() const override { return Features(Folders | Targets | Files); }
    KDevelop::ProjectFolderItem* import(KDevelop::IProject* project) override;
    KDevelop::IProjectBuilder* builder() const override;
    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem*) const override;
    KDevelop::Path::List frameworkDirectories(KDevelop::ProjectBaseItem*) const override;
    QHash<QString,QString> defines(KDevelop::ProjectBaseItem*) const override;
    QString extraArguments(KDevelop::ProjectBaseItem* item) const override;
    KDevelop::ProjectTargetItem* createTarget(const QString& target, KDevelop::ProjectFolderItem *parent) override;
    bool addFilesToTarget(const QList<KDevelop::ProjectFileItem*> &files, KDevelop::ProjectTargetItem *parent) override;
    bool removeTarget(KDevelop::ProjectTargetItem *target) override;
    bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*>&) override;
    bool hasBuildInfo(KDevelop::ProjectBaseItem* item) const override;
    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;
    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const override;
    KDevelop::ProjectFolderItem* createFolderItem(KDevelop::IProject * project, const KDevelop::Path & path, KDevelop::ProjectBaseItem * parent) override;
    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

private:
    KDevelop::IProjectBuilder* m_builder;
};
#endif // BUILDSYSTEM_H
