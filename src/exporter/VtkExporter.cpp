/*
 * VtkExporter.cpp
 *
 *  Created on: 11 Jun 2016
 *      Author: rian
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include "VtkExporter.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace raytracer {
namespace exporter {

	VtkExporter::VtkExporter() {
		// TODO Auto-generated constructor stub
	}

	VtkExporter::VtkExporter(const char *filepath) {

		// check if file exists
		std::ifstream infile(filepath);
		if (!infile.good()) {
			std::ofstream data;
			data.open(filepath);
			data.close();
		} else {
			BOOST_LOG_TRIVIAL(fatal) << "file " << filepath << " already exists! Overwriting.";
//			std::exit(0);
		}
	}

	void VtkExporter::dump(const char *filepath, std::list<Data> dataset) {

		std::ofstream data;
		int datasetSize = dataset.size();
		data.open(filepath);

		int numRays = 0;
		for (std::list<Data>::const_iterator it = dataset.begin(), end = dataset.end(); it != end; ++it) {
			const Data &elem = *it;
			if (numRays < elem.rayNumber) {
				numRays = elem.rayNumber;
			}
		}
		std::vector<std::vector<Data>> mappedDataset(numRays, std::vector<Data>(0));

		// sort data
		for (Data &elem : dataset) {
			int rayN = elem.rayNumber-1;
			mappedDataset[rayN].push_back(elem);
		}

		// header
		data << "# vtk DataFile Version 3.0" << std::endl;

		// title
		data << numRays << " Rays in vtk format"  << std::endl;

		// data type
		data << "ASCII" << std::endl;

		// geometry/topology
		data << "DATASET POLYDATA" << std::endl;

		// dataset attributes - points
		data << "POINTS " << datasetSize << " float" << std::endl;
		for (const auto &n : mappedDataset) {
			for (const Data &elem : n) {
				data << std::fixed << (int)round(elem.x) << " "
									<< (int)round(elem.y) << " "
									<< (int)round(elem.z) << std::endl;
			}
		}

		// dataset attributes - lines
		data << std::endl << "LINES " << datasetSize << " " << (datasetSize-1)*3 << std::endl;
		int lastCounter = 0;
		for (const auto &n : mappedDataset) {
			int i = 0;
			for (const Data &elem : n) {
				if (elem.x != n.back().x) {
					data << "2 " << lastCounter+i << " " << lastCounter+i+1 << std::endl;
				}
				i++;
			}
			lastCounter = i;
		}

		data.close();

	}

} /* namespace exporter */
} /* namespace raytracer */
