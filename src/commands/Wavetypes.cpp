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
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

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

		BOOST_LOG_TRIVIAL(info) << "Starting \"Wavetypes\" program";

		conf = Config("config/scenario_test.json");
		Application::getInstance().setCelestialConfig(conf);
	}

	void Wavetypes::run() {

		simulateOWaves();
		simulateXWaves();

	}

	void Wavetypes::stop() {

	}

	void Wavetypes::simulateOWaves() {

		BOOST_LOG_TRIVIAL(info) << "Simulating O-waves";

		appConf = Config("config/config_nomagneticfield.json");
		Application::getInstance().setApplicationConfig(appConf);

		list<Data> dataSet;
		Ionosphere io2;
		Ray r;
		int increment = 1;
		int MAX = 30e6;
		double PLASMA_FREQUENCY = 2.8e7;
		io2.setCollisionFrequency(0);
		io2.setElectronNumberDensity(0);

		for (int f = 0; f < MAX; f += increment) {
			r.frequency = f;
			Vector2d result = io2.getRefractiveIndexSquaredAHDR(&r, 0, PLASMA_FREQUENCY);
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
		int dataSetSize = dataSet.size();
		mfe.dump("data_IonosphereO_WaveTest.dat", dataSet);

		BOOST_LOG_TRIVIAL(info) << "Simulation completed. " << dataSetSize << " datapoints generated.";
	}

	void Wavetypes::simulateXWaves() {

		BOOST_LOG_TRIVIAL(info) << "Simulating X-waves";

		appConf = Config("config/config_simplemagneticfield.json");
		Application::getInstance().setApplicationConfig(appConf);

		Ionosphere io3;
		list<Data> dataSet, dataSet2;
		Ray r;
		int increment = 1;
		int MAX = 30e6;
		double PLASMA_FREQUENCY = 2.8e7;
		double angleToMagField = Constants::PI/2.0;

		io3.setCollisionFrequency(0);
		io3.setElectronNumberDensity(0);

		for (int f = 0; f < MAX; f += increment) {
			r.frequency = f;
			Vector2d result = io3.getRefractiveIndexSquaredAHDR(&r, angleToMagField, PLASMA_FREQUENCY);
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
		int dataSetSize = dataSet.size() + dataSet2.size();
		mfe.dump("data_IonosphereX_WaveTest_1.dat", dataSet);
		mfe.dump("data_IonosphereX_WaveTest_2.dat", dataSet2);

		BOOST_LOG_TRIVIAL(info) << "Simulation completed. " << dataSetSize << " datapoints generated.";
	}

} /* namespace commands */
} /* namespace raytracer */
