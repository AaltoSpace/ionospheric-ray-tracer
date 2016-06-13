//============================================================================
// Name        : IExporter.h
// Author      : Rian van Gijlswijk
// Description : Interface for data export implementation
//============================================================================

#ifndef IEXPORTER_H_
#define IEXPORTER_H_

#include <list>
#include "Data.h"

namespace raytracer {
namespace exporter {

	enum ExporterType {
		Csv = 1,
		Matlab = 2,
		Vtk = 3,
		Json = 4
	};

	class IExporter {

		public:
			IExporter() {}
			IExporter(const char *filepath);
			virtual ~IExporter() {}
			virtual void dump(const char *filepath, std::list<Data> dataset) = 0;

	};

}
}

#endif
