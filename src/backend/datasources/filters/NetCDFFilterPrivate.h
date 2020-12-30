/***************************************************************************
File                 : NetCDFFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for NetCDFFilter.
--------------------------------------------------------------------
Copyright            : (C) 2015-2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef NETCDFFILTERPRIVATE_H
#define NETCDFFILTERPRIVATE_H

#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif

class AbstractDataSource;

class NetCDFFilterPrivate {

public:
	explicit NetCDFFilterPrivate(NetCDFFilter*);

	void parse(const QString & fileName, QTreeWidgetItem* rootItem);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	QString readAttribute(const QString& fileName, const QString& name, const QString& varName);
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);
#ifdef HAVE_NETCDF
	static void handleError(int status, const QString& function);
	static QString translateFormat(int format);
	static QString translateDataType(nc_type type);
#endif

	const NetCDFFilter* q;

	QString currentVarName;
	int startRow{-1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};

private:
#ifdef HAVE_NETCDF
	int m_status;

	QString scanAttrs(int ncid, int varid, int attid, QTreeWidgetItem* parentItem = nullptr);
	void scanDims(int ncid, int ndims, QTreeWidgetItem* parentItem);
	void scanVars(int ncid, int nvars, QTreeWidgetItem* parentItem);
#endif
};

#endif
