/*
 * DatExporter.cpp
 *
 *  Created on: 22 Jan 2015
 *      Author: rian
 */

#include <iostream>
#include <fstream>
#include "CsvExporter.h"

namespace raytracer {
namespace exporter {

	CsvExporter::CsvExporter() {
		// TODO Auto-generated constructor stub
	}

	void CsvExporter::dump(const char *filepath, std::list<Data> dataset) {

		std::ofstream data;
		data.open(filepath);
		data << "x,y\n";
		while (!dataset.empty()) {
			data << dataset.front().x << "," << dataset.front().y << "\n";
			dataset.pop_front();
		}
		data.close();

	}

} /* namespace exporter */
} /* namespace raytracer */
