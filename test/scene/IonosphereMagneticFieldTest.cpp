#include "gtest/gtest.h"
#include "../../src/core/Application.h"
#include "../../src/scene/Ionosphere.h"
#include "../../src/tracer/Ray.h"
#include "../../src/math/Constants.h"
#include "../../src/core/Config.h"
#include "../../src/exporter/MagneticFieldExporter.h"

namespace {

	using namespace raytracer::scene;
	using namespace raytracer::tracer;
	using namespace raytracer::exporter;
	using namespace raytracer::math;
	using namespace raytracer::core;

	class IonosphereMagneticFieldTest : public ::testing::Test {

		protected:
			void SetUp() {

				appConf = Config("config/config_nomagneticfield.json");
				conf = Config("config/scenario_test.json");
				Application::getInstance().setApplicationConfig(appConf);
				Application::getInstance().setCelestialConfig(conf);

				io.setup();
				io.layerHeight = 1000;
				io.superimposeElectronNumberDensity(2.5e11, 125e3, 11.1e3);
			}

			Config conf, appConf;
			Ionosphere io;
	};

	TEST_F(IonosphereMagneticFieldTest, VacuumTest) {

		Ray r;
		r.frequency = 1;

		io.setCollisionFrequency(0);
		io.setElectronNumberDensity(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, 0);

		ASSERT_EQ(1, nSquared);
	}

	TEST_F(IonosphereMagneticFieldTest, NonMagnetizedNonCollisional) {

		Ray r;
		double PLASMA_FREQUENCY = 2.8e7;
		r.frequency = PLASMA_FREQUENCY / (2 * Constants::PI)+1;

		io.setCollisionFrequency(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_NEAR(0, nSquared, 1e-3);

		r.frequency = 1;

		nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_LT(0, nSquared);
	}

	TEST_F(IonosphereMagneticFieldTest, NonMagnetizedNonCollisional2) {

		Ray r;
		double PLASMA_FREQUENCY = 2.8e7;
		r.frequency = 4.5e6; // slightly above plasma frequency, so that X < 1

		io.setCollisionFrequency(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_GT(nSquared, 0);
		ASSERT_LT(nSquared, 1);
	}

	/**
	 * O wave: normal wave. Simulate the index of refraction as a function of
	 * wave frequency.
	 */
	TEST_F(IonosphereMagneticFieldTest, O_WaveExportTest) {

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

			if ((f % (increment*128)) == 0) {
				increment *= 2;
			}
		}

		MagneticFieldExporter mfe;
		mfe.dump("Debug/data_IonosphereO_WaveTest.dat", dataSet);
	}

	/**
	 * X wave: extraordinary wave. Simulate the index of refraction as a function of
	 * wave frequency.
	 */
	TEST_F(IonosphereMagneticFieldTest, X_WaveExportTest) {

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

			if ((f % (increment*128)) == 0) {
				increment *= 2;
			}
		}

		MagneticFieldExporter mfe;
		mfe.dump("Debug/data_IonosphereX_WaveTest_1.dat", dataSet);
		mfe.dump("Debug/data_IonosphereX_WaveTest_2.dat", dataSet2);
	}
}
