/*
 * Simulation.h
 *
 *  Created on: 15 Mar 2016
 *      Author: rian
 */

#ifndef CORE_COMMANDS_SIMULATION_H_
#define CORE_COMMANDS_SIMULATION_H_

#include "BaseCommand.h"

namespace raytracer {
namespace commands {

	class Simulation : public BaseCommand {

		public:
			static const int cmdId;

			static BaseCommand* createDerived() {
				return new Simulation;
			}
			static void RegisterDerivedClass(void);

			Simulation();
			void start();
			void run();
			void stop();
	};

} /* namespace threading */
} /* namespace raytracer */

#endif /* CORE_COMMANDS_SIMULATION_H_ */
