/*
 * ComplexDouble.cpp
 *
 *  Created on: 14 Feb 2016
 *      Author: rian
 */

#include <cmath>
#include "ComplexDouble.h"

namespace raytracer {
namespace math {

	ComplexDouble::ComplexDouble() {}

	ComplexDouble::ComplexDouble(double r, double i) {
		realNumber = r;
		imaginaryNumber = i;
	}

	double ComplexDouble::real() {
		return realNumber;
	}

	void ComplexDouble::real(double r) {
		realNumber = r;
	}

	double ComplexDouble::imag() {
		return imaginaryNumber;
	}

	void ComplexDouble::imag(double i) {
		imaginaryNumber = i;
	}

} /* namespace math */
} /* namespace raytracer */
