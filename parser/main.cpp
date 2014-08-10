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

#include "golexer.h"
#include "godebugvisitor.h"

#include "parsesession.h"

int main(int argc, char** argv)
{
    if(argc < 2)
	return 2;
    qDebug() << argv[1];
    QFile file(argv[1]);
    if(! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
	return 1;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QByteArray code = in.readAll().toUtf8();
    ParseSession session(code, 1);
    bool result=session.startParsing();
    
    go::DebugVisitor visitor(getLexer(session), code); 
    visitor.visitNode(session.ast()); 
    return !result ? 3 : 0;
}
