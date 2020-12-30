/***************************************************************************
  File                 : DatapickerImage.h
  Project              : LabPlot
  Description          : Worksheet for Datapicker
  --------------------------------------------------------------------
  Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
  Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)

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

#ifndef DATAPICKERIMAGE_H
#define DATAPICKERIMAGE_H

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include <QVector3D>
#include <QPen>

class QImage;
class QBrush;
class DatapickerImagePrivate;
class DatapickerImageView;
class ImageEditor;
class Segments;

class QGraphicsScene;
class QGraphicsPixmapItem;

class DatapickerImage : public AbstractPart {
	Q_OBJECT

public:
	explicit DatapickerImage(const QString& name, bool loading = false);
	~DatapickerImage() override;

	enum class GraphType {Cartesian, PolarInDegree, PolarInRadians, LogarithmicX, LogarithmicY, Ternary};
	enum class ColorAttributes {None, Intensity, Foreground, Hue, Saturation, Value};
	enum class PlotImageType {NoImage, OriginalImage, ProcessedImage};
	enum class PointsType {AxisPoints, CurvePoints, SegmentPoints};

	struct ReferencePoints {
		GraphType type{GraphType::Cartesian};
		QPointF scenePos[3];
		QVector3D logicalPos[3];
		double ternaryScale{1.0};
	};

	struct EditorSettings {
		int intensityThresholdLow{20};
		int intensityThresholdHigh{100};
		int foregroundThresholdLow{30};
		int foregroundThresholdHigh{90};
		int hueThresholdLow{0};
		int hueThresholdHigh{360};
		int saturationThresholdLow{30};
		int saturationThresholdHigh{100};
		int valueThresholdLow{30};
		int valueThresholdHigh{90};
	};

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	void createContextMenu(QMenu*);
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QRectF pageRect() const;
	void setPageRect(const QRectF&);
	QGraphicsScene *scene() const;
	void setPrinting(bool) const;
	void setSelectedInView(const bool);
	void setSegmentsHoverEvent(const bool);

	void setPlotImageType(const DatapickerImage::PlotImageType);
	DatapickerImage::PlotImageType plotImageType();

	bool isLoaded{false};
	QImage originalPlotImage;
	QImage processedPlotImage;
	QColor background;
	int *foregroundBins;
	int *hueBins;
	int *saturationBins;
	int *valueBins;
	int *intensityBins;

	QGraphicsPixmapItem* m_magnificationWindow{nullptr};

	CLASS_D_ACCESSOR_DECL(QString, fileName, FileName)
	CLASS_D_ACCESSOR_DECL(DatapickerImage::ReferencePoints, axisPoints, AxisPoints)
	CLASS_D_ACCESSOR_DECL(DatapickerImage::EditorSettings, settings, Settings)
	BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(PointsType, plotPointsType, PlotPointsType)
	BASIC_D_ACCESSOR_DECL(int, pointSeparation, PointSeparation)
	BASIC_D_ACCESSOR_DECL(int, minSegmentLength, minSegmentLength)

	BASIC_D_ACCESSOR_DECL(Symbol::Style, pointStyle, PointStyle)
	BASIC_D_ACCESSOR_DECL(qreal, pointOpacity, PointOpacity)
	BASIC_D_ACCESSOR_DECL(qreal, pointRotationAngle, PointRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, pointSize, PointSize)
	CLASS_D_ACCESSOR_DECL(QBrush, pointBrush, PointBrush)
	CLASS_D_ACCESSOR_DECL(QPen, pointPen, PointPen)
	BASIC_D_ACCESSOR_DECL(bool, pointVisibility, PointVisibility)

	typedef DatapickerImagePrivate Private;

private:
	void init();

	DatapickerImagePrivate* const d;
	mutable DatapickerImageView* m_view{nullptr};
	friend class DatapickerImagePrivate;
	Segments* m_segments;

signals:
	void requestProjectContextMenu(QMenu*);
	void requestUpdate();
	void requestUpdateActions();

	void fileNameChanged(const QString&);
	void rotationAngleChanged(float);
	void axisPointsChanged(const DatapickerImage::ReferencePoints&);
	void settingsChanged(const DatapickerImage::EditorSettings&);
	void minSegmentLengthChanged(const int);
	void pointStyleChanged(Symbol::Style);
	void pointSizeChanged(qreal);
	void pointRotationAngleChanged(qreal);
	void pointOpacityChanged(qreal);
	void pointBrushChanged(QBrush);
	void pointPenChanged(const QPen&);
	void pointVisibilityChanged(bool);
};
#endif
