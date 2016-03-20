/*
 * Wavetypes.h
 *
 *  Created on: 15 Mar 2016
 *      Author: rian
 */

#ifndef CORE_COMMANDS_WAVETYPES_H_
#define CORE_COMMANDS_WAVETYPES_H_

#include "BaseCommand.h"
#include "../core/Config.h"

namespace raytracer {
namespace commands {

	class Wavetypes : public BaseCommand {

		public:
			Wavetypes();
			void start();
			void run();
			void stop();

		private:
			core::Config conf, appConf;
			void simulateOWaves();
			void simulateXWaves();
	};

} /* namespace commands */
} /* namespace raytracer */

#endif /* CORE_COMMANDS_WAVETYPES_H_ */
