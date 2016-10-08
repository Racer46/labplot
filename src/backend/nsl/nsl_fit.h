/***************************************************************************
    File                 : nsl_fit.h
    Project              : LabPlot
    Description          : NSL (non)linear fitting functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef NSL_FIT_H
#define NSL_FIT_H

/* convert unbounded variable x to bounded variable where bounds are [min, max] */
double nsl_fit_map_bound(double x, double min, double max);
/* convert bounded variable x to unbounded variable where bounds are [min, max] */
double nsl_fit_map_unbound(double x, double min, double max);

#endif /* NSL_FIT_H */
