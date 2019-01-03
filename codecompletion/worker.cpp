/*************************************************************************************
*  Copyright (C) 2014 by Pavel Petrushkov <onehundredof@gmail.com>                  *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "worker.h"

#include "context.h"
#include "completiondebug.h"

namespace go
{
CodeCompletionWorker::CodeCompletionWorker(KDevelop::CodeCompletionModel* model): KDevelop::CodeCompletionWorker(model)
{

}

KDevelop::CodeCompletionContext* CodeCompletionWorker::createCompletionContext(const KDevelop::DUContextPointer& context,
                                                                               const QString& contextText,
                                                                               const QString& followingText,
                                                                               const KDevelop::CursorInRevision& position) const
{
    Q_UNUSED(followingText);
    qCDebug(COMPLETION) << "Completion test";
    //return go::CodeCompletionWorker::createCompletionContext(context, contextText, followingText, position);
    return new go::CodeCompletionContext(context, contextText, position);
}


}
