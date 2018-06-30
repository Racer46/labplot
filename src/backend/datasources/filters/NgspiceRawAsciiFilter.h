/***************************************************************************
File                 : NgspiceRawAsciiFilter.h
Project              : LabPlot
Description          : Ngspice RAW ASCII filter
--------------------------------------------------------------------
Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)
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
#ifndef NGSPICERAWASCIIFILTER_H
#define NGSPICERAWASCIIFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"

class QStringList;
class NgspiceRawAsciiFilterPrivate;

class NgspiceRawAsciiFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	NgspiceRawAsciiFilter();
	~NgspiceRawAsciiFilter() override;

	static bool isNgspiceAsciiFile(const QString& fileName);

	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
	                                      AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1) override;
	void write(const QString& fileName, AbstractDataSource*) override;

	QVector<QStringList> preview(const QString& fileName, int lines);

	void loadFilterSettings(const QString&) override;
	void saveFilterSettings(const QString&) const override;

	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setStartColumn(const int);
	int startColumn() const;
	void setEndColumn(const int);
	int endColumn() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<NgspiceRawAsciiFilterPrivate> const d;
	friend class NgspiceRawAsciiFilterPrivate;
};

#endif
