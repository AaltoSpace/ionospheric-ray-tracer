/*
 * IsotropicAntenna.cpp
 *
 *  Created on: 27 Jul 2015
 *      Author: rian
 */

#include "IsotropicAntenna.h"

namespace raytracer {
namespace radio {

	IsotropicAntenna::IsotropicAntenna() {

	}

	void IsotropicAntenna::setConfig(const Json::Value conf) {

		_nominalSignalPower = conf.get("nominalSignalPower", "").asDouble();
	}

	double IsotropicAntenna::getSignalPowerAt(double azimuth, double elevation) {

		return _nominalSignalPower;
	}


} /* namespace radio */
} /* namespace raytracer */
