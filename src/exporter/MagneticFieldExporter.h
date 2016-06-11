//============================================================================
// Name        : MagneticFieldExporter.h
// Author      : Rian van Gijlswijk
// Description : Exports data relevant for magnetic field inclusion
//============================================================================

#ifndef MATLABEXPORTER_H_
#define MATLABEXPORTER_H_

#include "IExporter.h"

namespace raytracer {
namespace exporter {

class MagneticFieldExporter : public IExporter {

	public:
		MagneticFieldExporter();
		MagneticFieldExporter(const char *filepath);
		~MagneticFieldExporter() {}
		void dump(const char *filepath, std::list<Data> dataset);

};

} /* namespace exporter */
} /* namespace raytracer */

#endif /* MagneticFieldExporter */
