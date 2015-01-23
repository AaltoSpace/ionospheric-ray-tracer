//============================================================================
// Name        : Application.h
// Author      : Rian van Gijlswijk
// Description : Main application file. Responsible for starting, running and
//				 stopping the simulation
//============================================================================

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <list>
#include "../exporter/Data.h"
#include "../scene/SceneManager.h"

namespace raytracer {
namespace core {

	using namespace scene;
	using namespace exporter;

	class Application {

		public:
			static Application& getInstance() {
				static Application instance;
				return instance;
			}
			void init();
			void start();
			void run();
			void stop();
			SceneManager getSceneManager();
			list<Data> dataSet;

		private:
			Application() {
				isRunning = false;
			}
			Application(Application const&);      // Don't Implement.
			void operator = (Application const&); // Don't implement
			void createScene();
			bool isRunning;
			SceneManager scm;

	};

} /* namespace core */
} /* namespace raytracer */

#endif /* APPLICATION_H_ */
