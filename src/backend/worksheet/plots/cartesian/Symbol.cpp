/***************************************************************************
    File                 : Symbol.cpp
    Project              : LabPlot
    Description          : Symbol
    --------------------------------------------------------------------
    Copyright            : (C) 2015-2020 Alexander Semke (alexander.semke@web.de)

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

/*!
  \class Symbol
  \brief

  \ingroup worksheet
*/

#include "Symbol.h"
#include <KLocalizedString>
#include <math.h>

int Symbol::stylesCount() {
	return 21;
}
#include <QDebug>
QPainterPath Symbol::pathFromStyle(Symbol::Style style) {
	QPainterPath path;
	QPolygonF polygon;
	switch (style) {
	case Style::NoSymbols:
		break;
	case Style::Circle:
		path.addEllipse(QPoint(0,0), 0.5, 0.5);
		break;
	case Style::Square:
		path.addRect(QRectF(- 0.5, -0.5, 1.0, 1.0));
		break;
	case Style::EquilateralTriangle:
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::RightTriangle:
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Bar:
		path.addRect(QRectF(- 0.5, -0.2, 1.0, 0.4));
		break;
	case Style::PeakedBar:
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.3, -0.2)<<QPointF(0.3, -0.2)<<QPointF(0.5, 0)
				<<QPointF(0.3, 0.2)<<QPointF(-0.3, 0.2)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::SkewedBar:
		polygon<<QPointF(-0.5, 0.2)<<QPointF(-0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.2, 0.2)<<QPointF(-0.5, 0.2);
		path.addPolygon(polygon);
		break;
	case Style::Diamond:
		polygon<<QPointF(-0.5, 0)<<QPointF(0, -0.5)<<QPointF(0.5, 0)<<QPointF(0, 0.5)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Lozenge:
		polygon<<QPointF(-0.25, 0)<<QPointF(0, -0.5)<<QPointF(0.25, 0)<<QPointF(0, 0.5)<<QPointF(-0.25, 0);
		path.addPolygon(polygon);
		break;
	case Style::Tie:
		polygon<<QPointF(-0.5, -0.5)<<QPointF(0.5, -0.5)<<QPointF(-0.5, 0.5)<<QPointF(0.5, 0.5)<<QPointF(-0.5, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::TinyTie:
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(-0.2, 0.5)<<QPointF(0.2, 0.5)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Plus:
		polygon<<QPointF(-0.2, -0.5)<<QPointF(0.2, -0.5)<<QPointF(0.2, -0.2)<<QPointF(0.5, -0.2)<<QPointF(0.5, 0.2)
				<<QPointF(0.2, 0.2)<<QPointF(0.2, 0.5)<<QPointF(-0.2, 0.5)<<QPointF(-0.2, 0.2)<<QPointF(-0.5, 0.2)
				<<QPointF(-0.5, -0.2)<<QPointF(-0.2, -0.2)<<QPointF(-0.2, -0.5);
		path.addPolygon(polygon);
		break;
	case Style::Boomerang:
		polygon<<QPointF(-0.5, 0.5)<<QPointF(0, -0.5)<<QPointF(0.5, 0.5)<<QPointF(0, 0)<<QPointF(-0.5, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::SmallBoomerang:
		polygon<<QPointF(-0.3, 0.5)<<QPointF(0, -0.5)<<QPointF(0.3, 0.5)<<QPointF(0, 0)<<QPointF(-0.3, 0.5);
		path.addPolygon(polygon);
		break;
	case Style::Star4:
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0, 0.5)<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Star5:
		polygon<<QPointF(-0.5, 0)<<QPointF(-0.1, -0.1)<<QPointF(0, -0.5)<<QPointF(0.1, -0.1)<<QPointF(0.5, 0)
				<<QPointF(0.1, 0.1)<<QPointF(0.5, 0.5)<<QPointF(0, 0.2)<<QPointF(-0.5, 0.5)
				<<QPointF(-0.1, 0.1)<<QPointF(-0.5, 0);
		path.addPolygon(polygon);
		break;
	case Style::Line:
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
		break;
	case Style::Cross:
		path = QPainterPath(QPointF(0, -0.5));
		path.lineTo(0, 0.5);
		path.moveTo(-0.5, 0);
		path.lineTo(0.5, 0);
		break;
	case Style::Heart: {
		//https://mathworld.wolfram.com/HeartCurve.html with additional
		//normalization to fit into a 1.0x1.0 rectangular
		int steps = 100;
		double range = 2*M_PI/(steps - 1);
		for (int i = 0; i < steps; ++i) {
			double t = i*range + M_PI/2;
			double x = pow(sin(t), 3);
			double y = -(13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))/17;
			polygon << QPointF(x/2, y/2);
		}
		double t = M_PI/2;
		double x = pow(sin(t), 3);
		double y = -(13*cos(t) - 5*cos(2*t) - 2*cos(3*t) - cos(4*t))/17;
		polygon << QPointF(x/2, y/2);
		path.addPolygon(polygon);
		break;
	}
	case Style::Lightning:
		polygon << QPointF(0, 0.5)
			<< QPointF(0.4, -0.03)
			<< QPointF(0, -0.03)
			<< QPointF(0.2, -0.5)
			<< QPointF(-0.4, 0.1)
			<< QPointF(0.06, 0.1)
			<< QPointF(0, 0.5);
		path.addPolygon(polygon);
		break;
	}

	return path;
}

QString Symbol::nameFromStyle(Symbol::Style style) {
	QString name;
	switch (style) {
	case Style::NoSymbols:
		name = i18n("none");
		break;
	case Style::Circle:
		name = i18n("circle");
		break;
	case Style::Square:
		name = i18n("square");
		break;
	case Style::EquilateralTriangle:
		name = i18n("equilateral triangle");
		break;
	case Style::RightTriangle:
		name = i18n("right triangle");
		break;
	case Style::Bar:
		name = i18n("bar");
		break;
	case Style::PeakedBar:
		name = i18n("peaked bar");
		break;
	case Style::SkewedBar:
		name = i18n("skewed bar");
		break;
	case Style::Diamond:
		name = i18n("diamond");
		break;
	case Style::Lozenge:
		name = i18n("lozenge");
		break;
	case Style::Tie:
		name = i18n("tie");
		break;
	case Style::TinyTie:
		name = i18n("tiny tie");
		break;
	case Style::Plus:
		name = i18n("plus");
		break;
	case Style::Boomerang:
		name = i18n("boomerang");
		break;
	case Style::SmallBoomerang:
		name = i18n("small boomerang");
		break;
	case Style::Star4:
		name = i18n("star4");
		break;
	case Style::Star5:
		name = i18n("star5");
		break;
	case Style::Line:
		name = i18n("line");
		break;
	case Style::Cross:
		name = i18n("cross");
		break;
	case Style::Heart:
		name = i18n("heart");
		break;
	case Style::Lightning:
		name = i18n("lightning");
		break;
	}

	return name;
}
