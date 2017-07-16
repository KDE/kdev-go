/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef BUILDDIRCHOOSER_H
#define BUILDDIRCHOOSER_H

#include <util/path.h>
#include "gobuildsystem_export.h"

#include <QDialog>
#include <QFlags>

class QDialogButtonBox;

namespace Ui {
    class GoBuildDirChooser;
}
namespace KDevelop {
    class IProject;
}

class KDEVGOBUILDSYSTEM_EXPORT GoBuildDirChooser : public QDialog
{
    Q_OBJECT
    public:
        explicit GoBuildDirChooser(QWidget* parent = nullptr);
        ~GoBuildDirChooser() override;

        KDevelop::Path buildFolder() const;
        void setBuildFolder(const KDevelop::Path &path);
    private:
        Ui::GoBuildDirChooser* m_chooserUi;
        QDialogButtonBox* m_buttonBox;
};


#endif // BUILDDIRCHOOSER_H
