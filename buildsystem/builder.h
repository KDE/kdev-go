/* KDevelop go build support
 *
 * Copyright 2017 Mikhail Ivchenko <ematirov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#ifndef BUILDER_H
#define BUILDER_H

#include <project/interfaces/iprojectbuilder.h>

namespace KDevelop {
    class ProjectBaseItem;
}

class GoBuilder: public QObject, public KDevelop::IProjectBuilder
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectBuilder)
public:
    KJob* build(KDevelop::ProjectBaseItem *item) override;
    KJob* clean(KDevelop::ProjectBaseItem *item) override;
    KJob* install(KDevelop::ProjectBaseItem *item, const QUrl &installPath) override;

private:
    KJob *createJobForAction(KDevelop::ProjectBaseItem *item, const QString &action) const;
};

#endif // BUILDER_H

