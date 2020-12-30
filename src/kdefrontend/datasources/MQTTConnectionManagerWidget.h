/***************************************************************************
File                 : MQTTConnectionManagerWidget.h
Project              : LabPlot
Description          : widget for managing MQTT connections
--------------------------------------------------------------------
Copyright            : (C) 2018 Ferencz Kovacs (kferike98@gmail.com)
Copyright            : (C) 2018-2019 Alexander Semke (alexander.semke@web.de)

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
#ifndef MQTTCONNECTIONMANAGERWIDGET_H
#define MQTTCONNECTIONMANAGERWIDGET_H

#include "ui_mqttconnectionmanagerwidget.h"

class QMqttClient;
class QTimer;

class MQTTConnectionManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit MQTTConnectionManagerWidget(QWidget*, const QString&);
	~MQTTConnectionManagerWidget() override;

	struct MQTTConnection {
		QString name;
		int port;
		QString hostName;
		bool useAuthentication;
		QString userName;
		QString password;
		bool useID;
		QString clientID;
		bool retain;
	};

	QString connection() const;
	void setCurrentConnection(const QString&);
	void saveConnections();
	bool checkConnections();

private:
	Ui::MQTTConnectionManagerWidget ui;
	QList<MQTTConnection> m_connections;
	MQTTConnection* m_currentConnection = nullptr;
	bool m_initializing{false};
	QString m_configPath;
	QString m_initConnName;
	QMqttClient* m_client{nullptr};
	bool m_testing{false};
	QTimer* m_testTimer{nullptr};

	QString uniqueName();
	void loadConnection();
	void dataChanged();

private slots:
	void testConnection();
	void loadConnections();
	void addConnection();
	void deleteConnection();
	void connectionChanged(int);
	void nameChanged(const QString&);
	void hostChanged(const QString&);
	void portChanged(const QString&);
	void userNameChanged(const QString&);
	void passwordChanged(const QString&);
	void clientIdChanged(const QString&);
	void authenticationChecked(int);
	void idChecked(int);
	void retainChecked(int);
	void onConnect();
	void onDisconnect();
	void testTimeout();

signals:
	void changed();
};

#endif	// MQTTCONNECTIONMANAGERWIDGET_H
