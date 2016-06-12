#include <cmath>
#include "../../contrib/jsoncpp/value.h"
#include "../../src/radio/AntennaFactory.h"
#include "../../src/math/Constants.h"
#include "gtest/gtest.h"

namespace {

	using namespace raytracer::radio;

	class AntennaFactoryTest : public ::testing::Test {

		protected:
			void setUp() {}
	};

	TEST_F(AntennaFactoryTest, NonExistingAntenna) {

		EXPECT_DEATH(AntennaFactory::Create("FubarAntenna"),
				"Instance type FubarAntenna does not exist!");
	}

	TEST_F(AntennaFactoryTest, IsotropicAntenna) {

		IAntenna* ant = AntennaFactory::Create("IsotropicAntenna");

		Json::Value config;

		config["type"] = "IsotropicAntenna";
		config["nominalSignalPower"] = 1.0;
		ant->setConfig(config);
		const double RAD2DEG = raytracer::math::Constants::PI / 180.0;

		ASSERT_NEAR(1.0, ant->getSignalPowerAt(0, 0), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(30*RAD2DEG, 0), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(0, 30*RAD2DEG), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(30*RAD2DEG, 30*RAD2DEG), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(60*RAD2DEG, 0), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(0, 60*RAD2DEG), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(90*RAD2DEG, 0), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(0, 90*RAD2DEG), 1e-3);
		ASSERT_NEAR(1.0, ant->getSignalPowerAt(90*RAD2DEG, 90*RAD2DEG), 1e-3);

	}

	TEST_F(AntennaFactoryTest, ShortDipoleAntenna) {

			IAntenna* ant = AntennaFactory::Create("ShortDipoleAntenna");

			Json::Value config;

			config["type"] = "IsotropicAntenna";
			config["nominalSignalPower"] = 1.0;
			ant->setConfig(config);
			const double RAD2DEG = raytracer::math::Constants::PI / 180.0;

			ASSERT_NEAR(1.760, ant->getSignalPowerAt(0, 0), 1e-3);
			ASSERT_NEAR(1.760, ant->getSignalPowerAt(30*RAD2DEG, 0), 1e-3);
			ASSERT_NEAR(1.320, ant->getSignalPowerAt(0, 30*RAD2DEG), 1e-3);
			ASSERT_NEAR(1.320, ant->getSignalPowerAt(30*RAD2DEG, 30*RAD2DEG), 1e-3);
			ASSERT_NEAR(1.760, ant->getSignalPowerAt(60*RAD2DEG, 0), 1e-3);
			ASSERT_NEAR(0.440, ant->getSignalPowerAt(0, 60*RAD2DEG), 1e-3);
			ASSERT_NEAR(1.760, ant->getSignalPowerAt(90*RAD2DEG, 0), 1e-3);
			ASSERT_NEAR(0.0, ant->getSignalPowerAt(0, 90*RAD2DEG), 1e-3);
			ASSERT_NEAR(0.0, ant->getSignalPowerAt(90*RAD2DEG, 90*RAD2DEG), 1e-3);

		}
}
