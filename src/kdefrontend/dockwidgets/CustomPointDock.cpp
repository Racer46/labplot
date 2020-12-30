/***************************************************************************
    File                 : CustomPointDock.cpp
    Project              : LabPlot
    Description          : widget for CustomPoint properties
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

#include "CustomPointDock.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"

#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <KLocalizedString>
#include <KConfig>

#include <QPainter>
#include <QDir>

CustomPointDock::CustomPointDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	//Validators
	ui.lePositionX->setValidator( new QDoubleValidator(ui.lePositionX) );
	ui.lePositionY->setValidator( new QDoubleValidator(ui.lePositionY) );

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	CustomPointDock::updateLocale();

	//SLOTS
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CustomPointDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &CustomPointDock::commentChanged);
	connect(ui.lePositionX, &QLineEdit::textChanged, this, &CustomPointDock::positionXChanged);
	connect(ui.dateTimeEditPositionX, &QDateTimeEdit::dateTimeChanged, this, &CustomPointDock::positionXDateTimeChanged);
	connect(ui.lePositionY, &QLineEdit::textChanged, this, &CustomPointDock::positionYChanged);
	connect( ui.chkVisible, &QCheckBox::clicked, this, &CustomPointDock::visibilityChanged);

	//Symbols
	connect( ui.cbSymbolStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomPointDock::symbolStyleChanged);
	connect( ui.sbSymbolSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CustomPointDock::symbolSizeChanged);
	connect( ui.sbSymbolRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomPointDock::symbolRotationChanged);
	connect( ui.sbSymbolOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &CustomPointDock::symbolOpacityChanged);
	connect( ui.cbSymbolFillingStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomPointDock::symbolFillingStyleChanged);
	connect( ui.kcbSymbolFillingColor, &KColorButton::changed, this, &CustomPointDock::symbolFillingColorChanged);
	connect( ui.cbSymbolBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CustomPointDock::symbolBorderStyleChanged);
	connect( ui.kcbSymbolBorderColor, &KColorButton::changed, this, &CustomPointDock::symbolBorderColorChanged);
	connect( ui.sbSymbolBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CustomPointDock::symbolBorderWidthChanged);

	//Template handler
	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::CustomPoint);
	ui.verticalLayout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CustomPointDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CustomPointDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CustomPointDock::info);

	init();
}

void CustomPointDock::init() {
	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);

	QPainter pa;
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	ui.cbSymbolStyle->setIconSize(QSize(iconSize, iconSize));

	QPen pen(Qt::SolidPattern, 0);
	const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);
	pa.setPen(pen);

	QTransform trafo;
	trafo.scale(15, 15);
	for (int i = 1; i < Symbol::stylesCount(); ++i) { //skip the first style "None"
		auto style = (Symbol::Style)i;
		pm.fill(Qt::transparent);
		pa.begin(&pm);
		pa.setPen( pen );
		pa.setRenderHint(QPainter::Antialiasing);
		pa.translate(iconSize/2,iconSize/2);
		pa.drawPath(trafo.map(Symbol::pathFromStyle(style)));
		pa.end();
		ui.cbSymbolStyle->addItem(QIcon(pm), Symbol::nameFromStyle(style), (int)style);
	}
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, Qt::black);
	m_initializing = false;
}

void CustomPointDock::setPoints(QList<CustomPoint*> list) {
	const Lock lock(m_initializing);
	m_pointsList = list;
	m_point = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_point);

	//if there is more than one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_point->name());
		ui.leComment->setText(m_point->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first custom point
	this->load();

	//for custom points being children of an InfoElement, the position is changed
	//via the parent settings -> disable the positioning here.
	bool enabled = (m_point->parentAspect()->type() != AspectType::InfoElement);
	ui.lePositionX->setEnabled(enabled);
	ui.dateTimeEditPositionX->setEnabled(enabled);
	ui.lePositionY->setEnabled(enabled);

	//SIGNALs/SLOTs
	// general
	connect(m_point, &CustomPoint::aspectDescriptionChanged, this, &CustomPointDock::pointDescriptionChanged);
	connect(m_point, &CustomPoint::positionChanged, this, &CustomPointDock::pointPositionChanged);
	connect(m_point, &CustomPoint::visibleChanged, this, &CustomPointDock::pointVisibilityChanged);

	//symbol
	connect(m_point, &CustomPoint::symbolStyleChanged, this, &CustomPointDock::pointSymbolStyleChanged);
	connect(m_point, &CustomPoint::symbolSizeChanged, this, &CustomPointDock::pointSymbolSizeChanged);
	connect(m_point, &CustomPoint::symbolRotationAngleChanged, this, &CustomPointDock::pointSymbolRotationAngleChanged);
	connect(m_point, &CustomPoint::symbolOpacityChanged, this, &CustomPointDock::pointSymbolOpacityChanged);
	connect(m_point, &CustomPoint::symbolBrushChanged, this, &CustomPointDock::pointSymbolBrushChanged);
	connect(m_point, &CustomPoint::symbolPenChanged, this, &CustomPointDock::pointSymbolPenChanged);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void CustomPointDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbSymbolSize->setLocale(numberLocale);
	ui.sbSymbolBorderWidth->setLocale(numberLocale);
	ui.lePositionX->setLocale(numberLocale);
	ui.lePositionY->setLocale(numberLocale);
}

//**********************************************************
//**** SLOTs for changes triggered in CustomPointDock ******
//**********************************************************
//"General"-tab
void CustomPointDock::positionXChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	double x = numberLocale.toDouble(ui.lePositionX->text(), &ok);
	if (ok) {
		QPointF pos{m_point->position()};
		pos.setX(x);
		for (auto* point : m_pointsList)
			point->setPosition(pos);
	}
}

void CustomPointDock::positionXDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 x = dateTime.toMSecsSinceEpoch();
	QPointF pos{m_point->position()};
	pos.setX(x);
	for (auto* point : m_pointsList)
		point->setPosition(pos);
}

void CustomPointDock::positionYChanged() {
	if (m_initializing)
		return;

	bool ok;
	SET_NUMBER_LOCALE
	double y = numberLocale.toDouble(ui.lePositionY->text(), &ok);
	if (ok) {
		QPointF pos{m_point->position()};
		pos.setY(y);
		for (auto* point : m_pointsList)
			point->setPosition(pos);
	}
}

void CustomPointDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	m_point->beginMacro(i18n("%1 CustomPoints: visibility changed", m_pointsList.count()));
	for (auto* point : m_pointsList)
		point->setVisible(state);
	m_point->endMacro();
}

void CustomPointDock::symbolStyleChanged(int) {
	auto style = static_cast<Symbol::Style>(ui.cbSymbolStyle->itemData(ui.cbSymbolStyle->currentIndex()).toInt());
	//enable/disable the  filling options in the GUI depending on the currently selected points.
	if (style != Symbol::Style::Line && style != Symbol::Style::Cross) {
		ui.cbSymbolFillingStyle->setEnabled(true);
		bool noBrush = (Qt::BrushStyle(ui.cbSymbolFillingStyle->currentIndex()) == Qt::NoBrush);
		ui.kcbSymbolFillingColor->setEnabled(!noBrush);
	} else {
		ui.kcbSymbolFillingColor->setEnabled(false);
		ui.cbSymbolFillingStyle->setEnabled(false);
	}

	bool noLine = (Qt::PenStyle(ui.cbSymbolBorderStyle->currentIndex()) == Qt::NoPen);
	ui.kcbSymbolBorderColor->setEnabled(!noLine);
	ui.sbSymbolBorderWidth->setEnabled(!noLine);

	if (m_initializing)
		return;

	m_point->beginMacro(i18n("%1 CustomPoints: style changed", m_pointsList.count()));
	for (auto* point : m_pointsList)
		point->setSymbolStyle(style);
	m_point->endMacro();
}

void CustomPointDock::symbolSizeChanged(double value) {
	if (m_initializing)
		return;

	m_point->beginMacro(i18n("%1 CustomPoints: size changed", m_pointsList.count()));
	for (auto* point : m_pointsList)
		point->setSymbolSize( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
	m_point->endMacro();
}

void CustomPointDock::symbolRotationChanged(int value) {
	if (m_initializing)
		return;

	m_point->beginMacro(i18n("%1 CustomPoints: rotation changed", m_pointsList.count()));
	for (auto* point : m_pointsList)
		point->setSymbolRotationAngle(value);
	m_point->endMacro();
}

void CustomPointDock::symbolOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	m_point->beginMacro(i18n("%1 CustomPoints: opacity changed", m_pointsList.count()));
	for (auto* point : m_pointsList)
		point->setSymbolOpacity(opacity);
	m_point->endMacro();
}

void CustomPointDock::symbolFillingStyleChanged(int index) {
	auto brushStyle = Qt::BrushStyle(index);
	ui.kcbSymbolFillingColor->setEnabled(!(brushStyle == Qt::NoBrush));

	if (m_initializing)
		return;

	QBrush brush;
	m_point->beginMacro(i18n("%1 CustomPoints: filling style changed", m_pointsList.count()));
	for (auto* point : m_pointsList) {
		brush = point->symbolBrush();
		brush.setStyle(brushStyle);
		point->setSymbolBrush(brush);
	}
	m_point->endMacro();
}

void CustomPointDock::symbolFillingColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QBrush brush;
	m_point->beginMacro(i18n("%1 CustomPoints: filling color changed", m_pointsList.count()));
	for (auto* point : m_pointsList) {
		brush = point->symbolBrush();
		brush.setColor(color);
		point->setSymbolBrush(brush);
	}
	m_point->endMacro();

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, color);
	m_initializing = false;
}

void CustomPointDock::symbolBorderStyleChanged(int index) {
	auto penStyle = Qt::PenStyle(index);

	if (penStyle == Qt::NoPen) {
		ui.kcbSymbolBorderColor->setEnabled(false);
		ui.sbSymbolBorderWidth->setEnabled(false);
	} else {
		ui.kcbSymbolBorderColor->setEnabled(true);
		ui.sbSymbolBorderWidth->setEnabled(true);
	}

	if (m_initializing)
		return;

	QPen pen;
	m_point->beginMacro(i18n("%1 CustomPoints: border style changed", m_pointsList.count()));
	for (auto* point : m_pointsList) {
		pen = point->symbolPen();
		pen.setStyle(penStyle);
		point->setSymbolPen(pen);
	}
	m_point->endMacro();
}

void CustomPointDock::symbolBorderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	m_point->beginMacro(i18n("%1 CustomPoints: border color changed", m_pointsList.count()));
	for (auto* point : m_pointsList) {
		pen = point->symbolPen();
		pen.setColor(color);
		point->setSymbolPen(pen);
	}
	m_point->endMacro();

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, color);
	m_initializing = false;
}

void CustomPointDock::symbolBorderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	m_point->beginMacro(i18n("%1 CustomPoints: border width changed", m_pointsList.count()));
	for (auto* point : m_pointsList) {
		pen = point->symbolPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		point->setSymbolPen(pen);
	}
	m_point->endMacro();
}

//*********************************************************
//**** SLOTs for changes triggered in CustomPoint *********
//*********************************************************
//"General"-tab
void CustomPointDock::pointDescriptionChanged(const AbstractAspect* aspect) {
	if (m_point != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void CustomPointDock::pointPositionChanged(QPointF position) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.lePositionX->setText(numberLocale.toString(position.x()));
	ui.dateTimeEditPositionX->setDateTime(QDateTime::fromMSecsSinceEpoch(position.x()));
	ui.lePositionY->setText(numberLocale.toString(position.y()));
	m_initializing = false;
}

//"Symbol"-tab
void CustomPointDock::pointSymbolStyleChanged(Symbol::Style style) {
	m_initializing = true;
	int index = ui.cbSymbolStyle->findData((int)style);
	ui.cbSymbolStyle->setCurrentIndex(index);
	m_initializing = false;
}

void CustomPointDock::pointSymbolSizeChanged(qreal size) {
	m_initializing = true;
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(size, Worksheet::Unit::Point) );
	m_initializing = false;
}

void CustomPointDock::pointSymbolRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbSymbolRotation->setValue(qRound(angle));
	m_initializing = false;
}

void CustomPointDock::pointSymbolOpacityChanged(qreal opacity) {
	m_initializing = true;
	ui.sbSymbolOpacity->setValue( qRound(opacity*100.0) );
	m_initializing = false;
}

void CustomPointDock::pointSymbolBrushChanged(const QBrush& brush) {
	m_initializing = true;
	ui.cbSymbolFillingStyle->setCurrentIndex((int) brush.style());
	ui.kcbSymbolFillingColor->setColor(brush.color());
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, brush.color());
	m_initializing = false;
}

void CustomPointDock::pointSymbolPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.cbSymbolBorderStyle->setCurrentIndex( (int) pen.style());
	ui.kcbSymbolBorderColor->setColor( pen.color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, pen.color());
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}

void CustomPointDock::pointVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void CustomPointDock::load() {
	if (!m_point)
		return;

	SET_NUMBER_LOCALE
	CartesianPlot* plot = static_cast<CartesianPlot*>(m_point->parent(AspectType::CartesianPlot));
	if (plot->xRangeFormat() == CartesianPlot::RangeFormat::Numeric) {
		ui.lPositionX->show();
		ui.lePositionX->show();
		ui.lPositionXDateTime->hide();
		ui.dateTimeEditPositionX->hide();

		ui.lePositionX->setText(numberLocale.toString(m_point->position().x()));
	} else {
		ui.lPositionX->hide();
		ui.lePositionX->hide();
		ui.lPositionXDateTime->show();
		ui.dateTimeEditPositionX->show();

		ui.dateTimeEditPositionX->setDisplayFormat(plot->xRangeDateTimeFormat());
		ui.dateTimeEditPositionX->setDateTime(QDateTime::fromMSecsSinceEpoch(m_point->position().x()));
	}

	ui.lePositionY->setText(numberLocale.toString(m_point->position().y()));

	int index = ui.cbSymbolStyle->findData((int)m_point->symbolStyle());
	ui.cbSymbolStyle->setCurrentIndex(index);
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(m_point->symbolSize(), Worksheet::Unit::Point) );
	ui.sbSymbolRotation->setValue( m_point->symbolRotationAngle() );
	ui.sbSymbolOpacity->setValue( qRound(m_point->symbolOpacity()*100.0) );

	ui.cbSymbolFillingStyle->setCurrentIndex( (int) m_point->symbolBrush().style() );
	ui.kcbSymbolFillingColor->setColor(  m_point->symbolBrush().color() );

	ui.cbSymbolBorderStyle->setCurrentIndex( (int) m_point->symbolPen().style() );
	ui.kcbSymbolBorderColor->setColor( m_point->symbolPen().color() );
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_point->symbolPen().widthF(), Worksheet::Unit::Point) );

	ui.chkVisible->setChecked( m_point->isVisible() );
}

void CustomPointDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_pointsList.size();
	if (size > 1)
		m_point->beginMacro(i18n("%1 custom points: template \"%2\" loaded", size, name));
	else
		m_point->beginMacro(i18n("%1: template \"%2\" loaded", m_point->name(), name));

	this->loadConfig(config);

	m_point->endMacro();
}

void CustomPointDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "CustomPoint" );

	int index = ui.cbSymbolStyle->findData((int)m_point->symbolStyle());
	ui.cbSymbolStyle->setCurrentIndex(group.readEntry("SymbolStyle", index));
	ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolSize", m_point->symbolSize()), Worksheet::Unit::Point) );
	ui.sbSymbolRotation->setValue( group.readEntry("SymbolRotation", m_point->symbolRotationAngle()) );
	ui.sbSymbolOpacity->setValue( qRound(group.readEntry("SymbolOpacity", m_point->symbolOpacity())*100.0) );
	ui.cbSymbolFillingStyle->setCurrentIndex( group.readEntry("SymbolFillingStyle", (int) m_point->symbolBrush().style()) );
	ui.kcbSymbolFillingColor->setColor(  group.readEntry("SymbolFillingColor", m_point->symbolBrush().color()) );
	ui.cbSymbolBorderStyle->setCurrentIndex( group.readEntry("SymbolBorderStyle", (int) m_point->symbolPen().style()) );
	ui.kcbSymbolBorderColor->setColor( group.readEntry("SymbolBorderColor", m_point->symbolPen().color()) );
	ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("SymbolBorderWidth",m_point->symbolPen().widthF()), Worksheet::Unit::Point) );

	m_initializing = true;
	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, ui.kcbSymbolFillingColor->color());
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, ui.kcbSymbolBorderColor->color());
	m_initializing = false;
}

void CustomPointDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "CustomPoint" );
	group.writeEntry("SymbolStyle", ui.cbSymbolStyle->itemData(ui.cbSymbolStyle->currentIndex()));
	group.writeEntry("SymbolSize", Worksheet::convertToSceneUnits(ui.sbSymbolSize->value(), Worksheet::Unit::Point));
	group.writeEntry("SymbolRotation", ui.sbSymbolRotation->value());
	group.writeEntry("SymbolOpacity", ui.sbSymbolOpacity->value()/100.0);
	group.writeEntry("SymbolFillingStyle", ui.cbSymbolFillingStyle->currentIndex());
	group.writeEntry("SymbolFillingColor", ui.kcbSymbolFillingColor->color());
	group.writeEntry("SymbolBorderStyle", ui.cbSymbolBorderStyle->currentIndex());
	group.writeEntry("SymbolBorderColor", ui.kcbSymbolBorderColor->color());
	group.writeEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(ui.sbSymbolBorderWidth->value(), Worksheet::Unit::Point));
	config.sync();
}
