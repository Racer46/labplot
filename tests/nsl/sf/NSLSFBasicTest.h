/***************************************************************************
    File                 : NSLSFBasicTest.h
    Project              : LabPlot
    Description          : NSL Tests for the basic special functions
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef NSLSFBASICTEST_H
#define NSLSFBASICTEST_H

#include "../NSLTest.h"

class NSLSFBasicTest : public NSLTest {
	Q_OBJECT

private slots:
	// log2
	void testlog2_int_C99();
	void testlog2_int();
	void testlog2_longlong();
	void testlog2_int2();
	void testlog2_int3();
	void testlog2p1_int();
private:
	QString m_dataDir;
};
#endif
