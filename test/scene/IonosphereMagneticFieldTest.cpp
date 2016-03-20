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
	 * X wave: extraordinary wave. Simulate the index of refraction as a function of
	 * wave frequency.
	 */
	TEST_F(IonosphereMagneticFieldTest, X_WaveExportTest) {


	}
}
