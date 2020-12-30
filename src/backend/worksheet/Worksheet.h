/***************************************************************************
    File                 : Worksheet.h
    Project              : LabPlot
    Description          : Worksheet (2D visualization) part
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2019 by Alexander Semke (alexander.semke@web.de)
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

#ifndef WORKSHEET_H
#define WORKSHEET_H

#include "backend/core/AbstractPart.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

class QGraphicsItem;
class QGraphicsScene;
class QRectF;

class WorksheetPrivate;
class WorksheetView;
class TreeModel;
class XYCurve;
class CartesianPlot;

class Worksheet : public AbstractPart {
	Q_OBJECT

public:
	explicit Worksheet(const QString& name, bool loading = false);
	~Worksheet() override;

	enum class Unit {Millimeter, Centimeter, Inch, Point};
	enum class Layout {NoLayout, VerticalLayout, HorizontalLayout, GridLayout};
	enum class CartesianPlotActionMode {ApplyActionToSelection, ApplyActionToAll};

	static double convertToSceneUnits(const double value, const Worksheet::Unit unit);
	static double convertFromSceneUnits(const double value, const Worksheet::Unit unit);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;
	QVector<AbstractAspect*> dependsOn() const override;
	QVector<AspectType> pasteTypes() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QRectF pageRect() const;
	void setPageRect(const QRectF&);
	QGraphicsScene* scene() const;
	void update();
	void setPrinting(bool) const;
	void setThemeName(const QString&);

	void setItemSelectedInView(const QGraphicsItem*, const bool);
	void setSelectedInView(const bool);
	void deleteAspectFromGraphicsItem(const QGraphicsItem*);
	void setIsClosing();
	void suppressSelectionChangedEvent(bool);

	CartesianPlotActionMode cartesianPlotActionMode();
	void setCartesianPlotActionMode(CartesianPlotActionMode mode);
	CartesianPlotActionMode cartesianPlotCursorMode();
	void setCartesianPlotCursorMode(CartesianPlotActionMode mode);
	void setPlotsLocked(bool);
	bool plotsLocked();
	int plotCount();
	CartesianPlot* plot(int index);
	TreeModel* cursorModel();

	void cursorModelPlotAdded(const QString& name);
	void cursorModelPlotRemoved(const QString& name);

	BASIC_D_ACCESSOR_DECL(double, backgroundOpacity, BackgroundOpacity)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
	BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)

	BASIC_D_ACCESSOR_DECL(bool, scaleContent, ScaleContent)
	BASIC_D_ACCESSOR_DECL(bool, useViewSize, UseViewSize)
	BASIC_D_ACCESSOR_DECL(Worksheet::Layout, layout, Layout)
	BASIC_D_ACCESSOR_DECL(double, layoutTopMargin, LayoutTopMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutBottomMargin, LayoutBottomMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutLeftMargin, LayoutLeftMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutRightMargin, LayoutRightMargin)
	BASIC_D_ACCESSOR_DECL(double, layoutHorizontalSpacing, LayoutHorizontalSpacing)
	BASIC_D_ACCESSOR_DECL(double, layoutVerticalSpacing, LayoutVerticalSpacing)
	BASIC_D_ACCESSOR_DECL(int, layoutRowCount, LayoutRowCount)
	BASIC_D_ACCESSOR_DECL(int, layoutColumnCount, LayoutColumnCount)

	QString theme() const;

	void setSuppressLayoutUpdate(bool);
	void updateLayout();

	void registerShortcuts() override;
	void unregisterShortcuts() override;

	typedef WorksheetPrivate Private;

public slots:
	void setTheme(const QString&);
	void cartesianPlotMousePressZoomSelectionMode(QPointF logicPos);
	void cartesianPlotMousePressCursorMode(int cursorNumber, QPointF logicPos);
	void cartesianPlotMouseMoveZoomSelectionMode(QPointF logicPos);
	void cartesianPlotMouseMoveCursorMode(int cursorNumber, QPointF logicPos);
	void cartesianPlotMouseReleaseZoomSelectionMode();
	void cartesianPlotMouseHoverZoomSelectionMode(QPointF logicPos);
	void cartesianPlotMouseHoverOutsideDataRect();
	void cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode);

	// slots needed by the cursor
	void updateCurveBackground(const QPen&, const QString& curveName);
	void updateCompleteCursorTreeModel();
	void cursorPosChanged(int cursorNumber, double xPos);
	void curveAdded(const XYCurve* curve);
	void curveRemoved(const XYCurve* curve);
	void curveDataChanged(const XYCurve* curve);

private:
	void init();
	WorksheetElement* aspectFromGraphicsItem(const WorksheetElement*, const QGraphicsItem*) const;
	void loadTheme(const QString&);

	WorksheetPrivate* const d;
	mutable WorksheetView* m_view{nullptr};
	friend class WorksheetPrivate;

private slots:
	void handleAspectAdded(const AbstractAspect*);
	void handleAspectAboutToBeRemoved(const AbstractAspect*);
	void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	void childSelected(const AbstractAspect*) override;
	void childDeselected(const AbstractAspect*) override;

signals:
	void requestProjectContextMenu(QMenu*);
	void itemSelected(QGraphicsItem*);
	void itemDeselected(QGraphicsItem*);
	void requestUpdate();
	void useViewSizeRequested();
	void cartesianPlotMouseModeChanged(CartesianPlot::MouseMode);
	void showCursorDock(TreeModel*, QVector<CartesianPlot*>);
	void propertiesExplorerRequested();

	void backgroundTypeChanged(PlotArea::BackgroundType);
	void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void backgroundBrushStyleChanged(Qt::BrushStyle);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void backgroundFileNameChanged(const QString&);
	void backgroundOpacityChanged(float);
	void scaleContentChanged(bool);
	void pageRectChanged(const QRectF&);
	void layoutChanged(Worksheet::Layout);
	void layoutTopMarginChanged(float);
	void layoutBottomMarginChanged(float);
	void layoutLeftMarginChanged(float);
	void layoutRightMarginChanged(float);
	void layoutVerticalSpacingChanged(float);
	void layoutHorizontalSpacingChanged(float);
	void layoutRowCountChanged(int);
	void layoutColumnCountChanged(int);
	void themeChanged(const QString&);
};

#endif
