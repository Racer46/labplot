
/***************************************************************************
    File                 : CartesianScale.cpp
    Project              : LabPlot
    Description          : Cartesian coordinate system for plots.
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 by Alexander Semke (alexander.semke@web.de)

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

#include "CartesianScale.h"

/**
 * \class CartesianScale
 * \brief Base class for cartesian coordinate system scales.
 */
CartesianScale::CartesianScale(Type type, const Interval<double> &interval, double a, double b, double c)
	: m_type(type), m_interval(interval), m_a(a), m_b(b), m_c(c) {
}

CartesianScale::~CartesianScale() = default;

void CartesianScale::getProperties(Type *type, Interval<double> *interval,
		double *a, double *b, double *c) const {
	if (type)
		*type = m_type;
	if (interval)
		*interval = m_interval;
	if (a)
		*a = m_a;
	if (b)
		*b = m_b;
	if (c)
		*c = m_c;
}

/**
 * \class CartesianCoordinateSystem::LinearScale
 * \brief implementation of the linear scale for cartesian coordinate system.
 */
class LinearScale : public CartesianScale {
public:
	LinearScale(const Interval<double> &interval, double offset, double gradient)
		: CartesianScale(Type::Linear, interval, offset, gradient, 0) {
			Q_ASSERT(gradient != 0.0);

		}

	~LinearScale() override = default;

	bool map(double *value) const override {
		*value = *value * m_b + m_a;
		return true;
	}

	bool inverseMap(double *value) const override {
		if (m_b == 0.0)
			return false;
		*value = (*value - m_a) / m_b;
		return true;
	}

	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}
};

/**
 * \class CartesianCoordinateSystem::LinearScale
 * \brief implementation of the linear scale for cartesian coordinate system.
 */
class LogScale : public CartesianScale {
public:
	LogScale(const Interval<double> &interval, double offset, double scaleFactor, double base, bool abs)
		: CartesianScale(Type::Log, interval, offset, scaleFactor, base), m_abs(abs) {
			Q_ASSERT(scaleFactor != 0.0);
			Q_ASSERT(base > 0.0);
	}

	~LogScale() override = default;

	bool map(double *value) const override {
		if (*value > 0.0)
			*value = log(*value)/log(m_c) * m_b + m_a;
		else if (m_abs)
			*value = log(fabs(*value))/log(m_c) * m_b + m_a;
		else
			return false;

		return true;
	}

	bool inverseMap(double *value) const override {
		if (m_a == 0.0)
			return false;
		if (m_c <= 0.0)
			return false;

		*value = pow(m_c, (*value - m_a) / m_b);
		return true;
	}
	int direction() const override {
		return m_b < 0 ? -1 : 1;
	}

private:
	bool m_abs;
};


/***************************************************************/

CartesianScale* CartesianScale::createLinearScale(const Interval<double> &interval,
		double sceneStart, double sceneEnd, double logicalStart, double logicalEnd) {

	double lDiff = logicalEnd - logicalStart;
	if (lDiff == 0.0)
		return nullptr;

	double b = (sceneEnd - sceneStart) / lDiff;
	double a = sceneStart - b * logicalStart;

	return new LinearScale(interval, a, b);
}

CartesianScale* CartesianScale::createLogScale(const Interval<double> &interval,
		double sceneStart, double sceneEnd, double logicalStart, double logicalEnd, CartesianPlot::Scale type) {

	double base;
	if (type == CartesianPlot::Scale::Log10 || type == CartesianPlot::Scale::Log10Abs)
		base = 10.0;
	else if (type == CartesianPlot::Scale::Log2 || type == CartesianPlot::Scale::Log2Abs)
		base = 2.0;
	else
		base = M_E;


	if (logicalStart <= 0.0 || logicalEnd <= 0.0)
		return nullptr;

	const double lDiff = (log(logicalEnd) - log(logicalStart)) / log(base);
	if (lDiff == 0.0)
		return nullptr;

	double b = (sceneEnd - sceneStart) / lDiff;
	double a = sceneStart - b * log(logicalStart)/log(base);

	bool abs = (type == CartesianPlot::Scale::Log10Abs || type == CartesianPlot::Scale::Log2Abs || type == CartesianPlot::Scale::LnAbs);
	return new LogScale(interval, a, b, base, abs);
}

