/***************************************************************************
    File                 : AbstractWorksheetElement.cpp
    Project              : LabPlot/SciDAVis
    Description          : Base class for all Worksheet children.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/Worksheet.h"
#include "worksheet/AbstractWorksheetElement.h"
#include "worksheet/AbstractCoordinateSystem.h"

#include <QPen>
#include <QPainterPath>
#include <QPainterPathStroker>

/**
 * \class AbstractWorksheetElement
 * \brief Base class for all Worksheet children.
 *
 */

AbstractWorksheetElement::AbstractWorksheetElement(const QString &name)
		: AbstractAspect(name) {
}

AbstractWorksheetElement::~AbstractWorksheetElement() {
}

/**
 * \fn QGraphicsItem *AbstractWorksheetElement::graphicsItem() const
 * \brief Return the graphics item representing this element.
 *
 *
 */

/**
 * \fn void AbstractWorksheetElement::setZValue(qreal z)
 * \brief Set the Z value for the drawing order.
 *
 * An element of high Z-value will be drawn on top of all elements in the worksheet with a lower Z-value.
 * Contrary to a QGraphicsItem's Z-value, the parent-child relation does not matter here. Using this
 * function on worksheet element containers is mostly not what you want, 
 * use WorksheetElementContainer::setZValueRange() instead.
 *
 * \see void AbstractWorksheetElement::setZValueRange(qreal minZ, qreal maxZ)
 */

/**
 * \fn qreal AbstractWorksheetElement::zValue () const
 * \brief Return the Z value for the drawing order.
 *
 * Using this function on worksheet element containers is mostly not what you want, 
 * use WorksheetElementContainer::zValueMin() and WorksheetElementContainer::zValueMax() instead.
 *
 * \see setZValue()
 */

#if 0
/**
 * \fn QRectF AbstractWorksheetElement::boundingRect() const
 * \brief Return the bounding rect in scene coodinates.
 *
 */

/**
 * \fn bool AbstractWorksheetElement::contains(const QPointF &position) const
 * \brief Return whether the given position is inside the element.
 *
 */
#endif

/**
 * \fn void AbstractWorksheetElement::setVisible(bool on)
 * \brief Show/hide the element.
 *
 */

/**
 * \fn bool AbstractWorksheetElement::isVisible() const
 * \brief Return whether the element is (at least) partially visible.
 *
 */

/**
 * \brief Return whether the element is fully visible (i.e., including all child elements).
 *
 * The standard implementation returns isVisible().
 */
bool AbstractWorksheetElement::isFullyVisible() const {
	return isVisible();
}
		
/**
 * \fn void AbstractWorksheetElement::retransform()
 * \brief Tell the element to newly transform its graphics item into its coordinate system.
 *
 * This method must not change the undo-aware data of the element, only
 * the graphics item which represents the item is to be updated.
 */
		
/**
 * \fn AbstractCoordinateSystem *AbstractWorksheetElement::coordinateSystem() const
 * \brief Return the current coordinate system (can be NULL which means don't transform).
 *
 * The standard implementation looks for the first ancestor within the worksheet which
 * inherits AbstractCoordinateSystem.
 */

AbstractCoordinateSystem *AbstractWorksheetElement::coordinateSystem() const {
	AbstractAspect * parent = parentAspect();
	while (parent) {
		AbstractCoordinateSystem *system = qobject_cast<AbstractCoordinateSystem *>(parent);
		if (system)
			return system;
		Worksheet *worksheet = qobject_cast<Worksheet *>(parent);
		if (worksheet)
			return NULL;
		parent = parent->parentAspect();
	}
	return NULL;
}

/**
    This does exactly what Qt internally does to creates a shape from a painter path.
*/
QPainterPath AbstractWorksheetElement::shapeFromPath(const QPainterPath &path, const QPen &pen)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);

    if (path == QPainterPath())
        return path;
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    if (pen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

