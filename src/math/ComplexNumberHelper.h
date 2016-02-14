/*
 * ComplexNumberHelper.h
 *
 *  Created on: 14 Feb 2016
 *      Author: rian
 */

#ifndef MATH_COMPLEXNUMBERHELPER_H_
#define MATH_COMPLEXNUMBERHELPER_H_

#include "ComplexDouble.h"

namespace raytracer {
namespace math {

	class ComplexNumberHelper {

		public:
			static ComplexNumberHelper& getInstance() {
				static ComplexNumberHelper instance;
				return instance;
			}
			/**
			 * Calculate the complex square root of two doubles
			 */
			ComplexDouble complexSquareRoot(double a, double b);

		private:
			ComplexNumberHelper() {}
			ComplexNumberHelper(ComplexNumberHelper const&);      // Don't Implement.
	};

} /* namespace math */
} /* namespace raytracer */

#endif /* MATH_COMPLEXNUMBERHELPER_H_ */
