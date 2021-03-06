/*
 * Config.cpp
 *
 *  Created on: 13 Feb 2015
 *      Author: rian
 */

#include "Config.h"
#include <iostream>
#include <fstream>
#include <string>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "../../contrib/jsoncpp/reader.h"

namespace raytracer {
namespace core {

	using namespace std;

	Config::Config() {}

	Config::Config(const char * filepath) {

		loadFromFile(filepath);
	}

	void Config::loadFromFile(const char * filepath) {

		Json::Reader r = Json::Reader();
		std::ifstream test(filepath, std::ifstream::binary);
		bool success = r.parse(test, _doc, false);
		if ( !success ) {
			// report to the user the failure and their locations in the document.
			BOOST_LOG_TRIVIAL(fatal) << "Formatting error in config file "
					<< filepath
					<< "!\n" << r.getFormattedErrorMessages();
			std::exit(0);
		}
	}

	int Config::getInt(const char * path) {

		if (!_doc.isMember(path)) {
			cerr << path << " does not exist!" << endl;
		}

		return _doc.get(path, "").asInt();
	}

	math::Vector3d Config::getVector3dFromObject(const Json::Value obj) {

		if (!obj.size() == 3){
			cerr << obj.asCString() << " is not a 3D vector! It should have 3 elements.";
		}

		math::Vector3d output = math::Vector3d(
			obj[0].asDouble(),
			obj[1].asDouble(),
			obj[2].asDouble());

		return output;
	}

	double Config::getDouble(const char * path) {

		if (!_doc.isMember(path)) {
			cerr << path << " does not exist!" << endl;
		}

		return atof(_doc.get(path, "").asCString());
	}

	Json::Value Config::getArray(const char * path) {

		if (!_doc.isMember(path)) {
			cerr << path << " does not exist!" << endl;
		}

		if (!_doc.get(path, "").isArray()) {
			cerr << path << " is not an array!" << endl;
		}

		return _doc[path];
	}

	Json::Value Config::getObject(const char * path) {

		if (!_doc.isMember(path)) {
			cerr << path << " does not exist!" << endl;
		}

		if (!_doc.get(path, "").isObject()) {
			cerr << path << " is not an object!" << endl;
		}

		return _doc[path];
	}

} /* namespace core */
} /* namespace raytracer */
