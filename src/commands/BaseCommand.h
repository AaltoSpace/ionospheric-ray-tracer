/*
 * BaseCommand.h
 *
 *  Created on: 15 Mar 2016
 *      Author: rian
 */

#ifndef CORE_COMMANDS_BASECOMMAND_H_
#define CORE_COMMANDS_BASECOMMAND_H_

namespace raytracer {
namespace commands {

	class BaseCommand {

		public:
			BaseCommand () {}
			virtual ~BaseCommand() {}
			virtual void start() = 0;
			virtual void run() = 0;
			virtual void stop() = 0;
	};

} /* namespace commands */
} /* namespace raytracer */



#endif /* CORE_COMMANDS_BASECOMMAND_H_ */
