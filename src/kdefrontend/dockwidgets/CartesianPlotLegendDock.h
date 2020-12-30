/***************************************************************************
    File                 : CartesianPlotLegendDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2013-2020 Alexander Semke (alexander.semke@web.de)
    Description          : widget for cartesian legend properties

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

#ifndef CARTESIANPLOTLEGENDDOCK_H
#define CARTESIANPLOTLEGENDDOCK_H

#include "ui_cartesianplotlegenddock.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <QList>
#include <KConfig>

class LabelWidget;

class CartesianPlotLegendDock : public BaseDock {
	Q_OBJECT

public:
	explicit CartesianPlotLegendDock(QWidget*);

	void setLegends(QList<CartesianPlotLegend*>);
	void activateTitleTab() const;
	void updateLocale() override;
	void updateUnits() override;

private:
	Ui::CartesianPlotLegendDock ui;
	QList<CartesianPlotLegend*> m_legendList;
	CartesianPlotLegend* m_legend{nullptr};
	LabelWidget* labelWidget{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	void init();
	void retranslateUi();

	//SLOTs for changes triggered in CartesianPlotLegendDock
	//"General"-tab
	void visibilityChanged(bool);
	void labelFontChanged(const QFont&);
	void labelColorChanged(const QColor&);
	void labelOrderChanged(int);
	void lineSymbolWidthChanged(double);
	void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void rotationChanged(int value);

	//"Background"-tab
  	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundBrushStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void backgroundOpacityChanged(int);
  	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderCornerRadiusChanged(double);
	void borderOpacityChanged(int);

	//"Layout"-tab
	void layoutTopMarginChanged(double);
	void layoutBottomMarginChanged(double);
	void layoutRightMarginChanged(double);
	void layoutLeftMarginChanged(double);
	void layoutHorizontalSpacingChanged(double);
	void layoutVerticalSpacingChanged(double);
	void layoutColumnCountChanged(int);

	//SLOTs for changes triggered in CartesianPlotLegend
	void legendDescriptionChanged(const AbstractAspect*);
	void legendLabelFontChanged(QFont&);
	void legendLabelColorChanged(QColor&);
	void legendLabelOrderChanged(bool);
	void legendLineSymbolWidthChanged(float);
	void legendPositionChanged(const CartesianPlotLegend::PositionWrapper&);
	void legendRotationAngleChanged(qreal);
	void legendVisibilityChanged(bool);

	void legendBackgroundTypeChanged(PlotArea::BackgroundType);
	void legendBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void legendBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void legendBackgroundBrushStyleChanged(Qt::BrushStyle);
	void legendBackgroundFirstColorChanged(QColor&);
	void legendBackgroundSecondColorChanged(QColor&);
	void legendBackgroundFileNameChanged(QString&);
	void legendBackgroundOpacityChanged(float);

	void legendBorderPenChanged(QPen&);
	void legendBorderCornerRadiusChanged(float);
	void legendBorderOpacityChanged(float);

	void legendLayoutTopMarginChanged(float);
	void legendLayoutBottomMarginChanged(float);
	void legendLayoutLeftMarginChanged(float);
	void legendLayoutRightMarginChanged(float);
	void legendLayoutVerticalSpacingChanged(float);
	void legendLayoutHorizontalSpacingChanged(float);
	void legendLayoutColumnCountChanged(int);

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
