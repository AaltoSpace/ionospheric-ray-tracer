/*
 * ComplexDouble.h
 *
 *  Created on: 14 Feb 2016
 *      Author: rian
 */

#ifndef MATH_COMPLEXDOUBLE_H_
#define MATH_COMPLEXDOUBLE_H_

#include <cmath>

#include <iostream>
#include <iomanip>

namespace raytracer {
namespace math {

	class ComplexDouble {

		public:
			ComplexDouble();
			ComplexDouble(double r, double i);
			double real() const;
			double imag() const;
			void real(double r);
			void imag(double i);

			friend std::ostream& operator<<(std::ostream &strm, const raytracer::math::ComplexDouble &v) {
				return strm << std::fixed << std::setprecision(4)
					<< v.real() << "+" << v.imag() << "i";
			}

		private:
			double realNumber;
			double imaginaryNumber;

	};

} /* namespace math */
} /* namespace raytracer */

#endif /* MATH_COMPLEXDOUBLE_H_ */
