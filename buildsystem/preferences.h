/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <project/projectconfigpage.h>
#include <util/path.h>
#include <QtGui/QStandardItem>

namespace Ui { class GoBuildDirChooser; }

class GoPreferences : public KDevelop::ConfigPage
{
Q_OBJECT
public:
    explicit GoPreferences(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent = nullptr);
    ~GoPreferences() override;

    QString name() const override;

    void apply() override;
    void reset() override;
private:

    KDevelop::IProject* m_project;
    Ui::GoBuildDirChooser* m_buildDirChooser;
};

#endif // PREFERENCES_H
