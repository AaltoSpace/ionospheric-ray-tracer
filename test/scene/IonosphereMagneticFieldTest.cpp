#include "gtest/gtest.h"
#include "../../src/core/Application.h"
#include "../../src/scene/Ionosphere.h"
#include "../../src/tracer/Ray.h"
#include "../../src/math/Constants.h"
#include "../../src/math/Vector2d.h"
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

	TEST_F(IonosphereMagneticFieldTest, ConfigNoMagnetismTest) {

		Config appConf2 = Config("config/config_nomagneticfield.json");
		Application::getInstance().setApplicationConfig(appConf2);
		Json::Value magFields = Application::getInstance().getApplicationConfig()
													.getArray("magneticFields");

		ASSERT_EQ(0, magFields.size());
		ASSERT_TRUE(magFields.isArray());
	}

	TEST_F(IonosphereMagneticFieldTest, ConfigWithMagnetismTest) {

		Config appConf2 = Config("config/config_simplemagneticfield.json");
		Application::getInstance().setApplicationConfig(appConf2);
		Json::Value magFields = Application::getInstance().getApplicationConfig()
													.getArray("magneticFields");

		double BTot = magFields[0].get("strength", "").asDouble();
		Vector3d k = Config::getVector3dFromObject(magFields[0].get("direction", ""));

		ASSERT_TRUE(magFields.isArray());
		ASSERT_EQ(1, magFields.size());
		ASSERT_NEAR(0.00005, BTot, 1e-7);
		ASSERT_EQ(0, k.x);
		ASSERT_EQ(0, k.y);
		ASSERT_EQ(1, k.z);
	}

	/**
	 * n^2 reduces to:
	 * n^2 = sqrt(1 - X)
	 * X = w_pe^2 / w^2 --> Inf
	 * Thus, complex n^2:
	 * n^2 = 0 - w_pe j
	 * Result of test: note that only real values are returned, not imaginary ones
	 */
	TEST_F(IonosphereMagneticFieldTest, VacuumTest) {

		Ray r;
		r.frequency = 1;
		double PLASMA_FREQUENCY = 2.8e7;

		io.setCollisionFrequency(0);
		io.setElectronNumberDensity(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);
		Vector2d complexNSquared = io.getRefractiveIndexSquaredAHDR(&r, 0, PLASMA_FREQUENCY);

		ASSERT_EQ(0, nSquared);
		ASSERT_EQ(0, complexNSquared.x);
		ASSERT_EQ(0, complexNSquared.y);
	}

	TEST_F(IonosphereMagneticFieldTest, NonMagnetizedNonCollisional) {

		Ray r;
		double PLASMA_FREQUENCY = 2.8e7;
		r.frequency = PLASMA_FREQUENCY / (2 * Constants::PI*1.01); // slightly below plasma frequency

		io.setCollisionFrequency(0);

		double nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_NEAR(0, nSquared, 1e-3);

		r.frequency = 1;

		nSquared = io.getRefractiveIndexSquared(&r, Ionosphere::REFRACTION_AHDR, PLASMA_FREQUENCY);

		ASSERT_EQ(0, nSquared); // function does not return negative numbers, but 0 if n^2 < 0
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
}
