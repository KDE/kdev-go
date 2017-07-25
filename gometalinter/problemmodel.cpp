/* KDevelop gometalinter support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "problemmodel.h"

#include "plugin.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/editor/documentrange.h>
#include <shell/problemmodelset.h>

#include <klocalizedstring.h>

namespace GoMetaLinter
{

inline KDevelop::ProblemModelSet* problemModelSet()
{
    return KDevelop::ICore::self()->languageController()->problemModelSet();
}

static const QString problemModelId = QStringLiteral("Go Meta Linter");

ProblemModel::ProblemModel(Plugin* plugin)
    : KDevelop::ProblemModel(plugin),
      m_plugin(plugin),
      m_project(nullptr)
{
    setFeatures(CanDoFullUpdate | ScopeFilter | SeverityFilter | Grouping | CanByPassScopeFilter);
    reset();
    problemModelSet()->addModel(problemModelId, i18n("Go Meta Linter"), this);
}

ProblemModel::~ProblemModel()
{
    problemModelSet()->removeModel(problemModelId);
}

KDevelop::IProject* ProblemModel::project() const
{
    return m_project;
}

bool ProblemModel::problemExists(KDevelop::IProblem::Ptr newProblem)
{
    for (auto problem : m_problems) {
        if (newProblem->source() == problem->source() &&
            newProblem->severity() == problem->severity() &&
            newProblem->finalLocation() == problem->finalLocation() &&
            newProblem->description() == problem->description() &&
            newProblem->explanation() == problem->explanation())
            return true;
    }

    return false;
}

void ProblemModel::addProblems(const QVector<KDevelop::IProblem::Ptr>& problems)
{
    static int maxLength = 0;

    if (m_problems.isEmpty()) {
        maxLength = 0;
    }

    for (auto problem : problems) {

        if (problemExists(problem)) {
            continue;
        }

        m_problems.append(problem);
        addProblem(problem);

        // This performs adjusting of columns width in the ProblemsView
        if (maxLength < problem->description().length()) {
            maxLength = problem->description().length();
            setProblems(m_problems);
        }
    }
}

void ProblemModel::reset()
{
    reset(nullptr, QString());
}

void ProblemModel::reset(KDevelop::IProject* project, const QString& path)
{
    m_project = project;
    m_path = path;

    clearProblems();
    m_problems.clear();

    QString tooltip = i18nc("@info:tooltip", "Re-Run Last Go Meta Linter Analysis");
    setFullUpdateTooltip(tooltip);
}

void ProblemModel::show()
{
    problemModelSet()->showModel(problemModelId);
}

void ProblemModel::forceFullUpdate()
{
    if (m_project && !m_plugin->isRunning()) {
        m_plugin->run(m_project, m_path);
    }
}

}
