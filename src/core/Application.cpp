//============================================================================
// Name        : Application.cpp
// Author      : Rian van Gijlswijk
// Description : Main application file.
//============================================================================

#include "Application.h"
#include <string>
#include <regex>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include "Timer.cpp"
#include "CommandLine.h"
#include "../math/Matrix3d.h"
#include "../exporter/CsvExporter.h"
#include "../exporter/JsonExporter.h"
#include "../exporter/MatlabExporter.h"
#include "../exporter/VtkExporter.h"
#include "../radio/AntennaFactory.h"
#include "../radio/IsotropicAntenna.h"
#include "../commands/Wavetypes.h"

namespace raytracer {
namespace core {

	using namespace std;
	using namespace commands;
	using namespace tracer;
	using namespace exporter;
	using namespace math;
	using namespace threading;
	using namespace radio;

	boost::mutex datasetMutex;
	boost::mutex tracingIncMutex;

	boost::threadpool::pool tp;

	void Application::init(int argc, char * argv[]) {

		BOOST_LOG_TRIVIAL(debug) << "Init application";

		parseCommandLineArgs(argc, argv);
	}

	void Application::parseCommandLineArgs(int argc, char * argv[]) {

		BOOST_LOG_TRIVIAL(debug) << "parseCommandLineArgs";

		if (argc < 2) {
			BOOST_LOG_TRIVIAL(fatal) << "You need to supply at least one argument!";
			usage();
		}

		// argv[2+] = options
		for (int i = 2; i < argc; i++) {

			if (strcmp(argv[i], "-h") == 0) {
				usage();

			} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scenario") == 0) {
				_celestialConfigFile = argv[i+1];

			} else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) {
				_applicationConfigFile = argv[i+1];

			} else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--magneticfield") == 0) {
				_includeMagneticField = true;

			} else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parallelism") == 0) {
				_parallelism = atoi(argv[i+1]);

			} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
				_outputFile = argv[i+1];

			} else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--iterations") == 0) {
				_iterations = atoi(argv[i+1]);

			} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
				_verbosity = boost::log::trivial::info;

			} else if (strcmp(argv[i], "-vv") == 0) {
				_verbosity = boost::log::trivial::debug;

			} else if (strcmp(argv[i], "-fmin") == 0) {
				_fmin = atoi(argv[i+1]);

			} else if (strcmp(argv[i], "-fstep") == 0) {
				_fstep = atoi(argv[i+1]);

			} else if (strcmp(argv[i], "-fmax") == 0) {
				_fmax = atoi(argv[i+1]);
			}
		}

		// argv[0] = program name
		// argv[1] = command
		string commandArgument = argv[1];
		if (commandArgument.substr(0, 1) == "-") {
			BOOST_LOG_TRIVIAL(fatal) << "No command supplied! Usage:";
			usage();
		} else if (commandArgument.compare("simulation") == 0) {

			// load scenario config file. Must be given.
			if (!std::regex_match (argv[argc-1], std::regex("[A-Za-z0-9_/]+\.json") )) {
				BOOST_LOG_TRIVIAL(fatal) << "No scenario file given! Exiting.";
				std::exit(0);
			}
			_celestialConfigFile = argv[argc-1];

			start();
			run();
		} else if (commandArgument.compare("wavetypes") == 0) {
			Wavetypes cmd;
			cmd.start();
			cmd.run();
			cmd.stop();
		}
	}

	void Application::usage() {
		std::cout 	<< "Ionospheric Ray Tracer\n\n"
					<< "Synopsis:\n"
					<< "\tirt command [-opts] scenarioConfig\n\n"
					<< "Description: \n"
					<< "\tPerform ionospheric ray tracing on a celestial object described by the _celestialConfig json file. "
					<< "If no config file is supplied, use a default scenario.\n\n"
					<< "Options:\n"
					<< "\t-c | --config\t Application config file\n"
					<< "\t-i | --iterations\t The number of consecutive times every ray option should be run.\n"
					<< "\t-h | --help\t This help.\n"
					<< "\t-m | --magneticfield\t Include magnetic field effects.\n"
					<< "\t-o | --output\t Path where output file should be stored.\n"
					<< "\t-p | --parallelism\t Multithreading indicator.\n"
					<< "\t-v | --verbose\t Verbose, display log output\n"
					<< "\t-vv \t\t Very verbose, display log and debug output\n";
		std::exit(0);
	}

	void Application::start() {

		BOOST_LOG_TRIVIAL(debug) << "Start application";

		_isRunning = true;

		_applicationConfig = Config(_applicationConfigFile);
		_celestialConfig = Config(_celestialConfigFile);

		if (_parallelism < 1) {
			_parallelism = _applicationConfig.getInt("parallelism");
		}
		if (_iterations < 1) {
			_iterations = _applicationConfig.getInt("iterations");
		}
		if (_fmin < 1) {
			_fmin = _applicationConfig.getObject("frequencies")["min"].asInt();
		}
		if (_fstep < 1) {
			_fstep = _applicationConfig.getObject("frequencies")["step"].asInt();
		}
		if (_fmax < 1) {
			_fmax = _applicationConfig.getObject("frequencies")["max"].asInt();
		}

//		boost::log::add_file_log("log/sample.log");

		boost::log::core::get()->set_filter(
				boost::log::trivial::severity >= _verbosity);

		tp = boost::threadpool::pool(_parallelism);

		BOOST_LOG_TRIVIAL(debug) << "applicationConfig: " << _applicationConfigFile << "\n"
				<< _applicationConfig << "\n"
				<< "celestialConfig:" << _celestialConfigFile << "\n"
				<< _celestialConfig;

		configureExporter();
	}

	void Application::run() {

		BOOST_LOG_TRIVIAL(debug) << "Run application";

		Timer tmr;
		int radius = _celestialConfig.getInt("radius");

		BOOST_LOG_TRIVIAL(info) << "Parallelism is " << _applicationConfig.getInt("parallelism");
		if (_verbosity > boost::log::trivial::info) {
			std::ostringstream stringStream;
			stringStream << "Parallelism is " << _applicationConfig.getInt("parallelism");
			CommandLine::getInstance().addToHeader(stringStream.str().c_str());
		}
		BOOST_LOG_TRIVIAL(info) << _applicationConfig.getInt("iterations") << " iterations";

		// load config values
		double SZAmin = _applicationConfig.getObject("SZA")["min"].asDouble();
		double SZAstep = _applicationConfig.getObject("SZA")["step"].asDouble();
		double SZAmax = _applicationConfig.getObject("SZA")["max"].asDouble();
		double azimuthMin = _applicationConfig.getObject("azimuth")["min"].asDouble();
		double azimuthStep = _applicationConfig.getObject("azimuth")["step"].asDouble();
		double azimuthMax = _applicationConfig.getObject("azimuth")["max"].asDouble();
		const Json::Value beacons = _applicationConfig.getArray("beacons");

		// trace a ray
		int rayCounter = 0;
		for (int iteration = 0; iteration < _applicationConfig.getInt("iterations"); iteration++) {

			BOOST_LOG_TRIVIAL(info) << "Iteration " << (iteration+1) << " of " << _applicationConfig.getInt("iterations");

			createScene();

			BOOST_LOG_TRIVIAL(info) << "Simulating " << beacons.size() << " beacons";
			BOOST_LOG_TRIVIAL(info) << "Scanning frequencies " << _fmin << " Hz to " << _fmax << "Hz with steps of " << _fstep << "Hz";
			BOOST_LOG_TRIVIAL(info) << "Scanning SZA " << SZAmin << " deg to " << SZAmax << " deg with steps of " << SZAstep << " deg";
			BOOST_LOG_TRIVIAL(info) << "Scanning azimuth " << azimuthMin << " deg to " << azimuthMax << " deg with steps of " << azimuthStep << " deg";

			if (_verbosity > boost::log::trivial::info) {
				std::ostringstream stringStream;
				stringStream << "Simulating " << beacons.size() << " beacons\n" << "Scanning frequencies " << _fmin << " Hz to " << _fmax
						<< "Hz with steps of " << _fstep << "Hz\n" << "Scanning SZA " << SZAmin << " deg to "
						<< SZAmax << " deg with steps of " << SZAstep << " deg\n"
						<< "Scanning azimuth " << azimuthMin << " deg to " << azimuthMax << " deg with steps of " << azimuthStep << " deg";
						CommandLine::getInstance().addToHeader(stringStream.str().c_str());
			}

			for(int b = 0; b < beacons.size(); b++) {

				double latitudeOffset = beacons[b].get("latitudeOffset", "").asDouble() * Constants::PI / 180.0;
				double longitudeOffset = beacons[b].get("longitudeOffset", "").asDouble() * Constants::PI / 180.0;
				double beaconAltitude = beacons[b].get("altitude", "").asDouble();
				const Json::Value antenna = beacons[b].get("antenna", "");
//				IAntenna* ant = AntennaFactory::createInstance(antenna.get("type", "").asString());
				IAntenna* ant = new IsotropicAntenna();
				ant->setConfig(antenna);

				Matrix3d latitude = Matrix3d::createRotationMatrix(latitudeOffset, Matrix3d::ROTATION_X);
				Matrix3d longitude = Matrix3d::createRotationMatrix(longitudeOffset, Matrix3d::ROTATION_Z);
				Matrix3d rotationMatrix = latitude * longitude;

				Vector3d startPosition = rotationMatrix * Vector3d(0, (radius+2+beaconAltitude), 0);
				BOOST_LOG_TRIVIAL(debug) << startPosition;

				for(double azimuth = azimuthMin; azimuth <= azimuthMax; azimuth += azimuthStep) {

					Matrix3d azimuthRotation = Matrix3d::createRotationMatrix(azimuth * Constants::PI / 180, Matrix3d::ROTATION_Y);

					for (int freq = _fmin; freq <= _fmax; freq += _fstep) {
						for (double elevation = SZAmin; elevation <= SZAmax; elevation += SZAstep) {

							Ray r;
							r.rayNumber = ++rayCounter;
							r.frequency = (double)freq;
							r.signalPower = ant->getSignalPowerAt(azimuth, elevation);
							r.o = startPosition;
							r.originalAngle = elevation * Constants::PI / 180.0;
							r.originBeaconId = b+1;
							r.originalAzimuth = azimuth * Constants::PI / 180.0;
							Vector3d direction = Vector3d(cos(Constants::PI/2.0 - r.originalAngle),
									sin(Constants::PI/2.0 - r.originalAngle),
									0).norm();
							r.d = azimuthRotation * direction;

							Worker w;
							w.schedule(&tp, r);

							numWorkers++;
						}
					}
				}
			}

			BOOST_LOG_TRIVIAL(info) << numWorkers << " workers queued";
			if (_verbosity > boost::log::trivial::info) {
				std::ostringstream stringStream;
				stringStream << numWorkers << " workers queued";
				CommandLine::getInstance().addToHeader(stringStream.str().c_str());
			}

			tp.wait();

			flushScene();
		}

		stop();

		double t = tmr.elapsed();
		double tracingsPerSec = _numTracings / t;
		char buffer[80];
		CommandLine::getInstance().updateBody("\n");
	    sprintf(buffer, "Elapsed: %5.2f sec. %d tracings done. %5.2f tracings/sec",
	    		t, _numTracings, tracingsPerSec);
	    BOOST_LOG_TRIVIAL(warning) << buffer;

		//CsvExporter ce;
		//ce.dump("Debug/data.csv", dataSet);
		_exporter->dump(_outputFile, dataSet);

	    BOOST_LOG_TRIVIAL(warning) << "Results stored at: " << _outputFile;
	}

	void Application::stop() {

		_isRunning = false;
	}

	/**
	 * Add geometries to the scenemanager
	 */
	void Application::createScene() {

		_scm = SceneManager();
		_scm.loadStaticEnvironment();

		int numSceneObjectsCreated = 0;
		double R = _celestialConfig.getInt("radius");
		double angularStepSize = _applicationConfig.getDouble("angularStepSize");

		for (double latitude = 0 * Constants::PI; latitude <= 2 * Constants::PI; latitude += angularStepSize) {
			for (double longitude = 0 * Constants::PI; longitude <= 2 * Constants::PI; longitude += angularStepSize) {

				Matrix3d latitudeM = Matrix3d::createRotationMatrix(latitude, Matrix3d::ROTATION_X);
				Matrix3d longitudeM = Matrix3d::createRotationMatrix(longitude, Matrix3d::ROTATION_Z);
				Matrix3d rotationMatrix = latitudeM * longitudeM;

				Vector3d startPosition = rotationMatrix * Vector3d(0, R, 0);

				Plane3d mesh = Plane3d(startPosition.norm(), startPosition);
				mesh.size = angularStepSize * R;
				Terrain* tr = new Terrain(mesh);

				numSceneObjectsCreated++;
				_scm.addToScene(tr);
			}
		}

		if (numSceneObjectsCreated > 1e9)
			BOOST_LOG_TRIVIAL(info) << setprecision(3) << numSceneObjectsCreated/1.0e9 << "G scene objects created";
		else if (numSceneObjectsCreated > 1e6)
			BOOST_LOG_TRIVIAL(info) << setprecision(3) << numSceneObjectsCreated/1.0e6 << "M scene objects created";
		else if (numSceneObjectsCreated > 1e3)
			BOOST_LOG_TRIVIAL(info) << setprecision(3) << numSceneObjectsCreated/1.0e3 << "K scene objects created";
		else
			BOOST_LOG_TRIVIAL(info) << setprecision(3) << numSceneObjectsCreated << " scene objects created";
	}

	/**
	 * Flush the scene by clearing the list of scene objects
	 */
	void Application::flushScene() {

		_scm.removeAllFromScene();
	}

	void Application::addToDataset(Data dat) {

		datasetMutex.lock();
		dataSet.push_back(dat);
		if (dataSet.size() > Data::MAX_DATASET_SIZE) {
			_exporter->dump(_outputFile, dataSet);
			dataSet.clear();
		}
		datasetMutex.unlock();
	}

	void Application::incrementTracing() {

		tracingIncMutex.lock();
		_numTracings++;
		tracingIncMutex.unlock();
	}

	SceneManager Application::getSceneManager() {

		return _scm;
	}

	Config Application::getApplicationConfig() {

		return _applicationConfig;
	}

	Config Application::getCelestialConfig() {

		return _celestialConfig;
	}

	void Application::setCelestialConfig(Config conf) {

		_celestialConfig = conf;
	}

	void Application::setApplicationConfig(Config conf) {

		_applicationConfig = conf;
	}

	int Application::getVerbosity() {

		return _verbosity;
	}

	bool Application::includeMagneticFieldEffects() {

		return _includeMagneticField;
	}

	void Application::configureExporter() {

		std::string outputFileStr = string(_outputFile);
		int pos = outputFileStr.find_last_of(".");
		std::string fileext = outputFileStr.substr(pos+1);
		BOOST_LOG_TRIVIAL(info) << "Found the following file extension for export: " << fileext;
		if (fileext == "csv") {
			_exporterType = ExporterType::Csv;
			_exporter = new CsvExporter(_outputFile);
			BOOST_LOG_TRIVIAL(info) << "CSV Exporter selected.\n";
		} else if (fileext == "dat") {
			_exporterType = ExporterType::Matlab;
			_exporter = new MatlabExporter(_outputFile);
			BOOST_LOG_TRIVIAL(info) << "Matlab Exporter selected.\n";
		} else if (fileext == "vtk") {
			_exporterType = ExporterType::Matlab;
			_exporter = new VtkExporter(_outputFile);
			BOOST_LOG_TRIVIAL(info) << "VTK Exporter selected.\n";
		} else if (fileext == "json") {
			_exporterType = ExporterType::Matlab;
			_exporter = new JsonExporter(_outputFile);
			BOOST_LOG_TRIVIAL(info) << "Json Exporter selected.\n";
		}
	}

} /* namespace core */
} /* namespace raytracer */
