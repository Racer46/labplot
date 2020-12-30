/***************************************************************************
    File                 : AbstractAspect.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 by Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007-2010 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2011-2015 by Alexander Semke (alexander.semke@web.de)
    Description          : Base class for all objects in a Project.

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
#ifndef ASPECTFACTORY_H

#include "backend/core/Project.h"
#include "backend/core/Workbook.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/datapicker/DatapickerCurve.h"

class AspectFactory {
public:
	static AbstractAspect* createAspect(AspectType type) {
		if (type == AspectType::Folder)
			return new CartesianPlot(QString());

		/* worksheet and all its children */
		else if (type == AspectType::Worksheet)
			return new Worksheet(QString());
		else if (type == AspectType::CartesianPlot)
			return new CartesianPlot(QString());
		else if (type == AspectType::TextLabel)
			return new TextLabel(QString());
		else if (type == AspectType::Image)
			return new Image(QString());

		/* CartesianPlot's children */
		else if (type == AspectType::Axis)
			return new Axis(QString());
		else if (type == AspectType::XYCurve)
			return new XYCurve(QString());
// 		else if (type == AspectType::CartesianPlotLegend)
// 			return new CartesianPlotLegend(QString());

		/* spreadsheet and its children */
		else if (type == AspectType::Spreadsheet)
			return new Spreadsheet(QString());
		else if (type == AspectType::Column)
			return new Column(QString());

		else if (type == AspectType::Matrix)
			return new Matrix(QString());
		else if (type == AspectType::Workbook)
			return new Workbook(QString());

		return nullptr;
	}
};

#endif
