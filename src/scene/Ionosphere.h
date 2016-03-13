//============================================================================
// Name        : Ionosphere.h
// Author      : Rian van Gijlswijk
// Description : Perform interaction between a ray and an ionospheric layer
//				 based on the ionospheric properties
//============================================================================

#ifndef IONOSPHERE_H_
#define IONOSPHERE_H_

#include "Geometry.h"
#include "../math/Vector2d.h"

namespace raytracer {
namespace scene {

	using namespace tracer;

	class Ionosphere : public Geometry {

		public:
			Ionosphere();
			Ionosphere(Plane3d mesh);
			Ionosphere(Vector3d n, Vector3d c);
			enum refractiveMethod {
				REFRACTION_SIMPLE,		// According to Kelso, 1964
				REFRACTION_AHDR			// Appleton-Hartree Dispersion Relation
			};
			enum waveMode {
				O_MODE,
				X_MODE,
				L_MODE,
				R_MODE
			};
			void setup();

			/**
			 * Interaction between ray and ionospheric layer
			 */
			void interact(Ray *r, Vector3d &hitpos);
			void refract(Ray *r);
			void reflect(Ray *r);

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
			void attenuate(Ray *r);
			void attenuateWithers(Ray *r);
			void rangeDelay(Ray *r);
			void phaseAdvance(Ray *r);
			void timeDelay(Ray *r);
			void exportData(Ray *r);

			/**
			 * Include the effects of magnetic fields
			 */
			void calculateMagneticFieldEffects(Ray *r);

			/**
			 * Calculate the plasma frequency which depends on the electron number density
			 * which depends on the altitude (y). Use a chapman profile.
			 * @unit: rad s^-1
			 */
			double getPlasmaFrequency();

			/**
			 * Calculate the plasma frequency at the layer maximum, which depends on the
			 * peak electron number density. The peak density for the layer with the highest
			 * electron number density is used.
			 * @unit: rad s^-1
			 */
			double getPeakPlasmaFrequency();

			/**
			 * Add electrons with a certain density to the electron density already available in this layer.
			 * This approach allows the superposition of multiple ionospheric profiles into one layer.
			 * The electron density at the layer is assumed to follow a chapman profile for the daytime.
			 */
			void superimposeElectronNumberDensity(double peakDensity, double peakAltitude, double neutralScaleHeight);

			/**
			 * Use a chapmanProfile to calculate the electron number density
			 * @unit: particles m^-3
			 */
			double getElectronNumberDensity();
			void setElectronNumberDensity(double n_e);

			/**
			 * Compute the plasma refractive index. Three refractive methods are supplied:
			 * - SIMPLE:
			 * - KELSO: The simplified method as described in Kelso, 1964, p.208
			 * - AHDR: Refractive index according to the Appleton-Hartree
			 * dispersion relation. Only applies to a magnetized cold plasma.
			 */
			double getRefractiveIndex(Ray *r, refractiveMethod m);
			double getRefractiveIndexSquared(Ray *r, refractiveMethod m, double plasmaFrequency);
			double getRefractiveIndexSquaredSimple(Ray *r, double plasmaFrequency);
			Vector2d getRefractiveIndexSquaredAHDR(Ray *r, double plasmaFrequency);

			/**
			 * The incident angle of a ray with respect to the ionospheric layer. This angle depends
			 * on the propagation angle of the ray and the angle of the layer w.r.t. the sun (SZA)
			 * The angle is then complementary to the angle between the ray direction vector and the
			 * normal of the plane.
			 */
			double getIncidentAngle(Ray *r);

			/**
			 * Model the collision frequency
			 * @unit Hz
			 */
			double getCollisionFrequency();
			void setCollisionFrequency();
			void setCollisionFrequency(double freq);

			/**
			 * Calculate the electron angular gyrofrequency [rad s^-1]
			 */
			double getGyroFrequency();

			/**
			 * Calculate the total electron content
			 *  (TEC) which this ray experiences as it passes through
			 * the ionosphere.
			 */
			double getTEC();
			int determineWaveBehaviour(Ray *r);
			double layerHeight = 0;
			double electronDensityVariability = 0;
			static constexpr double surfaceCollisionFrequency = 4.5e10;	// s^-1

			double angleToMagField = 0;

		private:
			double _electronNumberDensity = 0;	// m^-3
			double _peakElectronDensity = 0;	// m^-3
			double _collisionFrequency = 0;		// s^-1
	};

} /* namespace scene */
} /* namespace raytracer */

#endif /* IONOSPHERE_H_ */
