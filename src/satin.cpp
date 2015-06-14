// Program satin.cpp by Alan K Stewart
// Saturation Intensity Calculation

#include <fstream.h>
#include <iostream.h>
#include <iomanip.h>
#include <math.h>
#include <new.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#define N    8
#define RAD  0.18
#define W1   0.3
#define DR   0.002
#define DZ   0.04
#define AREA (M_PI * (RAD * RAD))
#define Z1   (M_PI * (W1 * W1) / 0.0106)

class Satin {

public:
	void startProcessing();
	int getInputPowers(int inputPowerData[]);
	int getLaserData(float smallSignalGain[], char outputFile[][9], char dischargePressure[][3], char carbonDioxide[][3]);
	void gaussianCalculation(int inputPower, float smallSignalGain, char outputFile[]);
	static void noMemory();

	Satin::Satin() {}
	Satin::~Satin() {}
};


void main() {
	Satin satin;
	satin.startProcessing();
}


void Satin::startProcessing() {
	time_t the_time;
	ofstream fd;
	char outputFile[N][9], dischargePressure[N][3], carbonDioxide[N][3];
	double startTime, timeDifference;
	float smallSignalGain[N];
	int pNum, lNum, inputPowerData[N];

	startTime = (double) clock() / CLOCKS_PER_SEC;

	pNum = getInputPowers(inputPowerData);
	lNum = getLaserData(smallSignalGain, outputFile, dischargePressure, carbonDioxide);

	for (int i = 0; i < lNum; i++) {
		fd.open(outputFile[i], ios::out);

		if (fd.fail()) {
			cout << "Error opening " << outputFile[i] << "\n";
			exit(1);
		}

		time(&the_time);
		fd << "Start date: "
		   << ctime(&the_time)
		   << "\nGaussian Beam\n\n"
		   << "Pressure in Main Discharge = "
		   << dischargePressure[i]
		   << "kPa\n"
		   << "Small-signal Gain = "
		   << smallSignalGain[i]
		   << "\nCO2 via "
		   << carbonDioxide[i]
		   << "\n\nPin\t\tPout\t\tSat. Int.\tln(Pout/Pin)\tPout-Pin\n"
		   << "(watts)\t\t(watts)\t\t(watts/cm2)\t\t\t(watts)\n";

		fd.flush();
		fd.close();

		for (int j = 0; j < pNum; j++) {
			gaussianCalculation(inputPowerData[j], smallSignalGain[i], outputFile[i]);
		}

		fd.open(outputFile[i], ios::in | ios::app);
		time(&the_time);
		fd << "\nEnd date: " << ctime(&the_time);

		fd.flush();
		fd.close();

	}

	timeDifference = ((double) clock() / CLOCKS_PER_SEC) - startTime;
	cout << "The CPU time was "
		 << setiosflags(ios::fixed)
		 << setprecision(2)
		 << timeDifference
		 << " seconds.\n";
}

int Satin::getInputPowers (int inputPowerData[]) {
	ifstream fd;
	char* inputPowerFile = "pin.dat";
	int i;

	fd.open(inputPowerFile, ios::in | ios::nocreate);

	if (fd.fail()) {
		cout << "Error opening " << inputPowerFile << "\n";
		exit(1);
	}

	for (i = 0; i < N; i++) {
		fd >> inputPowerData[i];

		if (fd.eof()) {
			break;
		}
	}

	fd.close();
	return i;
}

int Satin::getLaserData(float smallSignalGain[], char outputFile[][9], char dischargePressure[][3], char carbonDioxide[][3]) {
	ifstream fd;
	char* gainMediumDataFile = "laser.dat";
	int i;

	fd.open(gainMediumDataFile, ios::in | ios::nocreate);

	if (fd.fail()) {
		cout << "Error opening " << gainMediumDataFile << "\n";
		exit(1);
	}

	for (i = 0; i < N; i++) {
		fd >> outputFile[i];
		fd >> smallSignalGain[i];
		fd >> dischargePressure[i];
		fd >> carbonDioxide[i];

		if (fd.eof()) {
			break;
		}
	}

	fd.close();
	return i;
}

void Satin::gaussianCalculation(int inputPower, float smallSignalGain, char outputFile[]) {
	ofstream fd;
	double temp;
	double *expr;
	double zInc;
	double inputIntensity;
	double outputIntensity;
	double outputPower;
	float radius;
	int saturationIntensity;

	fd.open(outputFile, ios::in | ios::app);
	inputIntensity = 2 * inputPower / AREA;

	set_new_handler(noMemory);
	expr = new double[8 * 8001];

	for (int i = 0; i < 8001; i++) {
		zInc = ((double) i - 4000) / 25;
		expr[i] = zInc * 2 * DZ / (Z1 * Z1 + zInc * zInc);
	}

	for (saturationIntensity = 10000; saturationIntensity <= 25000; saturationIntensity += 1000) {
		outputPower = 0;
		temp = (double) saturationIntensity * (smallSignalGain / 32000) * DZ;

		for (radius = 0; radius <= 0.5; radius += DR) {
			outputIntensity = inputIntensity * exp(-2 * (radius * radius) / (RAD * RAD));

			for (int j = 0; j < 8001; j++) {
				outputIntensity *= (1 + temp / ((double) saturationIntensity + outputIntensity) - expr[j]);
			}

			outputPower += (outputIntensity * 2 * M_PI * radius * DR);
		}

		fd << setiosflags(ios::fixed)
		   << setprecision(1)
		   << inputPower
		   << "\t\t"
		   << setprecision(3)
		   << outputPower
		   << setiosflags(ios::fixed)
		   << setprecision(1)
		   << "\t\t"
		   << saturationIntensity
		   << "\t\t"
		   << setprecision(3)
		   << log(outputPower / inputPower)
		   << "\t\t"
		   << (outputPower - inputPower)
		   << "\n";
	}

	fd.flush();
	fd.close();
	delete expr;
}

void Satin::noMemory() {
	cerr << "\nThe free store is empty\n";
	exit(1);
}