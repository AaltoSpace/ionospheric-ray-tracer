/*
 * IsotropicAntenna.h
 *
 *  Created on: 27 Jul 2015
 *      Author: rian
 */

#ifndef RADIO_ISOTROPICANTENNA_H_
#define RADIO_ISOTROPICANTENNA_H_

#include "IAntenna.h"

namespace raytracer {
namespace radio {

	class IsotropicAntenna : public IAntenna {

		public:
			IsotropicAntenna();
			~IsotropicAntenna() {}
			void setConfig(const Json::Value conf);
			double getSignalPowerAt(double azimuth, double elevation);
	};


} /* namespace radio */
} /* namespace raytracer */

#endif /* RADIO_ISOTROPICANTENNA_H_ */
