/*
 * ComplexDouble.h
 *
 *  Created on: 14 Feb 2016
 *      Author: rian
 */

#ifndef MATH_COMPLEXDOUBLE_H_
#define MATH_COMPLEXDOUBLE_H_

#include <cmath>

namespace raytracer {
namespace math {

	class ComplexDouble {

		public:
			ComplexDouble();
			ComplexDouble(double r, double i);
			double real();
			double imag();
			void real(double r);
			void imag(double i);

		private:
			double realNumber;
			double imaginaryNumber;

	};

} /* namespace math */
} /* namespace raytracer */

#endif /* MATH_COMPLEXDOUBLE_H_ */
