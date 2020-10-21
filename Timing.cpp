#include <stdio.h>
#include <iostream>
#include <sstream>
#include "Timing.h"

Timing* Timing::mInstance = 0;

/**
 * Singleton: Get instance.
 */
Timing* Timing::getInstance() {
	if (mInstance == 0) {
		mInstance = new Timing();
	}

	return mInstance;
}

/**
 * Start recording time with any name.
 */
void Timing::startRecord(const std::string& name) {
	auto start = std::chrono::high_resolution_clock::now();

	auto it = mRecordings.find(name);
	if (it != mRecordings.end()) {
		it->second = start;
	} else {
		mRecordings.insert(std::pair<std::string, std::chrono::high_resolution_clock::time_point>(name, start));
	}

}

/**
 * Stop recording time with any name.
 */
void Timing::stopRecord(const std::string& name) {
	auto end = std::chrono::high_resolution_clock::now();

	auto it = mRecordings.find(name);
	if (it != mRecordings.end()) {
		auto start = it->second;
		auto result = end - start;

		mResults.insert(std::pair<std::string, std::chrono::duration<double, std::milli> >(name, result));
	}

}

/**
 * Print measured results human-readable.
 * Set prettyPrint to true to display mm:ss.ms instead of ms.
 */
void Timing::print(const bool prettyPrint) const {
	std::cout << "-----" << std::endl << "Results: " << std::endl << "-----" << std::endl;

	auto it = mResults.begin();
	while(it != mResults.end()) {
		if (prettyPrint) {
			std::cout << it->first << ": " << parseDate((int) it->second.count()) << std::endl;
		} else {
			std::cout << it->first << ": " << it->second.count() << "ms" << std::endl;
		}
		it++;
	}

	std::cout << "-----" << std::endl;
}

/**
 * Parse date from ms to mm:ss.ms.
 */
std::string Timing::parseDate(const int ms) const {
	int minutes = (int) (ms / 1000 / 60);
	int seconds = (int) ((ms % (1000 * 60)) / 1000);
	int milliseconds = (int) (ms % 1000);

	std::ostringstream stringStream;
	if (seconds == 60) {
		stringStream << minutes + 1 << ":00" << seconds << ".";
	} else {
  	stringStream << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << ".";
	}

	if (milliseconds < 100) {
		if (milliseconds < 10) {
			stringStream << "00" << milliseconds;
		} else {
			stringStream << "0" << milliseconds;
		}
	} else {
		stringStream << milliseconds;
	}

  return stringStream.str();
}

/**
 * Get results of setup, computation and finalization in form:
 * mm:ss.ms;mm:ss.ms;mm.ss.ms
 */
std::string Timing::getResults() const {
	std::ostringstream stringStream;

	auto start = mResults.find("setup");
	if (start != mResults.end()) {
		stringStream << parseDate((int) start->second.count()) << ";";
	}

	auto computation = mResults.find("computation");
	if (computation != mResults.end()) {
		stringStream << parseDate((int) computation->second.count()) << ";";
	}

	auto finalization = mResults.find("finalization");
	if (start != mResults.end()) {
		stringStream << parseDate((int) finalization->second.count());
	}

	return stringStream.str();
}

/**
 * Start recording the setup time.
 */
void Timing::startSetup() {
	this->startRecord("setup");
}

/**
 * Stop recording the setup time.
 */
void Timing::stopSetup() {
	this->stopRecord("setup");
}

/**
 * Start recording the computation time.
 */
void Timing::startComputation() {
	this->startRecord("computation");
}

/**
 * Stop recording the computation time.
 */
void Timing::stopComputation() {
	this->stopRecord("computation");
}

/**
 * Start recording the finalization time.
 */
void Timing::startFinalization() {
	this->startRecord("finalization");
}

/**
 * Stop recording the finalization time.
 */
void Timing::stopFinalization() {
	this->stopRecord("finalization");
}