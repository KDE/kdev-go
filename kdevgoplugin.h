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

/**
 * This plugin is heavily influenced by kdev-php and kdev-qmljs plugins.
 * If you have problems figuring out how something works, try looking for
 * similar code in these plugins, it should be better documented there.
 */

#ifndef KDEVGOPLUGIN_H
#define KDEVGOPLUGIN_H

#include <interfaces/iplugin.h>
#include <language/interfaces/ilanguagesupport.h>

#include "gohighlighting.h"

namespace KDevelop
{
  class IProject;
  class IDocument;
  class ParseJob;
}

class GoPlugin : public KDevelop::IPlugin, public KDevelop::ILanguageSupport
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::ILanguageSupport )
  public:
    explicit GoPlugin(QObject* parent, const QVariantList &args);
    
    virtual ~GoPlugin();
    
    virtual KDevelop::ILanguage* language() override;
    
    virtual KDevelop::ParseJob* createParseJob(const KDevelop::IndexedString& url) override;
    virtual QString name() const override;
    
    KDevelop::ICodeHighlighting* codeHighlighting() const;
    
private:
    Highlighting* m_highlighting;
};
#endif