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

#ifndef GOLANGDUCONTEXT_H
#define GOLANGDUCONTEXT_H

#include <language/duchain/ducontext.h>

class QWidget;

namespace KDevelop
{
    class Declaration;
    class TopDUContext;
}

namespace go {
  
template<class BaseContext>
class GoDUContext : public BaseContext
{
public:
  
    template<class Data>
    GoDUContext(Data& data) : BaseContext(data) {
    }
  
    ///Parameters will be reached to the base-class
    template<typename... Params>
    GoDUContext(Params... params) : BaseContext(params...) {
        static_cast<KDevelop::DUChainBase*>(this)->d_func_dynamic()->setClassId(this);
    }

    virtual QWidget* createNavigationWidget(KDevelop::Declaration* decl, KDevelop::TopDUContext* topContext,
                                            const QString& htmlPrefix, const QString& htmlSuffix) const override;

    enum {
        Identity = BaseContext::Identity + 51
    };
  
};

}

#endif