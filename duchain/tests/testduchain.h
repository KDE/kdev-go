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

#ifndef GOLANGTESTDUCHAIN_H
#define GOLANGTESTDUCHAIN_H

#include <QObject>

class TestDuchain : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void sanityCheck();
    void cleanupTestCase();
    void builtinFunctions_data();
    void builtinFunctions();
    void test_declareVariables();
    void test_constants();
    void test_constants_omittedType();
    void test_indexexpressions_data();
    void test_indexexpressions();
    void test_ifcontexts();
    void test_funccontexts();
    void test_rangeclause_data();
    void test_rangeclause();
    void test_typeswitch();
    void test_funcparams_data();
    void test_funcparams();
    void test_literals_data();
    void test_literals();
    void test_unaryOps_data();
    void test_unaryOps();
};


#endif