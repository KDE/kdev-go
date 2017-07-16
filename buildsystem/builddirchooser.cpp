/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "builddirchooser.h"

#include "ui_builddirchooser.h"
#include "utils.h"

#include <project/helper.h>
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <KFile>

using namespace KDevelop;

GoBuildDirChooser::GoBuildDirChooser(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure a build directory"));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    m_chooserUi = new Ui::GoBuildDirChooser;
    m_chooserUi->setupUi(mainWidget);

    m_chooserUi->buildFolder->setMode(KFile::Directory|KFile::ExistingOnly);
    mainLayout->addWidget(m_buttonBox);
}

GoBuildDirChooser::~GoBuildDirChooser()
{
    delete m_chooserUi;
}

KDevelop::Path GoBuildDirChooser::buildFolder() const
{
    return Path(m_chooserUi->buildFolder->url());
}
void GoBuildDirChooser::setBuildFolder(const KDevelop::Path &path)
{
    m_chooserUi->buildFolder->setUrl(path.toUrl());
}
