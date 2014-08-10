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

#include "navigation/navigationwidget.h"

#include "navigation/declarationnavigationcontext.h"
#include <language/duchain/topducontext.h>

using namespace KDevelop;

NavigationWidget::NavigationWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext, const QString& htmlPrefix, const QString& htmlSuffix)
{
    m_topContext = topContext;
    m_startContext = NavigationContextPointer(new DeclarationNavigationContext(DeclarationPointer(decl), m_topContext, nullptr));
    
    m_startContext->setPrefixSuffix(htmlPrefix, htmlSuffix);
    setContext(m_startContext);
}