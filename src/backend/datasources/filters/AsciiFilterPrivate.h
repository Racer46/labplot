/***************************************************************************
    File                 : AsciiFilterPrivate.h
    Project              : LabPlot
    Description          : Private implementation class for AsciiFilter.
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef ASCIIFILTERPRIVATE_H
#define ASCIIFILTERPRIVATE_H

class KFilterDev;
class AbstractDataSource;
class AbstractColumn;
class AbstractAspect;
class Spreadsheet;
class MQTTTopic;

class AsciiFilterPrivate {

public:
	explicit AsciiFilterPrivate(AsciiFilter*);

	int isPrepared();
	QString separator() const;

	//preview
	QVector<QStringList> preview(const QString& fileName, int lines);
	QVector<QStringList> preview(QIODevice&);

	//read
	void readDataFromDevice(QIODevice&, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void readFromLiveDeviceNotFile(QIODevice&, AbstractDataSource*,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	qint64 readFromLiveDevice(QIODevice&, AbstractDataSource*, qint64 from = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);

	//write
	void write(const QString& fileName, AbstractDataSource*);

	//helpers
	int prepareDeviceToRead(QIODevice&);
	void initDataContainers(Spreadsheet*);
	QString previewValue(const QString&, AbstractColumn::ColumnMode);
	void setValue(int col, int row, const QString& value);
	QStringList getLineString(QIODevice&);

#ifdef HAVE_MQTT
	int prepareToRead(const QString&);
	QVector<QStringList> preview(const QString& message);
	AbstractColumn::ColumnMode MQTTColumnMode() const;
	QString MQTTColumnStatistics(const MQTTTopic*) const;
	void readMQTTTopic(const QString& message, AbstractDataSource*);
	void setPreparedForMQTT(bool, MQTTTopic*, const QString&);
#endif

	const AsciiFilter* q;

	QString commentCharacter{'#'};
	QString separatingCharacter{QStringLiteral("auto")};
	QString dateTimeFormat;
	QLocale::Language numberFormat{QLocale::C};
	QLocale locale{QLocale::C};
	bool autoModeEnabled{true};
	bool headerEnabled{true};
	bool skipEmptyParts{false};
	bool simplifyWhitespacesEnabled{false};
	double nanValue{NAN};
	bool removeQuotesEnabled{false};
	bool createIndexEnabled{false};
	bool createTimestampEnabled{false};
	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};
	int mqttPreviewFirstEmptyColCount{0};

	//TODO: redesign and remove this later
	bool readingFile{false};
	QString readingFileName;

private:
	static const unsigned int m_dataTypeLines = 10;	// maximum lines to read for determining data types
	QString m_separator;
	int m_actualStartRow{1};
	int m_actualRows{0};
	int m_actualCols{0};
	int m_prepared{false};
	int m_columnOffset{0}; // indexes the "start column" in the datasource. Data will be imported starting from this column.
	std::vector<void*> m_dataContainer; // pointers to the actual data containers

	QDateTime parseDateTime(const QString& string, const QString& format);
};

#endif
