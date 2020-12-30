/***************************************************************************
File		: DatasetHandler.h
Project		: LabPlot
Description	: Processes a dataset's metadata file
--------------------------------------------------------------------
Copyright	: (C) 2019 Kovacs Ferencz (kferike98@gmail.com)

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
#ifndef DATASETHANDLER_H
#define DATASETHANDLER_H

#include <QString>
#include "backend/spreadsheet/Spreadsheet.h"

class QJsonObject;
class AsciiFilter;
class QNetworkAccessManager;
class QNetworkReply;

class DatasetHandler : public QObject{
	Q_OBJECT

public:
	explicit DatasetHandler(Spreadsheet*);
	~DatasetHandler();
	void processMetadata(const QJsonObject&);

private:
	Spreadsheet* m_spreadsheet;
	AsciiFilter* m_filter;
	QJsonObject* m_object{nullptr};
	QNetworkAccessManager* m_downloadManager;
	QNetworkReply* m_currentDownload{nullptr};
	QString m_fileName;
	bool m_invalidMetadataFile{false};
	QString m_containingDir;

	void loadJsonDocument(const QString& path);
	void configureFilter();
	void configureSpreadsheet();
	void prepareForDataset();
	void processDataset();
	void doDownload(const QUrl&);
	bool isHttpRedirect(QNetworkReply*);
	QString saveFileName(const QUrl&);
	bool saveToDisk(const QString& filename, QIODevice*);
	void markMetadataAsInvalid();

private slots:
	void downloadFinished(QNetworkReply*);

signals:
	void downloadCompleted();
	void downloadProgress(int progress);
};

#endif // DATASETHANDLER_H
