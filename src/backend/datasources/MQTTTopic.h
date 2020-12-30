/***************************************************************************
File		: MQTTTopic.h
Project		: LabPlot
Description	: Represents a topic of a MQTTSubscription
--------------------------------------------------------------------
Copyright	: (C) 2018 Kovacs Ferencz (kferike98@gmail.com)

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

#ifndef MQTTTOPIC_H
#define MQTTTOPIC_H

#include "backend/spreadsheet/Spreadsheet.h"

class MQTTSubscription;
class MQTTClient;

class AsciiFilter;

class MQTTTopic : public Spreadsheet {
	Q_OBJECT

public:
	MQTTTopic(const QString& name, MQTTSubscription* subscription, bool loading = false);
	~MQTTTopic() override;

	void setFilter(AsciiFilter*);
	AsciiFilter* filter() const;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	QString topicName() const;
	MQTTClient* mqttClient() const;
	void newMessage(const QString&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

private:
	void initActions();

	QString m_topicName;
	MQTTClient* m_MQTTClient;
	AsciiFilter* m_filter;
	QVector<QString> m_messagePuffer;
	QAction* m_plotDataAction;

public slots:
	void read();

private slots:
	void plotData();

signals:
	void readOccured();
};

#endif // MQTTTOPIC_H
