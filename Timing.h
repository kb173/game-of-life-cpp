#pragma once
#include <chrono>
#include <string>
#include <map>

/**
 * Measure high precision time intervals (using std::chrono).
 * Author: Karl Hofer <hoferk@technikum-wien.at>
 */
class Timing {
public:
	static Timing* getInstance();

	void startSetup();
	void stopSetup();
	void startComputation();
	void stopComputation();
	void startFinalization();
	void stopFinalization();

	void startRecord(const std::string& name);
	void stopRecord(const std::string& name);
	void print(const bool prettyPrint = false) const;
	std::string getResults() const;

private:
	Timing() {};
	std::map<std::string, std::chrono::high_resolution_clock::time_point > mRecordings;
	std::map<std::string, std::chrono::duration<double, std::milli> > mResults;
	std::string parseDate(const int ms) const;

	static Timing* mInstance;
};
