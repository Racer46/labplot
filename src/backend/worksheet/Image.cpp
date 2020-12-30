/***************************************************************************
    File                 : Image.cpp
    Project              : LabPlot
    Description          : Worksheet element to draw images
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke (alexander.semke@web.de)
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

#include "Image.h"
#include "Worksheet.h"
#include "ImagePrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <QIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class Image
 * \brief A label supporting rendering of html- and tex-formatted texts.
 *
 * The label is aligned relative to the specified position.
 * The position can be either specified by providing the x- and y- coordinates
 * in parent's coordinate system, or by specifying one of the predefined position
 * flags (\c HorizontalPosition, \c VerticalPosition).
 */


Image::Image(const QString& name)
	: WorksheetElement(name, AspectType::Image), d_ptr(new ImagePrivate(this)) {

	init();
}

Image::Image(const QString &name, ImagePrivate *dd)
	: WorksheetElement(name, AspectType::Image), d_ptr(dd) {

	init();
}

void Image::init() {
	Q_D(Image);

	KConfig config;
	KConfigGroup group = config.group("Image");

	d->opacity = group.readEntry("opacity", d->opacity);

	//geometry
	d->position.point.setX( group.readEntry("PositionXValue", d->position.point.x()) );
	d->position.point.setY( group.readEntry("PositionYValue", d->position.point.y()) );
	d->position.horizontalPosition = (WorksheetElement::HorizontalPosition) group.readEntry("PositionX", (int)d->position.horizontalPosition);
	d->position.verticalPosition = (WorksheetElement::VerticalPosition) group.readEntry("PositionY", (int)d->position.verticalPosition);
	d->horizontalAlignment = (WorksheetElement::HorizontalAlignment) group.readEntry("HorizontalAlignment", (int)d->horizontalAlignment);
	d->verticalAlignment = (WorksheetElement::VerticalAlignment) group.readEntry("VerticalAlignment", (int)d->verticalAlignment);
	d->rotationAngle = group.readEntry("Rotation", d->rotationAngle);

	//border
	d->borderPen = QPen(group.readEntry("BorderColor", d->borderPen.color()),
						group.readEntry("BorderWidth", d->borderPen.widthF()),
						(Qt::PenStyle) group.readEntry("BorderStyle", (int)(d->borderPen.style())));
	d->borderOpacity = group.readEntry("BorderOpacity", d->borderOpacity);

	//initial placeholder image
	d->image = QIcon::fromTheme("viewimage").pixmap(d->width, d->height).toImage();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
Image::~Image() = default;

QGraphicsItem* Image::graphicsItem() const {
	return d_ptr;
}

void Image::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(Image);
	d->setParentItem(item);
	d->updatePosition();
}

void Image::retransform() {
	Q_D(Image);
	d->retransform();
}

void Image::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG("Image::handleResize()");
	Q_UNUSED(pageResize);
	Q_UNUSED(horizontalRatio);
	Q_UNUSED(verticalRatio);
// 	Q_D(Image);

// 	double ratio = 0;
// 	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
// 		ratio = qMax(horizontalRatio, verticalRatio);
// 	else
// 		ratio = qMin(horizontalRatio, verticalRatio);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Image::icon() const{
	return QIcon::fromTheme("viewimage");
}

QMenu* Image::createContextMenu() {
	QMenu *menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"

	if (!visibilityAction) {
		visibilityAction = new QAction(i18n("Visible"), this);
		visibilityAction->setCheckable(true);
		connect(visibilityAction, &QAction::triggered, this, &Image::visibilityChanged);
	}

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(Image, QString, fileName, fileName)
BASIC_SHARED_D_READER_IMPL(Image, qreal, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(Image, int, width, width)
BASIC_SHARED_D_READER_IMPL(Image, int, height, height)
BASIC_SHARED_D_READER_IMPL(Image, bool, keepRatio, keepRatio)
CLASS_SHARED_D_READER_IMPL(Image, WorksheetElement::PositionWrapper, position, position)
BASIC_SHARED_D_READER_IMPL(Image, WorksheetElement::HorizontalAlignment, horizontalAlignment, horizontalAlignment)
BASIC_SHARED_D_READER_IMPL(Image, WorksheetElement::VerticalAlignment, verticalAlignment, verticalAlignment)
BASIC_SHARED_D_READER_IMPL(Image, qreal, rotationAngle, rotationAngle)

CLASS_SHARED_D_READER_IMPL(Image, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(Image, qreal, borderOpacity, borderOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(Image, SetFileName, QString, fileName, updateImage)
void Image::setFileName(const QString& fileName) {
	Q_D(Image);
	if (fileName != d->fileName)
		exec(new ImageSetFileNameCmd(d, fileName, ki18n("%1: set image")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetOpacity, qreal, opacity, update)
void Image::setOpacity(qreal opacity) {
	Q_D(Image);
	if (opacity != d->opacity)
		exec(new ImageSetOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetWidth, int, width, scaleImage)
void Image::setWidth(int width) {
	Q_D(Image);
	if (width != d->width)
		exec(new ImageSetWidthCmd(d, width, ki18n("%1: set width")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetHeight, int, height, scaleImage)
void Image::setHeight(int height) {
	Q_D(Image);
	if (height != d->height)
		exec(new ImageSetHeightCmd(d, height, ki18n("%1: set height")));
}

STD_SETTER_CMD_IMPL_S(Image, SetKeepRatio, bool, keepRatio)
void Image::setKeepRatio(bool keepRatio) {
	Q_D(Image);
	if (keepRatio != d->keepRatio)
		exec(new ImageSetKeepRatioCmd(d, keepRatio, ki18n("%1: change keep ratio")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetPosition, WorksheetElement::PositionWrapper, position, retransform);
void Image::setPosition(const WorksheetElement::PositionWrapper& pos) {
	Q_D(Image);
	if (pos.point != d->position.point || pos.horizontalPosition != d->position.horizontalPosition || pos.verticalPosition != d->position.verticalPosition)
		exec(new ImageSetPositionCmd(d, pos, ki18n("%1: set position")));
}

/*!
	sets the position without undo/redo-stuff
*/
void Image::setPosition(QPointF point) {
	Q_D(Image);
	if (point != d->position.point) {
		d->position.point = point;
		retransform();
	}
}

STD_SETTER_CMD_IMPL_F_S(Image, SetRotationAngle, qreal, rotationAngle, recalcShapeAndBoundingRect);
void Image::setRotationAngle(qreal angle) {
	Q_D(Image);
	if (angle != d->rotationAngle)
		exec(new ImageSetRotationAngleCmd(d, angle, ki18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetHorizontalAlignment, WorksheetElement::HorizontalAlignment, horizontalAlignment, retransform);
void Image::setHorizontalAlignment(const WorksheetElement::HorizontalAlignment hAlign) {
	Q_D(Image);
	if (hAlign != d->horizontalAlignment)
		exec(new ImageSetHorizontalAlignmentCmd(d, hAlign, ki18n("%1: set horizontal alignment")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetVerticalAlignment, WorksheetElement::VerticalAlignment, verticalAlignment, retransform);
void Image::setVerticalAlignment(const WorksheetElement::VerticalAlignment vAlign) {
	Q_D(Image);
	if (vAlign != d->verticalAlignment)
		exec(new ImageSetVerticalAlignmentCmd(d, vAlign, ki18n("%1: set vertical alignment")));
}

//Border
STD_SETTER_CMD_IMPL_F_S(Image, SetBorderPen, QPen, borderPen, update)
void Image::setBorderPen(const QPen &pen) {
	Q_D(Image);
	if (pen != d->borderPen)
		exec(new ImageSetBorderPenCmd(d, pen, ki18n("%1: set border")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetBorderOpacity, qreal, borderOpacity, update)
void Image::setBorderOpacity(qreal opacity) {
	Q_D(Image);
	if (opacity != d->borderOpacity)
		exec(new ImageSetBorderOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

//misc
STD_SWAP_METHOD_SETTER_CMD_IMPL_F(Image, SetVisible, bool, swapVisible, retransform);
void Image::setVisible(bool on) {
	Q_D(Image);
	exec(new ImageSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool Image::isVisible() const {
	Q_D(const Image);
	return d->isVisible();
}

void Image::setPrinting(bool on) {
	Q_D(Image);
	d->m_printing = on;
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void Image::visibilityChanged() {
	Q_D(const Image);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
ImagePrivate::ImagePrivate(Image* owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setAcceptHoverEvents(true);
}

QString ImagePrivate::name() const {
	return q->name();
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void ImagePrivate::retransform() {
	if (suppressRetransform)
		return;

	if (position.horizontalPosition != WorksheetElement::HorizontalPosition::Custom
	        || position.verticalPosition != WorksheetElement::VerticalPosition::Custom)
		updatePosition();

	float x = position.point.x();
	float y = position.point.y();
	float w = image.width();
	float h = image.height();

 	//depending on the alignment, calculate the new GraphicsItem's position in parent's coordinate system
	QPointF itemPos;
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

	suppressItemChangeEvent = true;
	setPos(itemPos);
	suppressItemChangeEvent = false;

	boundingRectangle.setX(-w/2);
	boundingRectangle.setY(-h/2);
	boundingRectangle.setWidth(w);
	boundingRectangle.setHeight(h);

	updateBorder();
}

void ImagePrivate::updateImage() {
	if (!fileName.isEmpty()) {
		image = QImage(fileName);
		width = image.width();
		height = image.height();
		q->widthChanged(width);
		q->heightChanged(height);
	} else {
		width = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
		height = Worksheet::convertToSceneUnits(3, Worksheet::Unit::Centimeter);
		image = QIcon::fromTheme("viewimage").pixmap(width, height).toImage();
		q->widthChanged(width);
		q->heightChanged(height);
	}

	retransform();
}

void ImagePrivate::scaleImage() {
	if (keepRatio) {
		if (width != image.width()) {
			//width was changed -> rescale the height to keep the ratio
			if (image.width() != 0)
				height = image.height()*width/image.width();
			else
				height = 0;
			q->heightChanged(height);
		} else {
			//height was changed -> rescale the width to keep the ratio
			if (image.height() != 0)
				width = image.width()*height/image.height();
			else
				width = 0;
			q->widthChanged(width);
		}
	}

	if (width != 0 && height != 0)
		image = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	retransform();
}

/*!
	calculates the position of the label, when the position relative to the parent was specified (left, right, etc.)
*/
void ImagePrivate::updatePosition() {
	//determine the parent item
	QRectF parentRect;
	QGraphicsItem* parent = parentItem();
	if (parent) {
		parentRect = parent->boundingRect();
	} else {
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

	emit q->positionChanged(position);
}

void ImagePrivate::updateBorder() {
	borderShapePath = QPainterPath();
	borderShapePath.addRect(boundingRectangle);
	recalcShapeAndBoundingRect();
}

bool ImagePrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	emit q->changed();
	emit q->visibleChanged(on);
	return oldValue;
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF ImagePrivate::boundingRect() const {
	return transformedBoundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath ImagePrivate::shape() const {
	return imageShape;
}

/*!
  recalculates the outer bounds and the shape of the label.
*/
void ImagePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QMatrix matrix;
	matrix.rotate(-rotationAngle);
	imageShape = QPainterPath();
	if (borderPen.style() != Qt::NoPen) {
		imageShape.addPath(WorksheetElement::shapeFromPath(borderShapePath, borderPen));
		transformedBoundingRectangle = matrix.mapRect(imageShape.boundingRect());
	} else {
		imageShape.addRect(boundingRectangle);
		transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	}

	imageShape = matrix.map(imageShape);

	emit q->changed();
}

void ImagePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->save();

	//draw the image
	painter->rotate(-rotationAngle);
	painter->setOpacity(opacity);
	painter->drawImage(boundingRectangle.topLeft(), image, image.rect());
	painter->restore();

	//draw the border
	if (borderPen.style() != Qt::NoPen) {
		painter->save();
		painter->rotate(-rotationAngle);
		painter->setPen(borderPen);
		painter->setOpacity(borderOpacity);
		painter->drawPath(borderShapePath);
		painter->restore();
	}

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(imageShape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(imageShape);
	}
}

QVariant ImagePrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		//convert item's center point in parent's coordinates
		WorksheetElement::PositionWrapper tempPosition;
		tempPosition.point = positionFromItemPosition(value.toPointF());
		tempPosition.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
		tempPosition.verticalPosition = WorksheetElement::VerticalPosition::Custom;

		//emit the signals in order to notify the UI.
		//we don't set the position related member variables during the mouse movements.
		//this is done on mouse release events only.
		emit q->positionChanged(tempPosition);
	}

	return QGraphicsItem::itemChange(change, value);
}

void ImagePrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//convert position of the item in parent coordinates to label's position
	QPointF point = positionFromItemPosition(pos());
	if (qAbs(point.x()-position.point.x())>20 && qAbs(point.y()-position.point.y())>20 ) {
		//position was changed -> set the position related member variables
		suppressRetransform = true;
		WorksheetElement::PositionWrapper tempPosition;
		tempPosition.point = point;
		tempPosition.horizontalPosition = WorksheetElement::HorizontalPosition::Custom;
		tempPosition.verticalPosition = WorksheetElement::VerticalPosition::Custom;
		q->setPosition(tempPosition);
		suppressRetransform = false;
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

/*!
 *	converts label's position to GraphicsItem's position.
 */
QPointF ImagePrivate::positionFromItemPosition(QPointF itemPos) {
	float x = itemPos.x();
	float y = itemPos.y();
	float w = image.width();
	float h = image.height();
	QPointF tmpPosition;

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

void ImagePrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void ImagePrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void ImagePrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
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
void Image::save(QXmlStreamWriter* writer) const {
	Q_D(const Image);

	writer->writeStartElement("image");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("fileName", d->fileName);
	writer->writeAttribute("opacity", QString::number(d->opacity));
	writer->writeEndElement();

	//geometry
	writer->writeStartElement("geometry");
	writer->writeAttribute("width", QString::number(d->width));
	writer->writeAttribute("height", QString::number(d->height));
	writer->writeAttribute("keepRatio", QString::number(d->keepRatio));
	writer->writeAttribute("x", QString::number(d->position.point.x()));
	writer->writeAttribute("y", QString::number(d->position.point.y()));
	writer->writeAttribute("horizontalPosition", QString::number(static_cast<int>(d->position.horizontalPosition)));
	writer->writeAttribute("verticalPosition", QString::number(static_cast<int>(d->position.verticalPosition)));
	writer->writeAttribute("horizontalAlignment", QString::number(static_cast<int>(d->horizontalAlignment)));
	writer->writeAttribute("verticalAlignment", QString::number(static_cast<int>(d->verticalAlignment)));
	writer->writeAttribute("rotationAngle", QString::number(d->rotationAngle));
	writer->writeAttribute("visible", QString::number(d->isVisible()));
	writer->writeEndElement();

	//border
	writer->writeStartElement("border");
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("borderOpacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	writer->writeEndElement(); // close "image" section
}

//! Load from XML
bool Image::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(Image);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "image")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();
			d->fileName = attribs.value("fileName").toString();
			READ_DOUBLE_VALUE("opacity", opacity);
		} else if (!preview && reader->name() == "geometry") {
			attribs = reader->attributes();

			READ_INT_VALUE("width", width, int);
			READ_INT_VALUE("height", height, int);
			READ_INT_VALUE("keepRatio", keepRatio, bool);

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
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();
			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("borderOpacity", borderOpacity);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (!preview) {
		d->image = QImage(d->fileName);
		d->scaleImage();
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Image::loadThemeConfig(const KConfig& config) {
	Q_UNUSED(config)
}

void Image::saveThemeConfig(const KConfig& config) {
	Q_UNUSED(config)
}
