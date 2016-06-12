//============================================================================
// Name        : VtkExporter.h
// Author      : Rian van Gijlswijk
// Description : Exports a 2D dataset to a .vtk file for use in ParaView
// More info   : http://www.visitusers.org/index.php?title=ASCII_VTK_Files
//				 http://www.vtk.org/wp-content/uploads/2015/04/file-formats.pdf
//============================================================================

#ifndef VtkEXPORTER_H_
#define VtkEXPORTER_H_

#include "IExporter.h"

namespace raytracer {
namespace exporter {


class VtkExporter : public IExporter {

	public:
		VtkExporter();
		VtkExporter(const char *filepath);
		~VtkExporter() {}
		void dump(const char *filepath, std::list<Data> dataset);

};

} /* namespace exporter */
} /* namespace raytracer */

#endif /* VtkEXPORTER_H_ */
