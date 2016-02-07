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
			}

			Config conf, appConf;
	};

	TEST_F(IonosphereMagneticFieldTest, VacuumTest) {

		Ionosphere io;
		Ray r;
		r.frequency = 1;

		io.setCollisionFrequency(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, 0);

		ASSERT_EQ(0, nSquared);
	}

	TEST_F(IonosphereMagneticFieldTest, NonMagnetizedNonCollisional) {

		Ionosphere io;
		Ray r;
		r.frequency = 1;
		double PLASMA_FREQUENCY = 2.8e7;

		io.setCollisionFrequency(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_EQ(0, nSquared);
	}

	TEST_F(IonosphereMagneticFieldTest, NonMagnetizedNonCollisional2) {

		Ionosphere io;
		Ray r;
		double PLASMA_FREQUENCY = 2.8e7;
		r.frequency = 4.5e6; // slightly above plasma frequency, so that X < 1

		io.setCollisionFrequency(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_EQ(0, nSquared);
	}

	/**
	 * O wave: normal wave. Simulate the index of refraction as a function of
	 * wave frequency.
	 */
	TEST_F(IonosphereMagneticFieldTest, O_WaveTest) {

		list<Data> dataSet;
		Ionosphere io;
		Ray r;
		int increment = 1;
		int MAX = 100e6;
		double PLASMA_FREQUENCY = 2.8e7;

		for (int f = 0; f < MAX; f += increment) {
			Data d;
			r.frequency = f;
			d.frequency = f;
			d.omega_p = PLASMA_FREQUENCY;
			d.n = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_KELSO, PLASMA_FREQUENCY);
			dataSet.push_back(d);

			if ((f % (increment*10)) == 0) {
				increment *= 10;
			}
		}

		MagneticFieldExporter mfe;
		mfe.dump("Debug/data_IonosphereO_WaveTest.dat", dataSet);
	}

	/**
	 * X wave: extraordinary wave. Simulate the index of refraction as a function of
	 * wave frequency.
	 */
	TEST_F(IonosphereMagneticFieldTest, X_WaveTest) {

		list<Data> dataSet;
		Ionosphere io;
		Ray r;
		int increment = 1;
		int MAX = 100e6;
		double PLASMA_FREQUENCY = 2.8e7;

		for (int f = 0; f < MAX; f += increment) {
			Data d;
			r.frequency = f;
			d.frequency = f;
			d.omega_p = PLASMA_FREQUENCY;
			d.n = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);
			dataSet.push_back(d);

			if ((f % (increment*10)) == 0) {
				increment *= 10;
			}
		}

		MagneticFieldExporter mfe;
		mfe.dump("Debug/data_IonosphereX_WaveTest.dat", dataSet);
	}
}
