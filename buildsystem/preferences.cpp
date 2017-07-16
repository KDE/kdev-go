/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "preferences.h"

#include "ui_builddirchooser.h"
#include "utils.h"

#include <KFile>
#include <KUrlRequester>

GoPreferences::GoPreferences(KDevelop::IPlugin *plugin, const KDevelop::ProjectConfigOptions &options, QWidget *parent)
        : ConfigPage(plugin, nullptr, parent), m_project(options.project)
{
    m_buildDirChooser = new Ui::GoBuildDirChooser;
    m_buildDirChooser->setupUi(this);
    m_buildDirChooser->buildFolder->setMode(KFile::Directory|KFile::ExistingOnly);
    connect(m_buildDirChooser->buildFolder, &KUrlRequester::textChanged, this, &GoPreferences::changed);
    reset();
}

QString GoPreferences::name() const
{
    return QStringLiteral("Go");
}

void GoPreferences::reset()
{
    m_buildDirChooser->buildFolder->setUrl(Go::currentBuildDir(m_project).toUrl());
}
void GoPreferences::apply()
{
    Go::setCurrentBuildDir(m_project, KDevelop::Path(m_buildDirChooser->buildFolder->url()));
}
GoPreferences::~GoPreferences()
{
    delete(m_buildDirChooser);
}
