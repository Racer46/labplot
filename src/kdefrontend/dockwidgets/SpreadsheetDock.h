/***************************************************************************
    File                 : SpreadsheetDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
   Copyright            : (C) 2010 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for column properties
                           
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

#ifndef SPREADSHEETDOCK_H
#define SPREADSHEETDOCK_H

#include <QList>
#include "ui_spreadsheetdock.h"
class Spreadsheet;


class SpreadsheetDock: public QWidget{
  Q_OBJECT
	  
  public:
	SpreadsheetDock(QWidget *parent);
	void setSpreadsheets(QList<Spreadsheet*>);
	  
  private:
	Ui::SpreadsheetDock ui;
	QList<Spreadsheet*> m_spreadsheets;
	bool m_initializing;
	  
	void save(const KConfig&);

  private slots:
	void nameChanged();
	void commentChanged();
	void rowCountChanged(int);
	void columnCountChanged(int);
	void commentsShownChanged(int);

	void loadSettings();
	void saveSettings();
	void saveDefaults();
};

#endif // SPREADSHEETDOCK_H
