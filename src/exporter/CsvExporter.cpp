/*
 * DatExporter.cpp
 *
 *  Created on: 22 Jan 2015
 *      Author: rian
 */

#include <iostream>
#include <fstream>
#include "CsvExporter.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace raytracer {
namespace exporter {

	CsvExporter::CsvExporter() {
		// TODO Auto-generated constructor stub
	}

	CsvExporter::CsvExporter(const char *filepath) {

		// check if file exists
		std::ifstream infile(filepath);
		if (!infile.good()) {
			std::ofstream data;
			data.open(filepath);
			data.close();
		} else {
			BOOST_LOG_TRIVIAL(fatal) << "file " << filepath << " already exists! Exiting.";
			std::exit(0);
		}
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
