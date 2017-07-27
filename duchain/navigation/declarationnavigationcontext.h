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

#ifndef GOLANGDECLNAVCONTEXT_H
#define GOLANGDECLNAVCONTEXT_H

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/types/identifiedtype.h>


class DeclarationNavigationContext : public KDevelop::AbstractDeclarationNavigationContext
{
public:
    DeclarationNavigationContext(KDevelop::DeclarationPointer decl,
                                 KDevelop::TopDUContextPointer topContext,
                                 KDevelop::AbstractNavigationContext* previousContext = 0);
protected: 
    void htmlFunction() override;
    
    void eventuallyMakeTypeLinks( KDevelop::AbstractType::Ptr type ) override;
    
    QString html(bool shorten) override;
};


#endif