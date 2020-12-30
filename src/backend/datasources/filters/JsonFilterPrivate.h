/***************************************************************************
    File                 : JsonFilterPrivate.h
    Project              : LabPlot
    Description          : Private implementation class for JsonFilter.
    --------------------------------------------------------------------
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Andrey Cygankov (craftplace.ms@gmail.com)
    Copyright            : (C) 2018-2020 by Alexander Semke (alexander.semke@web.de)

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

#ifndef JSONFILTERPRIVATE_H
#define JSONFILTERPRIVATE_H

#include "QJsonModel.h"

class QJsonDocument;
class AbstractDataSource;
class AbstractColumn;

class JsonFilterPrivate {

public:
	explicit JsonFilterPrivate (JsonFilter* owner);

	int checkRow(QJsonValueRef value, int& countCols);
	int parseColumnModes(const QJsonValue& row, const QString& rowName = QString());
	void setEmptyValue(int column, int row);
	void setValueFromString(int column, int row, const QString& value);

	int prepareDeviceToRead(QIODevice&);
	void readDataFromDevice(QIODevice&, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void importData(AbstractDataSource* = nullptr, AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace,
	                int lines = -1);

	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice& device, int lines);
	QVector<QStringList> preview(int lines);

	const JsonFilter* q;
	QJsonModel* model{nullptr};

	JsonFilter::DataContainerType containerType{JsonFilter::DataContainerType::Object};
	QJsonValue::Type rowType{QJsonValue::Object};
	QVector<int> modelRows;

	QString dateTimeFormat;
	QLocale::Language numberFormat{QLocale::C};
	double nanValue{NAN};
	bool createIndexEnabled{false};
	bool importObjectNames{false};
	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;

	int startRow{1};	// start row
	int endRow{-1};		// end row
	int startColumn{1};	// start column
	int endColumn{-1};	// end column

private:
	int m_actualRows{0};
	int m_actualCols{0};
	int m_prepared{false};
	int m_columnOffset{0}; // indexes the "start column" in the datasource. Data will be imported starting from this column.
	std::vector<void*> m_dataContainer; // pointers to the actual data containers (columns).
	QJsonDocument m_doc; //original and full JSON document
	QJsonDocument m_preparedDoc; // selected part of the full JSON document, the part that needs to be imported

	bool prepareDocumentToRead();
};

#endif
