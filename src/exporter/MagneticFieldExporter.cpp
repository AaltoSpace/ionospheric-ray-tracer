/*
 * MagneticFieldExporter.cpp
 *
 *  Created on: 22 Jan 2015
 *      Author: rian
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include "MagneticFieldExporter.h"

namespace raytracer {
namespace exporter {

	using namespace std;

	MagneticFieldExporter::MagneticFieldExporter() {
		// TODO Auto-generated constructor stub
	}

	void MagneticFieldExporter::dump(const char *filepath, list<Data> dataset) {

		ofstream data;
		data.open(filepath);
		while (!dataset.empty()) {
			data << std::fixed << std::setprecision(1) << dataset.front().rayNumber << ","
					<< std::setprecision(6) << dataset.front().n << ","
					<< std::setprecision(3) << dataset.front().omega_p << ","
					<< std::setprecision(1) << dataset.front().frequency << "\n";
			dataset.pop_front();
		}
		data.close();

	}

} /* namespace exporter */
} /* namespace raytracer */
