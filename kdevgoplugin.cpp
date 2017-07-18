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

#include "kdevgoplugin.h"

#include <godebug.h>
#include <KPluginFactory>
#include <KAboutData>
#include <language/codecompletion/codecompletion.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "codecompletion/model.h"
#include "golangparsejob.h"

K_PLUGIN_FACTORY_WITH_JSON(GoPluginFactory, "kdevgo.json", registerPlugin<GoPlugin>(); )

using namespace KDevelop;


GoPlugin::GoPlugin(QObject* parent, const QVariantList&)
    : KDevelop::IPlugin("kdevgoplugin", parent),
    ILanguageSupport()
{
    qCDebug(Go) << "Go Language Plugin is loaded\n";

    CodeCompletionModel* codeCompletion = new go::CodeCompletionModel(this);
    new KDevelop::CodeCompletion(this, codeCompletion, name());

    m_highlighting = new Highlighting(this);
}

GoPlugin::~GoPlugin()
{
}

ParseJob* GoPlugin::createParseJob(const IndexedString& url)
{
    qCDebug(Go) << "Creating golang parse job\n";
    return new GoParseJob(url, this); 
}

QString GoPlugin::name() const
{
  return "Golang"; 
}

KDevelop::ICodeHighlighting* GoPlugin::codeHighlighting() const
{
    return m_highlighting;
}

KDevelop::SourceFormatterItemList GoPlugin::sourceFormatterItems() const
{
    SourceFormatterItemList result;
    SourceFormatterStyleItem item;
    item.engine = "customscript";
    item.style = SourceFormatterStyle("Go fmt");
    item.style.setMimeTypes({SourceFormatterStyle::MimeHighlightPair{"text/x-go", "Go"}});
    item.style.setCaption("Go fmt support");
    item.style.setContent("gofmt $FILE");
    result.append(item);
    return result;
}

#include "kdevgoplugin.moc"
