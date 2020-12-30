/***************************************************************************
    File                 : ImportFileDialog.h
    Project              : LabPlot
    Description          : import data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2015 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef IMPORTFILEDIALOG_H
#define IMPORTFILEDIALOG_H

#include "ImportDialog.h"
#include "backend/datasources/LiveDataSource.h"

class MainWin;
class ImportFileWidget;

#ifdef HAVE_MQTT
class MQTTClient;
#endif

class QStatusBar;
class KMessageWidget;

class ImportFileDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportFileDialog(MainWin*, bool liveDataSource = false, const QString& fileName = QString());
	~ImportFileDialog() override;

	QString selectedObject() const override;
	LiveDataSource::SourceType sourceType() const;
	void importToLiveDataSource(LiveDataSource*, QStatusBar*) const;
	void importTo(QStatusBar*) const override;
#ifdef HAVE_MQTT
	void importToMQTT(MQTTClient*) const;
#endif

private:
	ImportFileWidget* m_importFileWidget;
	bool m_showOptions{false};
	QPushButton* m_optionsButton;
	KMessageWidget* m_messageWidget{nullptr};

protected slots:
	void checkOkButton() override;

private slots:
	void toggleOptions();
	void checkOnFitsTableToMatrix(const bool enable);
	void showErrorMessage(const QString&);
};

#endif //IMPORTFILEDIALOG_H
