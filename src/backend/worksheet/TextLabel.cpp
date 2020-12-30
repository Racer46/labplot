/***************************************************************************
    File                 : TextLabel.cpp
    Project              : LabPlot
    Description          : Text label supporting reach text and latex formatting
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2019 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "TextLabel.h"
#include "Worksheet.h"
#include "TextLabelPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

#include <QApplication>
#include <QBuffer>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextCursor>
#include <QtConcurrent/QtConcurrentRun>

#include <QIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class TextLabel
 * \brief A label supporting rendering of html- and tex-formatted texts.
 *
 * The label is aligned relative to the specified position.
 * The position can be either specified by providing the x- and y- coordinates
 * in parent's coordinate system, or by specifying one of the predefined position
 * flags (\c HorizontalPosition, \c VerticalPosition).
 */

TextLabel::TextLabel(const QString& name, Type type)
	: WorksheetElement(name, AspectType::TextLabel), d_ptr(new TextLabelPrivate(this)), m_type(type) {

	init();
}

TextLabel::TextLabel(const QString &name, TextLabelPrivate *dd, Type type)
	: WorksheetElement(name, AspectType::TextLabel), d_ptr(dd), m_type(type) {

	init();
}

TextLabel::TextLabel(const QString &name, CartesianPlot* plot, Type type):
	WorksheetElement(name, AspectType::TextLabel),
	d_ptr(new TextLabelPrivate(this,plot)), m_type(type), visibilityAction(nullptr) {
	init();
}

void TextLabel::init() {
	Q_D(TextLabel);

	QString groupName;
	switch (m_type) {
	case Type::General:
		groupName = "TextLabel";
		break;
	case Type::PlotTitle:
		groupName = "PlotTitle";
		break;
	case Type::AxisTitle:
		groupName = "AxisTitle";
		break;
	case Type::PlotLegendTitle:
		groupName = "PlotLegendTitle";
		break;
	case Type::InfoElementLabel:
		groupName = "InfoElementLabel";
	}

	const KConfig config;
	DEBUG("	config has group \"" << STDSTRING(groupName) << "\": " << config.hasGroup(groupName));
	// group is always valid if you call config.group(..;
	KConfigGroup group;
	if (config.hasGroup(groupName))
		group = config.group(groupName);

	// non-default settings
	d->staticText.setTextFormat(Qt::RichText);
	// explicitly set no wrap mode for text label to avoid unnecessary line breaks
	QTextOption textOption;
	textOption.setWrapMode(QTextOption::NoWrap);
	d->staticText.setTextOption(textOption);

	if (m_type == Type::PlotTitle || m_type == Type::PlotLegendTitle) {
		d->position.verticalPosition = WorksheetElement::VerticalPosition::Top;
		d->verticalAlignment = WorksheetElement::VerticalAlignment::Bottom;
	} else if (m_type == Type::AxisTitle) {
		d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
		d->position.verticalPosition = WorksheetElement::VerticalPosition::Custom;
	}

	// read settings from config if group exists
	if (group.isValid()) {
		//properties common to all types
		d->textWrapper.teXUsed = group.readEntry("TeXUsed", d->textWrapper.teXUsed);
		d->teXFont.setFamily(group.readEntry("TeXFontFamily", d->teXFont.family()));
		d->teXFont.setPointSize(group.readEntry("TeXFontSize", d->teXFont.pointSize()));
		d->fontColor = group.readEntry("TeXFontColor", d->fontColor);
		d->backgroundColor = group.readEntry("TeXBackgroundColor", d->backgroundColor);
		d->rotationAngle = group.readEntry("Rotation", d->rotationAngle);

		//border
		d->borderShape = (TextLabel::BorderShape)group.readEntry("BorderShape", (int)d->borderShape);
		d->borderPen = QPen(group.readEntry("BorderColor", d->borderPen.color()),
		                    group.readEntry("BorderWidth", d->borderPen.width()),
		                    (Qt::PenStyle) group.readEntry("BorderStyle", (int)(d->borderPen.style())));
		d->borderOpacity = group.readEntry("BorderOpacity", d->borderOpacity);

		//position and alignment relevant properties
		d->position.point.setX( group.readEntry("PositionXValue", d->position.point.x()) );
		d->position.point.setY( group.readEntry("PositionYValue", d->position.point.y()) );
		d->position.horizontalPosition = (HorizontalPosition) group.readEntry("PositionX", (int)d->position.horizontalPosition);
		d->position.verticalPosition = (VerticalPosition) group.readEntry("PositionY", (int)d->position.verticalPosition);
		d->horizontalAlignment = (WorksheetElement::HorizontalAlignment) group.readEntry("HorizontalAlignment", static_cast<int>(d->horizontalAlignment));
		d->verticalAlignment = (WorksheetElement::VerticalAlignment) group.readEntry("VerticalAlignment", static_cast<int>(d->verticalAlignment));
	}

	DEBUG("CHECK: default/run time image resolution: " << d->teXImageResolution << '/' << QApplication::desktop()->physicalDpiX());

	connect(&d->teXImageFutureWatcher, &QFutureWatcher<QImage>::finished, this, &TextLabel::updateTeXImage);
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
TextLabel::~TextLabel() = default;

QGraphicsItem* TextLabel::graphicsItem() const {
	return d_ptr;
}

/*!
 * \brief TextLabel::setParentGraphicsItem
 * Sets the parent graphicsitem, needed for binding to coord
 * \param item parent graphicsitem
 */
void TextLabel::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(TextLabel);
	d->setParentItem(item);
	d->updatePosition();
}

void TextLabel::retransform() {
	Q_D(TextLabel);
	d->retransform();
}

void TextLabel::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG("TextLabel::handleResize()");
	Q_UNUSED(pageResize);
	Q_D(TextLabel);

	double ratio = 0;
	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
		ratio = qMax(horizontalRatio, verticalRatio);
	else
		ratio = qMin(horizontalRatio, verticalRatio);

	d->teXFont.setPointSizeF(d->teXFont.pointSizeF() * ratio);
	d->updateText();

	//TODO: doesn't seem to work
	QTextDocument doc;
	doc.setHtml(d->textWrapper.text);
	QTextCursor cursor(&doc);
	cursor.select(QTextCursor::Document);
	QTextCharFormat fmt = cursor.charFormat();
	QFont font = fmt.font();
	font.setPointSizeF(font.pointSizeF() * ratio);
	fmt.setFont(font);
	cursor.setCharFormat(fmt);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon TextLabel::icon() const {
	return  QIcon::fromTheme("draw-text");
}

QMenu* TextLabel::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"

	if (!visibilityAction) {
		visibilityAction = new QAction(i18n("Visible"), this);
		visibilityAction->setCheckable(true);
		connect(visibilityAction, &QAction::triggered, this, &TextLabel::visibilityChanged);
	}

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(TextLabel, TextLabel::TextWrapper, text, textWrapper)
CLASS_SHARED_D_READER_IMPL(TextLabel, QColor, fontColor, fontColor);
CLASS_SHARED_D_READER_IMPL(TextLabel, QColor, backgroundColor, backgroundColor);
CLASS_SHARED_D_READER_IMPL(TextLabel, QFont, teXFont, teXFont);
CLASS_SHARED_D_READER_IMPL(TextLabel, TextLabel::PositionWrapper, position, position);
CLASS_SHARED_D_READER_IMPL(TextLabel, QPointF, positionLogical, positionLogical);
BASIC_SHARED_D_READER_IMPL(TextLabel, WorksheetElement::HorizontalAlignment, horizontalAlignment, horizontalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, WorksheetElement::VerticalAlignment, verticalAlignment, verticalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, rotationAngle, rotationAngle);

BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::BorderShape, borderShape, borderShape)
CLASS_SHARED_D_READER_IMPL(TextLabel, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, borderOpacity, borderOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(TextLabel, SetText, TextLabel::TextWrapper, textWrapper, updateText);
void TextLabel::setText(const TextWrapper &textWrapper) {
	Q_D(TextLabel);
	if ( (textWrapper.text != d->textWrapper.text) || (textWrapper.teXUsed != d->textWrapper.teXUsed)
	        || ((d->textWrapper.allowPlaceholder || textWrapper.allowPlaceholder) && (textWrapper.textPlaceholder != d->textWrapper.textPlaceholder)) ||
	        textWrapper.allowPlaceholder != d->textWrapper.allowPlaceholder)
		exec(new TextLabelSetTextCmd(d, textWrapper, ki18n("%1: set label text")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetPlaceholderText, TextLabel::TextWrapper, textWrapper, updateText);
void TextLabel::setPlaceholderText(const TextWrapper &textWrapper) {
	Q_D(TextLabel);
	if ( (textWrapper.textPlaceholder != d->textWrapper.textPlaceholder) || (textWrapper.teXUsed != d->textWrapper.teXUsed) )
		exec(new TextLabelSetPlaceholderTextCmd(d, textWrapper, ki18n("%1: set label placeholdertext")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXFont, QFont, teXFont, updateText);
void TextLabel::setTeXFont(const QFont& font) {
	Q_D(TextLabel);
	if (font != d->teXFont)
		exec(new TextLabelSetTeXFontCmd(d, font, ki18n("%1: set TeX main font")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXFontColor, QColor, fontColor, updateText);
void TextLabel::setFontColor(const QColor color) {
	Q_D(TextLabel);
	if (color != d->fontColor)
		exec(new TextLabelSetTeXFontColorCmd(d, color, ki18n("%1: set font color")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXBackgroundColor, QColor, backgroundColor, updateText);
void TextLabel::setBackgroundColor(const QColor color) {
	Q_D(TextLabel);
	if (color != d->backgroundColor)
		exec(new TextLabelSetTeXBackgroundColorCmd(d, color, ki18n("%1: set background color")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetPosition, TextLabel::PositionWrapper, position, retransform);
void TextLabel::setPosition(const PositionWrapper& pos) {
	Q_D(TextLabel);
	if (pos.point != d->position.point || pos.horizontalPosition != d->position.horizontalPosition || pos.verticalPosition != d->position.verticalPosition)
		exec(new TextLabelSetPositionCmd(d, pos, ki18n("%1: set position")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetPositionLogical, QPointF, positionLogical, retransform);
void TextLabel::setPositionLogical(QPointF pos) {
	Q_D(TextLabel);
	if (pos != d->positionLogical)
		exec(new TextLabelSetPositionLogicalCmd(d, pos, ki18n("%1: set logical position")));
}

/*!
 * \brief TextLabel::setPosition
 * sets the position without undo/redo-stuff
 * \param point point in scene coordinates
 */
void TextLabel::setPosition(QPointF point) {
	Q_D(TextLabel);
	if (point != d->position.point) {
		d->position.point = point;
		retransform();
	}
}

/*!
 * position is set to invalid if the parent item is not drawn on the scene
 * (e.g. axis is not drawn because it's outside plot ranges -> don't draw axis' title label)
 */
void TextLabel::setPositionInvalid(bool invalid) {
	Q_D(TextLabel);
	if (invalid != d->positionInvalid)
		d->positionInvalid = invalid;
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetRotationAngle, qreal, rotationAngle, recalcShapeAndBoundingRect);
void TextLabel::setRotationAngle(qreal angle) {
	Q_D(TextLabel);
	if (angle != d->rotationAngle)
		exec(new TextLabelSetRotationAngleCmd(d, angle, ki18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetHorizontalAlignment, TextLabel::HorizontalAlignment, horizontalAlignment, retransform);
void TextLabel::setHorizontalAlignment(const WorksheetElement::HorizontalAlignment hAlign) {
	Q_D(TextLabel);
	if (hAlign != d->horizontalAlignment)
		exec(new TextLabelSetHorizontalAlignmentCmd(d, hAlign, ki18n("%1: set horizontal alignment")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetVerticalAlignment, WorksheetElement::VerticalAlignment, verticalAlignment, retransform);
void TextLabel::setVerticalAlignment(const TextLabel::VerticalAlignment vAlign) {
	Q_D(TextLabel);
	if (vAlign != d->verticalAlignment)
		exec(new TextLabelSetVerticalAlignmentCmd(d, vAlign, ki18n("%1: set vertical alignment")));
}

//Border
STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderShape, TextLabel::BorderShape, borderShape, updateBorder)
void TextLabel::setBorderShape(TextLabel::BorderShape shape) {
	Q_D(TextLabel);
	if (shape != d->borderShape)
		exec(new TextLabelSetBorderShapeCmd(d, shape, ki18n("%1: set border shape")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderPen, QPen, borderPen, update)
void TextLabel::setBorderPen(const QPen &pen) {
	Q_D(TextLabel);
	if (pen != d->borderPen)
		exec(new TextLabelSetBorderPenCmd(d, pen, ki18n("%1: set border")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderOpacity, qreal, borderOpacity, update)
void TextLabel::setBorderOpacity(qreal opacity) {
	Q_D(TextLabel);
	if (opacity != d->borderOpacity)
		exec(new TextLabelSetBorderOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

//misc
STD_SWAP_METHOD_SETTER_CMD_IMPL_F(TextLabel, SetVisible, bool, swapVisible, retransform);
void TextLabel::setVisible(bool on) {
	Q_D(TextLabel);
	exec(new TextLabelSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

/*!
 * \brief TextLabel::setCoordBinding
 * Binds TextLabel to the coord of the parent CartesianPlot, but only if coord binding is enabled
 * \param on set(true) or reset(false) coord binding
 */
void TextLabel::setCoordBinding(bool on) {
	Q_D(TextLabel);
	if (!d->m_coordBindingEnable)
		return;
	d->m_coordBinding = on;
	d->positionLogical = d->cSystem->mapSceneToLogical(d->pos(), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	emit positionLogicalChanged(d->positionLogical);
}

/*!
 * \brief TextLabel::enableCoordBinding
 * \param enable enable or disable coord binding
 * \param plot parent plot to which should be bind, if no parentAspect added. If parentAspect exist, this plot will be used.
 * If argument CartesianPlot and parentAspect is not available, not able to enable bind too coord.
 * \return true if successfully bind, otherwise false
 */
bool TextLabel::enableCoordBinding(bool enable, const CartesianPlot* plot) {
	Q_D(TextLabel);

	if (!d->plot) // if infoelement is parent, it has no cartesianplot so d->plot stays nullptr because of the dynamic cast
		d->plot = dynamic_cast<const CartesianPlot*>(parentAspect());

	if (d->plot == nullptr) {
		if (plot == nullptr) {
			d->m_coordBindingEnable = false;
			return false;
		} else {
			d->m_coordBindingEnable = enable;
			d->plot = plot;
			d->cSystem = dynamic_cast<const CartesianCoordinateSystem*>(d->plot->coordinateSystem());
			return true;
		}
	}
	d->m_coordBindingEnable = enable;
	d->cSystem = dynamic_cast<const CartesianCoordinateSystem*>(d->plot->coordinateSystem());
	return true;
}

QRectF TextLabel::size() {
	Q_D(TextLabel);
	return d->size();
}

QPointF TextLabel::findNearestGluePoint(QPointF scenePoint) {
	Q_D(TextLabel);
	return d->findNearestGluePoint(scenePoint);
}

TextLabel::GluePoint TextLabel::gluePointAt(int index) {
	Q_D(TextLabel);
	return d->gluePointAt(index);
}

int TextLabel::gluePointCount() {
	Q_D(const TextLabel);
	return d->m_gluePoints.length();
}

bool TextLabel::isVisible() const {
	Q_D(const TextLabel);
	return d->isVisible();
}

/*!
 * \brief TextLabel::isAttachedToCoordEnabled
 * \return true if bind to coord is enabled
 */
bool TextLabel::isAttachedToCoordEnabled() const {
	Q_D(const TextLabel);
	return d->m_coordBindingEnable;
}

/*!
 * \brief TextLabel::isAttachedToCoord
 * \return true if TextLabel is attached to the coord.
 */
bool TextLabel::isAttachedToCoord() const {
	Q_D(const TextLabel);
	return d->m_coordBinding;
}

void TextLabel::setPrinting(bool on) {
	Q_D(TextLabel);
	d->m_printing = on;
}

void TextLabel::updateTeXImage() {
	Q_D(TextLabel);
	d->updateTeXImage();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void TextLabel::visibilityChanged() {
	Q_D(const TextLabel);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
TextLabelPrivate::TextLabelPrivate(TextLabel* owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

TextLabelPrivate::TextLabelPrivate(TextLabel* owner, CartesianPlot* plot)
	: plot(plot), q(owner) {

	if(plot)
		cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem());

	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setAcceptHoverEvents(true);
}

QString TextLabelPrivate::name() const {
	return q->name();
}

/*!
 * \brief TextLabelPrivate::size
 * \return Size and position of the TextLabel in scene coords
 */
QRectF TextLabelPrivate::size() {
	float w, h;
	if (textWrapper.teXUsed) {
		//image size is in pixel, convert to scene units
		w = teXImage.width()*teXImageScaleFactor;
		h = teXImage.height()*teXImageScaleFactor;
	} else {
		//size is in points, convert to scene units
		w = staticText.size().width()*scaleFactor;
		h = staticText.size().height()*scaleFactor;
	}
	float x = position.point.x();
	float y = position.point.y();
	return QRectF(x,y,w,h);
}

/*!
 * \brief TextLabelPrivate::findNearestGluePoint
 * Finds the glue point, which is nearest to @param point in scene coords.
 * \param point reference position
 * \return Nearest point to @param point
 */
QPointF TextLabelPrivate::findNearestGluePoint(QPointF scenePoint) {
	if (m_gluePoints.isEmpty())
		return boundingRectangle.center();

	if (m_gluePoints.length() == 1)
		return mapParentToPlotArea(mapToParent(m_gluePoints.at(0).point));

	QPointF point = mapParentToPlotArea(mapToParent(m_gluePoints.at(0).point));
	QPointF nearestPoint = point;
	double distance2 = pow(point.x()-scenePoint.x(), 2) + pow(point.y()-scenePoint.y(), 2);
	// assumption, more than one point available
	for (int i = 1; i < m_gluePoints.length(); i++) {
		point = mapParentToPlotArea(mapToParent(m_gluePoints.at(i).point));
		double distance2_temp = pow(point.x()-scenePoint.x(), 2) + pow(point.y()-scenePoint.y(), 2);
		if (distance2_temp < distance2) {
			nearestPoint = point;
			distance2 = distance2_temp;
		}
	}

	return nearestPoint;
}

/*!
 * \brief TextLabelPrivate::gluePointAt
 * Returns the gluePoint at a specific index.
 * \param index
 * \return Returns gluepoint at index \param index
 */
TextLabel::GluePoint TextLabelPrivate::gluePointAt(int index) {
	QPointF pos;
	QString name;

	if (m_gluePoints.isEmpty() || index > m_gluePoints.length()) {
		pos = boundingRectangle.center();
		name = "center";
	} else if (index < 0) {
		pos = m_gluePoints.at(0).point;
		name = m_gluePoints.at(0).name;
	} else {
		pos = m_gluePoints.at(index).point;
		name = m_gluePoints.at(index).name;
	}

	return TextLabel::GluePoint(mapParentToPlotArea(mapToParent(pos)), name);
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void TextLabelPrivate::retransform() {
	if (suppressRetransform)
		return;

/*
	double x, y;
	if(m_coordBindingEnable && m_coordBinding) {
		//the position in logical coordinates was changed, calculate the position in scene coordinates
		position.point = cSystem->mapLogicalToScene(positionLogical, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

		x = positionLogical.x();
		y = positionLogical.y();
	} else {
		//the position in scene coordinates was changed, calculate the position in logical coordinates
		positionLogical = cSystem->mapSceneToLogical(position.point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

		x = position.point.x();
		y = position.point.y();
	}
*/

	if (position.horizontalPosition != WorksheetElement::HorizontalPosition::Custom
	        || position.verticalPosition != WorksheetElement::VerticalPosition::Custom)
		updatePosition();

	double x = position.point.x();
	double y = position.point.y();

	//determine the size of the label in scene units.
	double w, h;
	if (textWrapper.teXUsed) {
		//image size is in pixel, convert to scene units
		w = teXImage.width()*teXImageScaleFactor;
		h = teXImage.height()*teXImageScaleFactor;
	} else {
		//size is in points, convert to scene units
		w = staticText.size().width()*scaleFactor;
		h = staticText.size().height()*scaleFactor;
	}

	QPointF itemPos;
	//depending on the alignment, calculate the new GraphicsItem's position in parent's coordinate system
	switch (horizontalAlignment) {
	case WorksheetElement::HorizontalAlignment::Left:
		itemPos.setX(x - w/2);
		break;
	case WorksheetElement::HorizontalAlignment::Center:
		itemPos.setX(x);
		break;
	case WorksheetElement::HorizontalAlignment::Right:
		itemPos.setX(x + w/2);
		break;
	}

	switch (verticalAlignment) {
	case WorksheetElement::VerticalAlignment::Top:
		itemPos.setY(y - h/2);
		break;
	case WorksheetElement::VerticalAlignment::Center:
		itemPos.setY(y);
		break;
	case WorksheetElement::VerticalAlignment::Bottom:
		itemPos.setY(y + h/2);
		break;
	}

	itemPos = mapPlotAreaToParent(itemPos);
	suppressItemChangeEvent = true;
	setPos(itemPos);
	suppressItemChangeEvent = false;

	boundingRectangle.setX(-w/2);
	boundingRectangle.setY(-h/2);
	boundingRectangle.setWidth(w);
	boundingRectangle.setHeight(h);

	updateBorder();

	emit q->changed();
}

/*!
	calculates the position of the label, when the position relative to the parent was specified (left, right, etc.)
*/
void TextLabelPrivate::updatePosition() {
	//determine the parent item
	QRectF parentRect;
	QGraphicsItem* parent = parentItem();
	if (parent)
		parentRect = parent->boundingRect();
	else {
		if (!scene())
			return;

		parentRect = scene()->sceneRect();
	}

	if (position.horizontalPosition != WorksheetElement::HorizontalPosition::Custom) {
		if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Left)
			position.point.setX( parentRect.x() );
		else if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Center)
			position.point.setX( parentRect.x() + parentRect.width()/2 );
		else if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Right)
			position.point.setX( parentRect.x() + parentRect.width() );
	}

	if (position.verticalPosition != WorksheetElement::VerticalPosition::Custom) {
		if (position.verticalPosition == WorksheetElement::VerticalPosition::Top)
			position.point.setY( parentRect.y() );
		else if (position.verticalPosition == WorksheetElement::VerticalPosition::Center)
			position.point.setY( parentRect.y() + parentRect.height()/2 );
		else if (position.verticalPosition == WorksheetElement::VerticalPosition::Bottom)
			position.point.setY( parentRect.y() + parentRect.height() );
	}

	if(m_coordBindingEnable && m_coordBinding)
		emit q->positionLogicalChanged(positionLogical);
	else
		emit q->positionChanged(position);
}

/*!
	updates the static text.
 */
void TextLabelPrivate::updateText() {
	if (suppressRetransform)
		return;

	if (textWrapper.teXUsed) {
		TeXRenderer::Formatting format;
		format.fontColor = fontColor;
		format.backgroundColor = backgroundColor;
		format.fontSize = teXFont.pointSize();
		format.fontFamily = teXFont.family();
		format.dpi = teXImageResolution;
		QFuture<QImage> future = QtConcurrent::run(TeXRenderer::renderImageLaTeX, textWrapper.text, &teXRenderSuccessful, format);
		teXImageFutureWatcher.setFuture(future);

		//don't need to call retransorm() here since it is done in updateTeXImage
		//when the asynchronous rendering of the image is finished.
	} else {
		staticText.setText(textWrapper.text);

		//the size of the label was most probably changed.
		//call retransform() to recalculate the position and the bounding box of the label
		retransform();
	}
}

void TextLabelPrivate::updateTeXImage() {
	teXImage = teXImageFutureWatcher.result();
	retransform();
	DEBUG("teXRenderSuccessful =" << teXRenderSuccessful);
	emit q->teXImageUpdated(teXRenderSuccessful);
}

void TextLabelPrivate::updateBorder() {
	typedef TextLabel::GluePoint GluePoint;

	borderShapePath = QPainterPath();
	switch (borderShape) {
	case (TextLabel::BorderShape::NoBorder): {
			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x()+boundingRectangle.width()/2, boundingRectangle.y()), "top"));
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x()+boundingRectangle.width(), boundingRectangle.y()+boundingRectangle.height()/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x()+boundingRectangle.width()/2, boundingRectangle.y()+boundingRectangle.height()), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x(), boundingRectangle.y()+boundingRectangle.height()/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::Rect): {
			borderShapePath.addRect(boundingRectangle);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width()/2, boundingRectangle.y()), "top"));
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width(), boundingRectangle.y() + boundingRectangle.height()/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width()/2, boundingRectangle.y() + boundingRectangle.height()), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x(), boundingRectangle.y() + boundingRectangle.height()/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::Ellipse): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			const QRectF ellipseRect(xs  - 0.1 * w, ys - 0.1 * h, 1.2 * w,  1.2 * h);
			borderShapePath.addEllipse(ellipseRect);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width()/2, ellipseRect.y()), "top"));
			m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width(), ellipseRect.y() + ellipseRect.height()/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width()/2, ellipseRect.y() + ellipseRect.height()), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(ellipseRect.x(), ellipseRect.y() + ellipseRect.height()/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::RoundSideRect): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs, ys);
			borderShapePath.lineTo(xs + w, ys);
			borderShapePath.quadTo(xs + w + h/2, ys + h/2, xs + w, ys + h);
			borderShapePath.lineTo(xs, ys + h);
			borderShapePath.quadTo(xs - h/2, ys + h/2, xs, ys);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w + h/2, ys + h/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs - h/2, ys + h/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::RoundCornerRect): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs + h * 0.2, ys);
			borderShapePath.lineTo(xs + w - h * 0.2, ys);
			borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
			borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
			borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + 0.2 * h, ys + h);
			borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
			borderShapePath.lineTo(xs, ys + 0.2 * h);
			borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs+w/2,ys), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs+w,ys+h/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs+w/2,ys+h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs,ys+h/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::InwardsRoundCornerRect): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs, ys - 0.3 * h);
			borderShapePath.lineTo(xs + w, ys - 0.3 * h);
			borderShapePath.quadTo(xs + w, ys, xs + w + 0.3 * h, ys);
			borderShapePath.lineTo(xs + w + 0.3 * h, ys + h);
			borderShapePath.quadTo(xs + w, ys + h, xs + w, ys + h + 0.3 * h);
			borderShapePath.lineTo(xs, ys + h + 0.3 * h);
			borderShapePath.quadTo(xs, ys + h, xs - 0.3 * h, ys + h);
			borderShapePath.lineTo(xs - 0.3 * h, ys);
			borderShapePath.quadTo(xs, ys, xs, ys - 0.3 * h);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.3 * h), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w + 0.3 * h, ys + h / 2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.3 * h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs - 0.3 * h, ys + h / 2), "left"));
			break;
		}
	case (TextLabel::BorderShape::DentedBorderRect): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs - 0.2 * h, ys - 0.2 * h);
			borderShapePath.quadTo(xs + w / 2, ys, xs + w + 0.2 * h, ys - 0.2 * h);
			borderShapePath.quadTo(xs + w, ys + h / 2, xs + w + 0.2 * h, ys + h + 0.2 * h);
			borderShapePath.quadTo(xs + w / 2, ys + h, xs - 0.2 * h, ys + h + 0.2 * h);
			borderShapePath.quadTo(xs, ys + h / 2, xs - 0.2 * h, ys - 0.2 * h);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.2 * h), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w + 0.2 * h, ys + h / 2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.2 * h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs - 0.2 * h, ys + h / 2), "left"));
			break;
		}
	case (TextLabel::BorderShape::Cuboid): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs, ys);
			borderShapePath.lineTo(xs + w, ys);
			borderShapePath.lineTo(xs + w, ys + h);
			borderShapePath.lineTo(xs, ys + h);
			borderShapePath.lineTo(xs, ys);
			borderShapePath.lineTo(xs + 0.3 * h, ys - 0.2 * h);
			borderShapePath.lineTo(xs + w + 0.3 * h, ys - 0.2 * h);
			borderShapePath.lineTo(xs + w, ys);
			borderShapePath.moveTo(xs + w, ys + h);
			borderShapePath.lineTo(xs + w + 0.3 * h, ys + h - 0.2 * h);
			borderShapePath.lineTo(xs + w + 0.3 * h, ys - 0.2 * h);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.1 * h), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w + 0.15 * h, ys + h / 2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), "left"));
			break;
		}
	case (TextLabel::BorderShape::UpPointingRectangle): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs + h * 0.2, ys);
			borderShapePath.lineTo(xs + w / 2 - 0.2 * h, ys);
			borderShapePath.lineTo(xs + w / 2, ys - 0.2 * h);
			borderShapePath.lineTo(xs + w / 2 + 0.2 * h, ys);
			borderShapePath.lineTo(xs + w - h * 0.2, ys);
			borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
			borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
			borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + 0.2 * h, ys + h);
			borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
			borderShapePath.lineTo(xs, ys + 0.2 * h);
			borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys - h * 0.2), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs, ys + h/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::DownPointingRectangle): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs +h * 0.2, ys);
			borderShapePath.lineTo(xs + w - h * 0.2, ys);
			borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
			borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
			borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + w / 2 + 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + w / 2, ys + h + 0.2 * h);
			borderShapePath.lineTo(xs + w / 2 - 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + 0.2 * h, ys + h);
			borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
			borderShapePath.lineTo(xs, ys + 0.2 * h);
			borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h  + 0.2 * h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs, ys + h/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::LeftPointingRectangle): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs + h*0.2, ys);
			borderShapePath.lineTo(xs + w - h * 0.2, ys);
			borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
			borderShapePath.lineTo(xs + w,  ys + h - 0.2 * h);
			borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + 0.2 * h, ys + h);
			borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
			borderShapePath.lineTo(xs, ys + h / 2 + 0.2 * h);
			borderShapePath.lineTo(xs - 0.2 * h, ys + h / 2);
			borderShapePath.lineTo(xs, ys + h / 2 - 0.2 * h);
			borderShapePath.lineTo(xs, ys + 0.2 * h);
			borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs - 0.2 * h, ys + h/2), "left"));
			break;
		}
	case (TextLabel::BorderShape::RightPointingRectangle): {
			const double xs = boundingRectangle.x();
			const double ys = boundingRectangle.y();
			const double w = boundingRectangle.width();
			const double h = boundingRectangle.height();
			borderShapePath.moveTo(xs + h * 0.2, ys);
			borderShapePath.lineTo(xs + w - h * 0.2, ys);
			borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
			borderShapePath.lineTo(xs + w, ys + h / 2 - 0.2 * h);
			borderShapePath.lineTo(xs + w + 0.2 * h, ys + h / 2);
			borderShapePath.lineTo(xs + w, ys + h / 2 + 0.2 * h);
			borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
			borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
			borderShapePath.lineTo(xs + 0.2 * h, ys + h);
			borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
			borderShapePath.lineTo(xs, ys + 0.2 * h);
			borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

			m_gluePoints.clear();
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
			m_gluePoints.append(GluePoint(QPointF(xs + w + 0.2 * h, ys + h/2), "right"));
			m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
			m_gluePoints.append(GluePoint(QPointF(xs, ys + h/2), "left"));
			break;
		}
	}

	recalcShapeAndBoundingRect();
}

bool TextLabelPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();

	//When making a graphics item invisible, it gets deselected in the scene.
	//In this case we don't want to deselect the item in the project explorer.
	//We need to supress the deselection in the view.
	auto* worksheet = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));
	if (worksheet) {
		worksheet->suppressSelectionChangedEvent(true);
		setVisible(on);
		worksheet->suppressSelectionChangedEvent(false);
	} else
		setVisible(on);

	emit q->changed();
	emit q->visibleChanged(on);
	return oldValue;
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF TextLabelPrivate::boundingRect() const {
	return transformedBoundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath TextLabelPrivate::shape() const {
	return labelShape;
}

/*!
  recalculates the outer bounds and the shape of the label.
*/
void TextLabelPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QMatrix matrix;
	matrix.rotate(-rotationAngle);
	labelShape = QPainterPath();
	if (borderShape != TextLabel::BorderShape::NoBorder) {
		labelShape.addPath(WorksheetElement::shapeFromPath(borderShapePath, borderPen));
		transformedBoundingRectangle = matrix.mapRect(labelShape.boundingRect());
	} else {
		labelShape.addRect(boundingRectangle);
		transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	}

	labelShape = matrix.map(labelShape);

	// rotate gluePoints
	for (int i=0; i < m_gluePoints.length(); i++)
		m_gluePoints[i].point = matrix.map(m_gluePoints[i].point);

	emit q->changed();
}

void TextLabelPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (positionInvalid || textWrapper.text.isEmpty())
		return;

	painter->save();
	painter->rotate(-rotationAngle);

	//draw the text
	if (textWrapper.teXUsed) {
		if (boundingRect().width() != 0.0 &&  boundingRect().height() != 0.0)
			painter->drawImage(boundingRect(), teXImage);
	} else {
		// don't set pen color, the color is already in the HTML code
		//painter->setPen(fontColor);
		painter->scale(scaleFactor, scaleFactor);
		qreal w = staticText.size().width();
		qreal h = staticText.size().height();
		//staticText.setPerformanceHint(QStaticText::AggressiveCaching);
		//QDEBUG(Q_FUNC_INFO << ", Drawing text:" << staticText.text())
		painter->drawStaticText(QPointF(-w/2., -h/2.), staticText);
	}
	painter->restore();

	//draw the border
	if (borderShape != TextLabel::BorderShape::NoBorder) {
		painter->save();
		painter->setPen(borderPen);
		painter->setOpacity(borderOpacity);

		painter->rotate(-rotationAngle);
		painter->drawPath(borderShapePath);
		painter->restore();
	}

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(labelShape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(labelShape);
	}

#define DEBUG_TEXTLABEL_GLUEPOINTS 0
#if DEBUG_TEXTLABEL_GLUEPOINTS
	// just for debugging
	painter->setPen(QColor(Qt::GlobalColor::red));
	QRectF gluePointRect(0,0,10,10);
	for (int i=0; i < m_gluePoints.length(); i++) {
		gluePointRect.moveTo(m_gluePoints[i].point.x() - gluePointRect.width()/2, m_gluePoints[i].point.y() - gluePointRect.height()/2);
		painter->fillRect(gluePointRect, QColor(Qt::GlobalColor::red));
	}
#endif
}

QVariant TextLabelPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		//convert item's center point in parent's coordinates
		TextLabel::PositionWrapper tempPosition;
		tempPosition.point = positionFromItemPosition(mapParentToPlotArea(value.toPointF()));
		tempPosition.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
		tempPosition.verticalPosition = WorksheetElement::VerticalPosition::Custom;

		//emit the signals in order to notify the UI.
		// don't use setPosition here, because then all small changes are on the undo stack
		if(m_coordBindingEnable && m_coordBinding) {
			QPointF tempPoint = cSystem->mapSceneToLogical(tempPosition.point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			emit q->positionLogicalChanged(tempPoint);
		} else
			emit q->positionChanged(tempPosition);
	}

	return QGraphicsItem::itemChange(change, value);
}
/*
void TextLabelPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if(m_coordBinding)
		positionLogical = cSystem->mapSceneToLogical(mapParentToPlotArea(pos()), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	return QGraphicsItem::mouseMoveEvent(event);
}*/

void TextLabelPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//convert position of the item in parent coordinates to label's position
	QPointF point = positionFromItemPosition(mapParentToPlotArea(pos()));
	if (point != position.point) {
		//position was changed -> set the position related member variables
		suppressRetransform = true;
		TextLabel::PositionWrapper tempPosition;
		tempPosition.point = point;
		tempPosition.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
		tempPosition.verticalPosition = WorksheetElement::VerticalPosition::Custom;
		q->setPosition(tempPosition);
		suppressRetransform = false;
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

void TextLabelPrivate::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right
	        || event->key() == Qt::Key_Up ||event->key() == Qt::Key_Down) {
		const int delta = 5;
		QPointF point = positionFromItemPosition(pos());
		WorksheetElement::PositionWrapper tempPosition;

		if (event->key() == Qt::Key_Left) {
			point.setX(point.x() - delta);
			tempPosition.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
			tempPosition.verticalPosition = position.verticalPosition;
		} else if (event->key() == Qt::Key_Right) {
			point.setX(point.x() + delta);
			tempPosition.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
			tempPosition.verticalPosition = position.verticalPosition;
		} else if (event->key() == Qt::Key_Up) {
			point.setY(point.y() - delta);
			tempPosition.horizontalPosition = position.horizontalPosition;
			tempPosition.verticalPosition = WorksheetElement::VerticalPosition::Custom;
		} else if (event->key() == Qt::Key_Down) {
			point.setY(point.y() + delta);
			tempPosition.horizontalPosition = position.horizontalPosition;
			tempPosition.verticalPosition = WorksheetElement::VerticalPosition::Custom;
		}

		tempPosition.point = point;
		q->setPosition(tempPosition);
	}

	QGraphicsItem::keyPressEvent(event);
}

/*!
 *	converts label's position in plotArea coordinates to labels position.
 */
QPointF TextLabelPrivate::positionFromItemPosition(QPointF itemPos) {
	double x = itemPos.x();
	double y = itemPos.y();
	double w, h;
	QPointF tmpPosition;
	if (textWrapper.teXUsed) {
		w = teXImage.width()*scaleFactor;
		h = teXImage.height()*scaleFactor;
	} else {
		w = staticText.size().width()*scaleFactor;
		h = staticText.size().height()*scaleFactor;
	}

	//depending on the alignment, calculate the new position
	switch (horizontalAlignment) {
	case WorksheetElement::HorizontalAlignment::Left:
		tmpPosition.setX(x + w/2);
		break;
	case WorksheetElement::HorizontalAlignment::Center:
		tmpPosition.setX(x);
		break;
	case WorksheetElement::HorizontalAlignment::Right:
		tmpPosition.setX(x - w/2);
		break;
	}

	switch (verticalAlignment) {
	case WorksheetElement::VerticalAlignment::Top:
		tmpPosition.setY(y + h/2);
		break;
	case WorksheetElement::VerticalAlignment::Center:
		tmpPosition.setY(y);
		break;
	case WorksheetElement::VerticalAlignment::Bottom:
		tmpPosition.setY(y - h/2);
		break;
	}

	return tmpPosition;
}

/*!
 * \brief TextLabelPrivate::mapPlotAreaToParent
 * Mapping a point from the PlotArea (CartesianPlot::plotArea) coordinates to the parent
 * coordinates of this item
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Custompoint, so when changing anything, change it also there
 * \param point point in plotArea coordinates
 * \return point in parent coordinates
 */
QPointF TextLabelPrivate::mapPlotAreaToParent(QPointF point) {
	AbstractAspect* parent = q->parent(AspectType::CartesianPlot);

	if (parent) {
		CartesianPlot* plot = static_cast<CartesianPlot*>(parent);
		// first mapping to item coordinates and from there back to parent
		// WorksheetinfoElement: parentItem()->parentItem() == plot->graphicsItem()
		// plot->graphicsItem().pos() == plot->plotArea()->graphicsItem().pos()
		return mapToParent(mapFromItem(plot->plotArea()->graphicsItem(), point));
	}

	return point; // don't map if no parent set. Then it's during load
}

/*!
 * \brief TextLabelPrivate::mapParentToPlotArea
 * Mapping a point from parent coordinates to plotArea coordinates
 * Needed because in some cases the parent is not the PlotArea, but a child of it (Marker/InfoElement)
 * IMPORTANT: function is also used in Custompoint, so when changing anything, change it also there
 * \param point point in parent coordinates
 * \return point in PlotArea coordinates
 */
QPointF TextLabelPrivate::mapParentToPlotArea(QPointF point) {
	AbstractAspect* parent = q->parent(AspectType::CartesianPlot);
	if (parent) {
		CartesianPlot* plot = static_cast<CartesianPlot*>(parent);
		// mapping from parent to item coordinates and them to plot area
		return mapToItem(plot->plotArea()->graphicsItem(), mapFromParent(point));
	}

	return point; // don't map if no parent set. Then it's during load
}

void TextLabelPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void TextLabelPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void TextLabelPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		emit q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void TextLabel::save(QXmlStreamWriter* writer) const {
	Q_D(const TextLabel);

	writer->writeStartElement( "textLabel" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->position.point.x()) );
	writer->writeAttribute( "y", QString::number(d->position.point.y()) );
	writer->writeAttribute( "horizontalPosition", QString::number(static_cast<int>(d->position.horizontalPosition)) );
	writer->writeAttribute( "verticalPosition", QString::number(static_cast<int>(d->position.verticalPosition)) );
	writer->writeAttribute( "horizontalAlignment", QString::number(static_cast<int>(d->horizontalAlignment)) );
	writer->writeAttribute( "verticalAlignment", QString::number(static_cast<int>(d->verticalAlignment)) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeAttribute( "enableCoordBinding", QString::number(d->m_coordBindingEnable));
	writer->writeAttribute( "coordBinding", QString::number(d->m_coordBinding));
	writer->writeAttribute( "logicalPosX", QString::number(d->positionLogical.x()));
	writer->writeAttribute( "logicalPosY", QString::number(d->positionLogical.y()));
	writer->writeEndElement();

	writer->writeStartElement( "text" );
	writer->writeCharacters( d->textWrapper.text );
	writer->writeEndElement();

	writer->writeStartElement( "textPlaceholder");
	writer->writeCharacters( d->textWrapper.textPlaceholder);
	writer->writeEndElement();

	writer->writeStartElement( "format" );
	writer->writeAttribute( "placeholder", QString::number(d->textWrapper.allowPlaceholder) );
	writer->writeAttribute( "teXUsed", QString::number(d->textWrapper.teXUsed) );
	WRITE_QFONT(d->teXFont);
	writer->writeAttribute( "fontColor_r", QString::number(d->fontColor.red()) );
	writer->writeAttribute( "fontColor_g", QString::number(d->fontColor.green()) );
	writer->writeAttribute( "fontColor_b", QString::number(d->fontColor.blue()) );
	writer->writeEndElement();

	//border
	writer->writeStartElement("border");
	writer->writeAttribute("borderShape", QString::number(static_cast<int>(d->borderShape)));
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("borderOpacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	if (d->textWrapper.teXUsed) {
		writer->writeStartElement("teXImage");
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
		d->teXImage.save(&buffer, "PNG");
		writer->writeCharacters(ba.toBase64());
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "textLabel" section
}

//! Load from XML
bool TextLabel::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(TextLabel);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	bool teXImageFound = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "textLabel")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->position.point.setX(str.toDouble());

			str = attribs.value("y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("y").toString());
			else
				d->position.point.setY(str.toDouble());

			READ_INT_VALUE("horizontalPosition", position.horizontalPosition, WorksheetElement::HorizontalPosition);
			READ_INT_VALUE("verticalPosition", position.verticalPosition, WorksheetElement::VerticalPosition);
			READ_INT_VALUE("horizontalAlignment", horizontalAlignment, WorksheetElement::HorizontalAlignment);
			READ_INT_VALUE("verticalAlignment", verticalAlignment, WorksheetElement::VerticalAlignment);
			READ_DOUBLE_VALUE("rotationAngle", rotationAngle);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());

			str = attribs.value("enableCoordBinding").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("enableCoordBinding").toString());
			else
				enableCoordBinding(str.toInt());

			str = attribs.value("coordBinding").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("coordBinding").toString());
			else
				setCoordBinding(str.toInt());

			str = attribs.value("logicalPosX").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("logicalPosX").toString());
			else
				d->positionLogical.setX(str.toDouble());

			str = attribs.value("logicalPosY").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("logicalPosY").toString());
			else
				d->positionLogical.setY(str.toDouble());

		} else if (!preview && reader->name() == "text")
			d->textWrapper.text = reader->readElementText();
		else if (!preview && reader->name() == "textPlaceholder")
			d->textWrapper.textPlaceholder = reader->readElementText();
		else if (!preview && reader->name() == "format") {
			attribs = reader->attributes();

			READ_INT_VALUE("teXUsed", textWrapper.teXUsed, bool);
			READ_QFONT(d->teXFont);

			str = attribs.value("placeholder").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("teXUsed").toString());
			else
				d->textWrapper.allowPlaceholder = str.toInt();

			str = attribs.value("teXUsed").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("teXUsed").toString());
			else
				d->textWrapper.teXUsed = str.toInt();

			str = attribs.value("fontColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fontColor_r").toString());
			else
				d->fontColor.setRed( str.toInt() );

			str = attribs.value("fontColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fontColor_g").toString());
			else
				d->fontColor.setGreen( str.toInt() );

			str = attribs.value("fontColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fontColor_b").toString());
			else
				d->fontColor.setBlue( str.toInt() );
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();
			READ_INT_VALUE("borderShape", borderShape, BorderShape);
			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("borderOpacity", borderOpacity);
		} else if (!preview && reader->name() == "teXImage") {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray ba = QByteArray::fromBase64(content.toLatin1());
			teXImageFound = d->teXImage.loadFromData(ba);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (preview)
		return true;

	//in case we use latex and the image was stored (older versions of LabPlot didn't save the image)and loaded,
	//we just need to retransform.
	//otherwise, we set the static text and retransform in updateText()
	if ( !(d->textWrapper.teXUsed && teXImageFound) )
		d->updateText();
	else
		retransform();

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void TextLabel::loadThemeConfig(const KConfig& config) {
	Q_D(TextLabel);

	KConfigGroup group = config.group("Label");
	d->fontColor = group.readEntry("FontColor", QColor(Qt::black)); // used when it's latex text
	d->backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::white)); // used when it's latex text

	if (!d->textWrapper.teXUsed && !d->textWrapper.text.isEmpty()) {
		// TODO: Replace QTextEdit by QTextDocument, because this does not contain the graphical stuff
		// to set the color in a html text, a qTextEdit must be used
		QTextEdit te;
		te.setHtml(d->textWrapper.text);
		te.selectAll();
		te.setTextColor(d->fontColor);
		//te.setTextBackgroundColor(backgroundColor); // for plain text no background color supported, due to bug https://bugreports.qt.io/browse/QTBUG-25420

		TextWrapper wrapper(te.toHtml(), false, true);
		te.setHtml(d->textWrapper.textPlaceholder);
		te.selectAll();
		te.setTextColor(d->fontColor);
		wrapper.textPlaceholder = te.toHtml();
		wrapper.allowPlaceholder = d->textWrapper.allowPlaceholder;

		// update the text. also in the Widget to which is connected

		setText(wrapper);
	}

	// otherwise when changing theme while the textlabel dock is visible, the
	// color comboboxes do not change the color
	backgroundColorChanged(d->backgroundColor);
	fontColorChanged(d->fontColor);

	group = config.group("CartesianPlot");
	QPen pen = this->borderPen();
	pen.setColor(group.readEntry("BorderColor", pen.color()));
	pen.setStyle((Qt::PenStyle)(group.readEntry("BorderStyle", (int) pen.style())));
	pen.setWidthF(group.readEntry("BorderWidth", pen.widthF()));
	this->setBorderPen(pen);
	this->setBorderOpacity(group.readEntry("BorderOpacity", this->borderOpacity()));
}

void TextLabel::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("Label");
	//TODO
// 	group.writeEntry("TeXFontColor", (QColor) this->fontColor());
}
