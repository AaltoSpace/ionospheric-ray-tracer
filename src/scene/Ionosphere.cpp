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
#include "GeometryType.h"
#include "../core/Application.h"
#include "../core/Config.h"
#include "../exporter/Data.h"
#include "../math/NormalDistribution.h"
#include "../math/Constants.h"
#include "../math/ComplexNumberHelper.h"

namespace raytracer {
namespace scene {

	using namespace std;
	using namespace exporter;
	using namespace core;

	Ionosphere::Ionosphere() : Geometry() {

		type = GeometryType::ionosphere;
	}

	Ionosphere::Ionosphere(Plane3d mesh) : Geometry(mesh) {

		type = GeometryType::ionosphere;
	}

	Ionosphere::Ionosphere(Vector3d n, Vector3d c) : Geometry(n, c) {

		type = GeometryType::ionosphere;
	}

	/**
	 * Precalculate fixed values for this ionospheric layer. Assume static ionosphere as the
	 * time periods that we're interested in are an order of magnitude lower than the processes
	 * of photochemical reactions (discussion with Cyril Simon Wedlund)
	 */
	void Ionosphere::setup() {

		setCollisionFrequency();
	}

	/**
	 * Interaction between ray and ionospheric layer
	 */
	void Ionosphere::interact(Ray *r, Vector3d &hitpos) {

		BOOST_LOG_TRIVIAL(debug) << "Interact with ionosphere at alt " << r->altitude;
		altitude = r->altitude;

		setup();

		int waveBehaviour = determineWaveBehaviour(r);

		if (waveBehaviour == Ray::wave_reflection) {
			reflect(r);
		} else if (waveBehaviour == Ray::wave_refraction) {
			refract(r);
		}
		r->o = hitpos;

		attenuate(r);
		rangeDelay(r);
		phaseAdvance(r);
		timeDelay(r);

		if (Application::getInstance().includeMagneticFieldEffects()) {

		}

		exportData(r);
	}

	/**
	 *
	 */
	void Ionosphere::refract(Ray *r) {

		double refractiveIndex = 0;

		// load magnetic field parameters
		Json::Value magFields = Application::getInstance().getApplicationConfig()
									.getArray("magneticFields");
		double magneticFieldStrength = getMagneticFieldStrengthFromConfig();

		// calculate the nonlinear refractive index in a magnetized environment
		if (magneticFieldStrength > 0) {
			BOOST_LOG_TRIVIAL(debug) << "Retrieving refraction for magnetized ionosphere";
			// todo: calculate correct angle to magnetic field
			Vector3d k = Config::getVector3dFromObject(magFields[0].get("direction", ""));
			double angleToMagfield = 0;//acos(r->d.dot(k));
			// NONLINEAR solver: iterate the part below
			// todo: implement nonlinear solver iterator
			Vector2d refractiveIndex = getRefractiveIndexSquaredAHDR(r, angleToMagfield, getPlasmaFrequency());
			// todo: recalculate theta_r based on new refractive index

		// calculate the linear refractive index in a nonmagnetized environment
		} else {
			refractiveIndex = sqrt(getRefractiveIndexSquaredSimple(r, getPlasmaFrequency()));
		}
		double theta_i = getIncidentAngle(r);

		double ratio = r->previousRefractiveIndex/refractiveIndex;
		double coefficient = ratio * cos(theta_i) - sqrt(1 - pow(ratio, 2) * (1 - pow(cos(theta_i), 2)));
		Vector3d newR = Vector3d();

		if (r->d.y > 0)
			newR = r->d * ratio - mesh3d.normal * coefficient;
		else
			newR = r->d * ratio + mesh3d.normal * coefficient;

//		BOOST_LOG_TRIVIAL(debug) << std::fixed << "n1: " << r->previousRefractiveIndex << ", n2: " << refractiveIndex;
//		BOOST_LOG_TRIVIAL(debug) << "REFRACT Alt: " << std::setprecision(0) << getAltitude() << "\tr.d_i: " << r->d << "\tr.d_r: " << newR;
//		BOOST_LOG_TRIVIAL(debug) << "N: " << mesh3d.normal << "\tn1/n2: " << ratio << "\ttheta_i: " << theta_i*180/Constants::PI << "\ttheta_r: " << newR.angle(mesh3d.normal) * 180 / Constants::PI;

		r->d = newR.norm();
		r->previousRefractiveIndex = refractiveIndex;
	}

	/**
	 *
	 */
	void Ionosphere::reflect(Ray *r) {

		double refractiveIndex = getRefractiveIndex(r, Ionosphere::REFRACTION_SIMPLE);
		double theta_i = getIncidentAngle(r);

		Vector3d newR = Vector3d();

		if (r->d.y > 0)
			newR = r->d - mesh3d.normal * 2 * cos(theta_i);
		else
			newR = r->d - mesh3d.normal * 2 * cos(Constants::PI - theta_i);

		BOOST_LOG_TRIVIAL(debug) << std::fixed << "REFLECT Alt: " << std::setprecision(0) << getAltitude() << "\tr.d_i: " << r->d << "\tr.d_r: " << newR << "\tN: " << mesh3d.normal << "\ttheta_i: " << theta_i;

		r->d = newR.norm();
		//r->previousRefractiveIndex = refractiveIndex;
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
	void Ionosphere::attenuate(Ray *r) {

		double theta_r = getMesh().normal.angle(r->d);
		if (theta_r > Constants::PI/2) {
			theta_r -= Constants::PI/2;
		}
		double magnitude = layerHeight * cos(theta_r);
		double collisionFrequency = getCollisionFrequency();

		double loss = -4.6e-5
				* (getElectronNumberDensity() * collisionFrequency / (pow(2 * Constants::PI * r->frequency, 2) + pow(collisionFrequency, 2)))
				* magnitude;

		r->signalPower += loss;
	}

	/**
	 * Range delay. Data from Ho, 2002.
	 * @ todo: check formulas
	 * @ unit: m
	 */
	void Ionosphere::rangeDelay(Ray *r) {

		r->rangeDelay += 0.403 * getTEC() / pow(r->frequency, 2);
	}

	/**
	 * Phase advance. Data from Ho, 2002.
	 * @ todo: check formulas
	 * @ unit: rad
	 */
	void Ionosphere::phaseAdvance(Ray *r) {

		r->phaseAdvance += (8.44e-7 / r->frequency ) * getTEC();
	}

	/**
	 * Time delay. Data from Ho, 2002.
	 * @ todo: check formulas
	 * @ unit: sec
	 */
	void Ionosphere::timeDelay(Ray *r) {

		r->timeDelay += (1.34e-7 / pow(r->frequency, 2)) * getTEC();
	}

	void Ionosphere::exportData(Ray *r) {

		Data d;
		d.x = r->o.x;
		d.y = r->o.y;
		d.z = r->o.z;
		d.rayNumber = r->rayNumber;
		d.mu_r_sqrt = pow(r->previousRefractiveIndex, 2);
		d.n_e = getElectronNumberDensity();
		d.omega_p = getPlasmaFrequency();
		d.theta_0 = r->originalAngle;
		d.frequency = r->frequency;
		d.signalPower = r->signalPower;
		d.timeOfFlight = r->timeOfFlight;
		d.collisionType = GeometryType::ionosphere;
		d.beaconId = r->originBeaconId;
		d.azimuth_0 = r->originalAzimuth;
		Application::getInstance().addToDataset(d);
	}

	/**
	 * Include the effects of magnetic fields
	 */
	void calculateMagneticFieldEffects(Ray *r) {

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
	 * Calculate the plasma frequency at the layer maximum, which depends on the
	 * peak electron number density. The peak density for the layer with the highest
	 * electron number density is used.
	 * @unit: rad s^-1
	 */
	double Ionosphere::getPeakPlasmaFrequency() {

		return sqrt(_peakElectronDensity * pow(Constants::ELEMENTARY_CHARGE, 2)
				/ (Constants::ELECTRON_MASS * Constants::PERMITTIVITY_VACUUM));
	}

	/**
	 * Use a chapmanProfile to calculate the electron number density
	 * @unit: particles m^-3
	 */
	double Ionosphere::getElectronNumberDensity() {

		return _electronNumberDensity;
	}

	void Ionosphere::setElectronNumberDensity(double n_e) {

		_electronNumberDensity = n_e;
	}

	/**
	 * Add electrons with a certain density to the electron density already available in this layer.
	 * This approach allows the superposition of multiple ionospheric profiles into one layer.
	 * The electron density at the layer is assumed to follow a chapman profile for the daytime.
	 */
	void Ionosphere::superimposeElectronNumberDensity(double peakDensity, double peakAltitude, double neutralScaleHeight) {

		double SZA = mesh3d.normal.angle(Vector3d::SUBSOLAR);
		double correctedPeakAltitude = peakAltitude + 1e4 * log(1/cos(SZA));
		double normalizedHeight = (getAltitude() - correctedPeakAltitude) / neutralScaleHeight;

		if (peakDensity > _peakElectronDensity) {
			_peakElectronDensity = peakDensity;
		}

		double electronNumberDensity = peakDensity *
				exp(0.5f * (1.0f - normalizedHeight - (1.0 / cos(SZA)) * exp(-normalizedHeight) ));

		_electronNumberDensity += electronNumberDensity;

		BOOST_LOG_TRIVIAL(debug) << "Set n_e at alt=" << altitude << " to " << _electronNumberDensity;
	}

	/**
	 * Compute the plasma refractive index. Three refractive methods are supplied:
	 * - SIMPLE: The simplified method as described in Kelso, 1964, p.208
	 * - AHDR: Refractive index according to the Appleton-Hartree
	 * dispersion relation. Only applies to a magnetized cold plasma.
	 */
	double Ionosphere::getRefractiveIndex(Ray *r, refractiveMethod m) {

		return sqrt(getRefractiveIndexSquared(r, m, getPlasmaFrequency() ));
	}

	double Ionosphere::getRefractiveIndexSquared(Ray *r, refractiveMethod m, double plasmaFrequency) {

		if (m == REFRACTION_SIMPLE) {
			return getRefractiveIndexSquaredSimple(r, plasmaFrequency);
//		} else if (m == REFRACTION_AHDR) {
//			return getRefractiveIndexSquaredAHDR(r, plasmaFrequency).x;
		} else {
			BOOST_LOG_TRIVIAL(error) << "The given refractive method does not exist!";
		}
	}

	double Ionosphere::getRefractiveIndexSquaredSimple(Ray *r, double plasmaFrequency) {

		double X = pow(getPlasmaFrequency(), 2) / pow(2 * Constants::PI * r->frequency, 2);
		double Z = getCollisionFrequency() / getPlasmaFrequency();
		double a = 1.0 / (1 + pow(Z, 2));

		return 1 - X;
	}

	Vector2d Ionosphere::getRefractiveIndexSquaredAHDR(Ray *r, double angleToMagField, double plasmaFrequency) {

		double angularFrequency = 2 * Constants::PI * r->frequency;
		double X = pow(plasmaFrequency, 2) / pow(angularFrequency, 2);

		double Y = getGyroFrequency() / angularFrequency;
		double Y_T = 0;
		double Y_L = 0;
		if (Y > 0) {
			Json::Value magFields = Application::getInstance().getApplicationConfig()
							.getArray("magneticFields");
			double BTot = getMagneticFieldStrengthFromConfig();
			Vector3d k = Config::getVector3dFromObject(magFields[0].get("direction", ""));
			Vector3d B = Vector3d(BTot * sin(angleToMagField), 0, BTot * cos(angleToMagField));
			Y_T = Y * sin(angleToMagField); //Y * Y * k.cross(B).magnitude() / BTot;
			Y_L = Y * cos(angleToMagField); //k.dot(B) / BTot;
		}
		double Z = getCollisionFrequency() / angularFrequency;
//		BOOST_LOG_TRIVIAL(info) << "X:" << X << ",Y:" << Y << ",Z:" << Z << ",YT:" << Y_T;
		double alpha = (pow(Y_T, 4) * (pow(1.0 - X, 2) - pow(Z, 2)))
				/(4.0 * pow(pow(1.0-X, 2) + pow(Z, 2), 2))
				+ pow(Y_L, 2);
		// ((1-X)*Z) /(2*((1-X)^2 + Z^2)^2);
		double beta = ((1.0 - X) * Z) / (2.0 * pow(pow(1.0 - X, 2) + pow(Z, 2), 2));
		ComplexDouble Ephi = ComplexNumberHelper::getInstance().complexSquareRoot(alpha, beta);
		double E = Ephi.real();
		double phi = Ephi.imag();
//		BOOST_LOG_TRIVIAL(info) << "a:" << alpha << ",b:" << beta << ",E:" << E << ",phi:" << phi;
		//W = (Y_T^2*(1-X)) / (2*((1-X)^2)+Z^2);
		double W = (pow(Y_T, 2) * (1.0 - X))/(2 * (pow(1.0 - X, 2) + pow(Z, 2) ));
		//Q = (Y_T^2*Z)     / (2*((1-X)^2)+Z^2);
		double Q = (pow(Y_T, 2) * Z)/(2.0 * (pow(1.0 - X, 2) + pow(Z, 2) ));
		// Derive two pure Real variables M and N:
		// M(1) = (1-W+E)/X;
		double M_1 = (1.0 - W + E) / X;
		// M(2) = (1-W-E)/X;
		double M_2 = (1.0 - W - E) / X;
		// N(1) = (-Z -Q + Theta)/X;
		double N_1 = (-Z -Q + phi) / X;
		// N(2) = (-Z -Q - Theta)/X;
		double N_2 = (-Z -Q - phi) / X;

//		BOOST_LOG_TRIVIAL(info) << "M1:" << M_1 << ",M2:" << M_2 << ",N1:" << N_1 << ",N2:" << N_2;

		// Derive two pure Real variables A and B:
		// A(1) = (1 - (M(1)/(M(1)^2 + N(1)^2)));
		double A_1 = (1.0 - (M_1/(pow(M_1, 2) + pow(N_1, 2) )  ));
		// A(2) = (1 - (M(2)/(M(2)^2 + N(2)^2)));
		double A_2 = (1.0 - (M_2/(pow(M_2, 2) + pow(N_2, 2) )  ));
		// B(1) = N(1)/(M(1)^2 + N(1)^2);
		double B_1 = (N_1/(pow(M_1, 2) + pow(N_1, 2) )  );
		// B(2) = N(2)/(M(2)^2 + N(2)^2);
		double B_2 = (N_2/(pow(M_2, 2) + pow(N_2, 2) )  );

//		BOOST_LOG_TRIVIAL(info) << "1:" << ComplexNumberHelper::getInstance().complexSquareRoot(A_1, B_1);
//		BOOST_LOG_TRIVIAL(info) << "2:" << ComplexNumberHelper::getInstance().complexSquareRoot(A_2, B_2);

		Vector2d result;
		result.x = ComplexNumberHelper::getInstance().complexSquareRoot(A_1, B_1).real();
		result.y = ComplexNumberHelper::getInstance().complexSquareRoot(A_2, B_2).real();
		return result;

	}

	/**
	 * The incident angle of a ray with respect to the ionospheric layer. This angle depends
	 * on the propagation angle of the ray and the angle of the layer w.r.t. the sun (SZA)
	 * The angle is then complementary to the angle between the ray direction vector and the
	 * normal of the plane.
	 * @bugfix 296d831
	 */
	double Ionosphere::getIncidentAngle(Ray *r) {

		// note: the ABSOLUTE angle. Angle between vectors doesnt work.
		return acos(abs(r->d * mesh3d.normal) / (r->d.magnitude() * mesh3d.normal.magnitude()));
	}

	double Ionosphere::getCollisionFrequency() {

		return _collisionFrequency;
	}

	/**
	 * Model the collision frequency
	 * @unit Hz
	 */
	void Ionosphere::setCollisionFrequency() {

		double nCO2 = Application::getInstance().getCelestialConfig().getDouble("surfaceNCO2")
				* exp(-getAltitude() / Constants::NEUTRAL_SCALE_HEIGHT);
		_collisionFrequency = 1.0436e-07 * nCO2;
	}

	void Ionosphere::setCollisionFrequency(double frequency) {

		_collisionFrequency = frequency;
	}

	/**
	 * Calculate the electron angular gyrofrequency [rad s^-1]
	 */
	double Ionosphere::getGyroFrequency() {

		return Constants::ELEMENTARY_CHARGE * getMagneticFieldStrengthFromConfig() / Constants::ELECTRON_MASS;
	}

	/**
	 * Calculate the total electron content
	 *  (TEC) which this ray experiences as it passes through
	 * the ionosphere.
	 */
	double Ionosphere::getTEC() {

		return getElectronNumberDensity() * layerHeight;
	}

	/**
	 * A wave is either reflected or refracted depending on its incident angle. According to
	 * Snells' law, a critical angle exists for which the reflected angle is 90 deg. Incident
	 * angles above this critical angle are reflected, not refracted.
	 * @ todo: REWRITE COMPLETELY
	 */
	int Ionosphere::determineWaveBehaviour(Ray *r) {

		r->behaviour = Ray::wave_none;

		double criticalAngle;
		double refractiveIndex = getRefractiveIndex(r, Ionosphere::REFRACTION_SIMPLE);
		double incidentAngle = getIncidentAngle(r);
		double angularFrequency = 2 * Constants::PI * r->frequency;
		double epsilon = 1e-5;

		if (incidentAngle > Constants::PI/2)
			incidentAngle -= Constants::PI/2;

		if (angularFrequency < getPlasmaFrequency())
			r->behaviour = Ray::wave_reflection;
		else {

			if (refractiveIndex <= r->previousRefractiveIndex)
				criticalAngle = asin(refractiveIndex / r->previousRefractiveIndex);
			else
				criticalAngle = asin(r->previousRefractiveIndex / refractiveIndex);

			if (r->previousRefractiveIndex > refractiveIndex && incidentAngle >= criticalAngle)
				r->behaviour = Ray::wave_reflection;
			else
				r->behaviour = Ray::wave_refraction;
		}

		return r->behaviour;
	}

	/**
	 * Retrieve the magnetic field strength from the current configuration file
	 * Return 0 if the value cannot be found in the config
	 */
	double Ionosphere::getMagneticFieldStrengthFromConfig() {

		Json::Value magFields = Application::getInstance().getApplicationConfig()
						.getArray("magneticFields");

		if (magFields.isNull() || magFields.size() < 1) {
			return 0;
		}

		return magFields[0].get("strength", "").asDouble();
	}

} /* namespace scene */
} /* namespace raytracer */
