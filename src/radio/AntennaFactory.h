/*
 * AntennaFactory.h
 *
 *  Created on: 27 Jul 2015
 *      Author: rian
 */

#ifndef RADIO_ANTENNAFACTORY_H_
#define RADIO_ANTENNAFACTORY_H_

#include <iostream>
#include <string>
#include <map>
#include "IAntenna.h"
#include "IsotropicAntenna.h"
#include "ShortDipoleAntenna.h"

namespace raytracer {
namespace radio {

	class AntennaFactory {

		public:
			static IAntenna* Create(const char* objType) {
				if (objType == "IsotropicAntenna") {
					return new IsotropicAntenna;
				} else if (objType == "ShortDipoleAntenna") {
					return new ShortDipoleAntenna;
				} else {
					std::cerr << "Instance type " << objType << " does not exist!";
					std::exit(-1);
				}
			}
	};

} /* namespace radio */
} /* namespace raytracer */

#endif /* RADIO_ANTENNAFACTORY_H_ */
