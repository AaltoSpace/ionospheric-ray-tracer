/*
 * Ionosphere.cpp
 *
 *  Created on: 22 Jan 2015
 *      Author: rian
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include "Ionosphere.h"
#include "../math/Constants.h"
#include "../exporter/Data.h"
#include "../core/Application.h"
#include "../core/Config.h"
#include "../math/NormalDistribution.h"

namespace raytracer {
namespace scene {

	using namespace std;
	using namespace exporter;
	using namespace core;

	Ionosphere::Ionosphere() {}

	Ionosphere::Ionosphere(Geometry g) {
		geom = g;
	}

	/**
	 * Precalculate fixed values for this ionospheric layer.
	 */
	void Ionosphere::setup() {

		_altitude = getAltitude();
		_peakProductionAltitude = NormalDistribution::getInstance().get(Ionosphere::peakProductionAltitude, 1);
		_electronPeakDensity = NormalDistribution::getInstance().get(Ionosphere::electronPeakDensity,
				electronDensityVariability);
	}

	/**
	 * Interaction between ray and ionospheric layer
	 */
	void Ionosphere::interact(Ray *r, Vector2d &hitpos) {

		setup();

		double magnitude = r->o.distance(hitpos);

		refract(r, hitpos);

		attenuate(r, magnitude);

		exportData(r);
	}

	void Ionosphere::refract(Ray *r, Vector2d &hitpos) {

		double refractiveIndex = getRefractiveIndex(r, Ionosphere::REFRACTION_KELSO);
		double SZA = geom.getSolarZenithAngle2d();
		double beta = atan(r->d.y/r->d.x);
		double theta_i = getIncidentAngle(r);
		int waveBehaviour = determineWaveBehaviour(r);

		if (waveBehaviour == Ray::wave_reflection) {
			r->behaviour = Ray::wave_reflection;
			double beta_r = - beta - 2*SZA;
			r->setAngle(beta_r);
//			cout << "Reflect this ray! beta_r:" << beta_r * 180 / Constants::PI << " d.x,d.y:" << r->d.x << "," << r->d.y << "\n";
		} else if (waveBehaviour == Ray::wave_refraction) {
			if (r->d.y < 0) {
				theta_i = Constants::PI - theta_i;
			}
			r->behaviour = Ray::wave_refraction;
			double theta_r = asin((r->previousRefractiveIndex/refractiveIndex * sin(theta_i)));
			if (theta_r > 0.9 * Constants::PI/2) {
//				cerr << "Bend this ray! refraction: theta_i:" << theta_i * 180 / Constants::PI << ", theta_r:" << theta_r * 180 / Constants::PI << endl;
			}
			double beta_2 = Constants::PI/2 - theta_r - SZA;
			if (r->d.y < 0) {
//				cerr << "Ray going down! r->d.y:" << r->d.y << ", beta_2:" << beta_2;
				beta_2 = -Constants::PI/2 + theta_r - SZA;
				r->setAngle(beta_2);
//				cerr << " changed to r->d.y:" << r->d.y << ",beta_2:" << beta_2 << endl;
			}
			r->setAngle(beta_2);
//			cout << "Bend this ray! refraction: " << refractiveIndex << " theta_i:" << theta_i * 180 / Constants::PI << ", theta_r:" << theta_r * 180 / Constants::PI << " \n";
		} else if (waveBehaviour == Ray::wave_none) {
			r->behaviour = Ray::wave_none;
			cout << "Ray goes straight!\n";
		} else {
			cerr << "No idea what to do with this ray!";
		}
		r->o.x = hitpos.x;
		r->o.y = hitpos.y;
		r->previousRefractiveIndex = refractiveIndex;
	}

	/**
	 * A radio wave is attenuated by its propagation through the ionosphere.
	 * The attenuation calculation is based on Nielsen, 2007. Attenuation depends
	 * on the electron-neutral collision frequency, the altitude w.r.t the peak
	 * production altitude and the radio wave frequency. The result is in db/m
	 * and thus is internally converted to attenuation in db, taking the layer
	 * height and the ray angle into account.
	 * A ray with a higher SZA will travel a longer path through the layer and
	 * thus face more attenuation.
	 */
	void Ionosphere::attenuate(Ray *r, double magnitude) {

		double theta_r = r->getAngle() - Constants::PI/2 + geom.getSolarZenithAngle2d();
		if (r->d.y < 0) {
			theta_r = - r->getAngle() - Constants::PI/2 + geom.getSolarZenithAngle2d();
		}

		double mu_r = sqrt(1 - pow(getPlasmaFrequency(), 2) / pow(2 * Constants::PI * r->frequency, 2));

		double ki = (-pow(getPlasmaFrequency(), 2) / (2 * Constants::C * mu_r))
				* getCollisionFrequency() / (pow(2 * Constants::PI * r->frequency, 2) + pow(getCollisionFrequency(), 2));

//		printf("wp: %4.2e, f: %4.2e, mu_r: %4.2e, colFreq: %4.2e, ki: %4.2e ", getPlasmaFrequency(), r->frequency, mu_r, getCollisionFrequency(), ki);

		double loss = 20 * log10(exp(1)) * ki * magnitude * abs(1 / cos(geom.getSolarZenithAngle2d())) * cos(abs(theta_r));

//		printf("magnitude: %4.2f, totalLoss: %4.2e, theta: %4.2f, alt: %4.2f \n", magnitude, r->signalPower, theta_r, _altitude());

//		printf("Loss: %4.2e ", loss);

		r->signalPower += loss;
	}

	/**
	 * A radio wave is attenuated by its propagation through the ionosphere.
	 * The attenuation calculation is based on Withers, 2011. The ray frequencies
	 * are considered intermediate frequencies with neither rayFreq << plasmaFreq and
	 * plasmaFreq << rayFreq. Thus, the altitude w.r.t scale height depends what
	 * attenuation factor (A) needs to be used.
	 * Possible cases:
	 *  A = P_lo if (z0 - zl)/H < -1
	 *  A = minimum[P_lo, P_hi] if (z0 - zl)/H > -1
	 * Subsequently: P = P_t / A where P is the resulting power
	 */
	void Ionosphere::attenuateWithers(Ray *r) {

		double zL = Constants::NEUTRAL_SCALE_HEIGHT * log(Ionosphere::surfaceCollisionFrequency
				/ (2 * Constants::PI * r->frequency));

		printf("zL: %4.2e", zL);

		double pLowDb = 8.69 * (1/cos(geom.getSolarZenithAngle2d())) * ((Constants::NEUTRAL_SCALE_HEIGHT * getElectronPeakDensity())
				/(2 * Constants::ELECTRON_MASS * Constants::C * Constants::PERMITTIVITY_VACUUM * r->frequency * 2 *Constants::PI));

		pLowDb = 7.3e-10 * (1/cos(geom.getSolarZenithAngle2d())) * Constants::NEUTRAL_SCALE_HEIGHT * 1e9 / r->frequency;
		double pLowW =  pow(10, pLowDb/10);
		printf("pLowDb: %4.2e, pLowW: %4.2e ", pLowDb, pLowW);

		if (_peakProductionAltitude < zL - Constants::NEUTRAL_SCALE_HEIGHT) {
			r->signalPower /= pLowDb;
		} else {
			double pHigh = 8.69 * (1/cos(geom.getSolarZenithAngle2d())) * ((Constants::NEUTRAL_SCALE_HEIGHT *
					_peakProductionAltitude * pow(Constants::ELEMENTARY_CHARGE, 2) * Ionosphere::surfaceCollisionFrequency)
					/(2 * Constants::ELECTRON_MASS * Constants::C * pow(2 * Constants::PI * r->frequency,2)));
			printf("pHigh: %4.2e, ", pHigh);
			if (pLowDb < pHigh) r->signalPower /= pLowW;
			else r->signalPower /= pHigh;
		}
	}

	void Ionosphere::exportData(Ray *r) {

		Data d;
		d.x = r->o.x;
		d.y = r->o.y;
		d.rayNumber = r->rayNumber;
		d.mu_r_sqrt = pow(r->previousRefractiveIndex, 2);
		d.n_e = getElectronNumberDensity();
		d.omega_p = getPlasmaFrequency();
		d.theta_0 = r->originalAngle;
		d.frequency = r->frequency;
		d.signalPower = r->signalPower;
		d.collisionType = Geometry::ionosphere;
		Application::getInstance().addToDataset(d);
	}

	/**
	 * Return a randomly distributed electron peak density. The electron peak randomly varies with a certain
	 * standard deviation. The mean and standard deviation are based on the normal peak electron density and
	 * delta ne as given by [Gurnett, 2009].
	 * @unit: particles m^-3
	 */
	double Ionosphere::getElectronPeakDensity() {

		return _electronPeakDensity;
	}

	/**
	 * Calculate the plasma frequency which depends on the electron number density
	 * which depends on the altitude (y). Use a chapman profile.
	 * @unit: rad s^-1
	 */
	double Ionosphere::getPlasmaFrequency() {

		return sqrt(getElectronNumberDensity() * pow(Constants::ELEMENTARY_CHARGE, 2)
				/ (Constants::ELECTRON_MASS * Constants::PERMITTIVITY_VACUUM));
	}

	/**
	 * Use a chapmanProfile to calculate the electron number density
	 * @unit: particles m^-3
	 */
	double Ionosphere::getElectronNumberDensity() {

		double normalizedHeight = (_altitude - _peakProductionAltitude) / Constants::NEUTRAL_SCALE_HEIGHT;

		return getElectronPeakDensity() *
				exp(0.5f * (1.0f - normalizedHeight - (1.0 / cos(geom.getSolarZenithAngle2d())) * exp(-normalizedHeight) ));
	}

	/**
	 * Compute the plasma refractive index. Three refractive methods are supplied:
	 * - SIMPLE:
	 * - KELSO: The simplified method as described in Kelso, 1964, p.208
	 * - AHDR: Refractive index according to the Appleton-Hartree
	 * dispersion relation
	 */
	double Ionosphere::getRefractiveIndex(Ray *r, refractiveMethod m) {

		double n = 1.0;

		if (m == REFRACTION_SIMPLE) {

			n = sqrt(1 - getPlasmaFrequency() / (2 * Constants::PI * r->frequency));
		} else if (m == REFRACTION_KELSO) {

			n = sqrt(1 - getElectronNumberDensity() * pow(Constants::ELEMENTARY_CHARGE, 2) /
								(Constants::ELECTRON_MASS * Constants::PERMITTIVITY_VACUUM * pow(2 * Constants::PI * r->frequency,2)));
		} else if (m == REFRACTION_AHDR) {

			//n = 1 - X / (1);
		}

		return n;
	}

	/**
	 * Interpolate the altitude of this ionospheric layer. Throw an exception
	 * if the difference between the y-components of the edges of the mesh is
	 * too large, for this would imply the ionospheric layer is under an angle
	 * which does not make sense.
	 */
	double Ionosphere::getAltitude() {

		double xAvg = (geom.mesh2d.begin.x + geom.mesh2d.end.x)/2;
		double yAvg = (geom.mesh2d.begin.y + geom.mesh2d.end.y)/2;

		return sqrt(pow(xAvg, 2) + pow(yAvg, 2)) - Config::getInstance().getInt("radius");
	}

	/**
	 * The incident angle of a ray with respect to the ionospheric layer. This angle depends
	 * on the propagation angle of the ray and the angle of the layer w.r.t. the sun (SZA)
	 */
	double Ionosphere::getIncidentAngle(Ray *r) {

		double SZA = geom.getSolarZenithAngle2d();
		double beta = atan(r->d.y/r->d.x);
		double theta_i = Constants::PI/2 - beta - SZA;

		return theta_i;
	}

	/**
	 * Model the collision frequency according to Nielsen 2007, figure 4. This is an interpolated
	 * approximation and therefore only valid between 30 and 200km altitude.
	 */
	double Ionosphere::getCollisionFrequency() {

		if (_altitude < 30e3 || _altitude > 200e3) {
			cerr << "Altitude: " << _altitude << endl;
			printf("Begin x,y, end x,y (%4.2f, %4.2f), (%4.2f, %4.2f)\n",
					geom.mesh2d.begin.x, geom.mesh2d.begin.y, geom.mesh2d.end.x, geom.mesh2d.end.y);
			throw std::invalid_argument("Collision frequency interpolation not valid for this altitude! Altitude must be between 30 and 200km.");
		}

		return Ionosphere::surfaceCollisionFrequency * exp(-(_altitude - 73e3) / 6200);
	}

	/**
	 * A wave is either reflected or refracted depending on its incident angle. According to
	 * Snells' law, a critical angle exists for which the reflected angle is 90 deg. Incident
	 * angles above this critical angle are reflected, not refracted.
	 */
	int Ionosphere::determineWaveBehaviour(Ray *r) {

		double refractiveIndex = getRefractiveIndex(r, Ionosphere::REFRACTION_KELSO);
		double criticalAngle;
		double incidentAngle = getIncidentAngle(r);

		if (incidentAngle > Constants::PI/2)
			incidentAngle -= Constants::PI/2;

		if (refractiveIndex <= r->previousRefractiveIndex)
			criticalAngle = asin(refractiveIndex / r->previousRefractiveIndex);
		else
			criticalAngle = asin(r->previousRefractiveIndex / refractiveIndex);

		if (incidentAngle >= criticalAngle)
			return Ray::wave_reflection;
		else
			return Ray::wave_refraction;

		return Ray::wave_none;
	}

} /* namespace scene */
} /* namespace raytracer */
