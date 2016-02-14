/*
 * ComplexNumberHelper.cpp
 *
 *  Created on: 14 Feb 2016
 *      Author: rian
 */

#include "ComplexNumberHelper.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

namespace raytracer {
namespace math {

	/**
	 * Calculate the complex square root of two doubles
	 */
	ComplexDouble ComplexNumberHelper::complexSquareRoot(double a, double b) {

		double real = sqrt( (sqrt(pow(a, 2) + pow(b, 2)) + a) / 2.0 );
		double imag = sqrt( (sqrt(pow(a, 2) + pow(b, 2)) - a) / 2 );

		if (std::isnan(real)) {
			BOOST_LOG_TRIVIAL(error) << "No real value found, all values are imaginary!";
		}

		return ComplexDouble(real, imag);
	}

} /* namespace math */
} /* namespace raytracer */
