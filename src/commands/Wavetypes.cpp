/*
 * Wavetypes.cpp
 *
 *  Created on: 15 Mar 2016
 *      Author: rian
 */

#include "Wavetypes.h"
#include "../../src/core/Application.h"
#include "../../src/scene/Ionosphere.h"
#include "../../src/tracer/Ray.h"
#include "../../src/math/Constants.h"
#include "../../src/core/Config.h"
#include "../../src/exporter/MagneticFieldExporter.h"

namespace raytracer {
namespace commands {

	using namespace raytracer::scene;
	using namespace raytracer::tracer;
	using namespace raytracer::exporter;
	using namespace raytracer::math;
	using namespace raytracer::core;

	Wavetypes::Wavetypes() {
		// TODO Auto-generated constructor stub
	}

	void Wavetypes::start() {
		appConf = Config("config/config_nomagneticfield.json");
		conf = Config("config/scenario_test.json");
		Application::getInstance().setApplicationConfig(appConf);
		Application::getInstance().setCelestialConfig(conf);
	}

	void Wavetypes::run() {

		simulateOWaves();
		simulateXWaves();

	}

	void Wavetypes::stop() {

	}

	void Wavetypes::simulateOWaves() {

		list<Data> dataSet;
		Ionosphere io2;
		Ray r;
		int increment = 1;
		int MAX = 30e6;
		double PLASMA_FREQUENCY = 2.8e7;
		io2.setElectronNumberDensity(0);

		for (int f = 0; f < MAX; f += increment) {
			r.frequency = f;
			Vector2d result = io2.getRefractiveIndexSquaredAHDR(&r, PLASMA_FREQUENCY);
			Data d;
			d.frequency = f;
			d.omega_p = PLASMA_FREQUENCY;
			d.n = result.x;
			dataSet.push_back(d);

			if ((f % (increment*512)) == 0) {
				increment *= 2;
			}
		}

		MagneticFieldExporter mfe;
		mfe.dump("Debug/data_IonosphereO_WaveTest.dat", dataSet);
	}

	void Wavetypes::simulateXWaves() {
		appConf = Config("config/config_simplemagneticfield.json");
		Application::getInstance().setApplicationConfig(appConf);

		Ionosphere io3;
		list<Data> dataSet, dataSet2;
		Ray r;
		int increment = 1;
		int MAX = 30e6;
		double PLASMA_FREQUENCY = 2.8e7;
		io3.angleToMagField = Constants::PI/2.0;
		io3.setElectronNumberDensity(0);

		for (int f = 0; f < MAX; f += increment) {
			r.frequency = f;
			Vector2d result = io3.getRefractiveIndexSquaredAHDR(&r, PLASMA_FREQUENCY);
			Data d, d2;
			d.frequency = f;
			d.omega_p = PLASMA_FREQUENCY;
			d.n = result.x;
			dataSet.push_back(d);
			d2.frequency = f;
			d2.omega_p = PLASMA_FREQUENCY;
			d2.n = result.y;
			dataSet2.push_back(d2);

			if ((f % (increment*512)) == 0) {
				increment *= 2;
			}
		}

		MagneticFieldExporter mfe;
		mfe.dump("Debug/data_IonosphereX_WaveTest_1.dat", dataSet);
		mfe.dump("Debug/data_IonosphereX_WaveTest_2.dat", dataSet2);
	}

} /* namespace commands */
} /* namespace raytracer */
