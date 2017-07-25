/* KDevelop gometalinter support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef GOMETALINTER_PROBLEMMODEL_H
#define GOMETALINTER_PROBLEMMODEL_H

#include <shell/problemmodel.h>

namespace KDevelop
{
    class IProject;
}

namespace GoMetaLinter
{

class Plugin;

class ProblemModel : public KDevelop::ProblemModel
{
public:
    explicit ProblemModel(Plugin* plugin);
    ~ProblemModel() override;

    KDevelop::IProject* project() const;

    void addProblems(const QVector<KDevelop::IProblem::Ptr>& problems);

    void reset();
    void reset(KDevelop::IProject* project, const QString& path);

    void show();

    void forceFullUpdate() override;

private:
    bool problemExists(KDevelop::IProblem::Ptr newProblem);

    using KDevelop::ProblemModel::setProblems;

    Plugin* m_plugin;

    KDevelop::IProject* m_project;
    QString m_path;

    QVector<KDevelop::IProblem::Ptr> m_problems;
};

}

#endif