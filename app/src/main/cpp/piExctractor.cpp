#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <string>
#include <complex>
#include <algorithm>
#include <memory>
#include <vector>
#include "../include/utils/big_decimal.h"

using namespace std;
using namespace chrono;

double wall_clock()
{
	auto ratio_object = chrono::high_resolution_clock::period();
	double ratio = (double)ratio_object.num / ratio_object.den;
	return chrono::high_resolution_clock::now().time_since_epoch().count() * ratio;
}

int main(int argc, char *argv[])
{
	cout << "my own pi generator version 07/06/2022 (1)" << endl;
	FILE *input_file = fopen("piDigits.txt", "r");
	if (!input_file)
	{
		cout << "failed load Pi Sample" << endl;
		return EXIT_FAILURE;
	}

	big_decimal *const phiSets = new big_decimal[7];
	long *const paramsOut = new long[3];
	thread *const pThread = new thread[3];
	paramsOut[0] = 0;
	phiSets[2] = 65536;
	double start = wall_clock();
	cout << "start tracking" << endl;
	cout.precision(10);
	bool running = true;
	double digitRate = 0, lastTime = start;
	pThread[2] = thread([&]() {
		do
		{
			cout << '\r' << flush;
			cout << "pi Tracking correcrt : " << paramsOut[0] << " rate : " << digitRate << " seconds per digit    ";
		} while (digitRate);
	});
	try
	{
		for (phiSets[5] = 0, phiSets[6] = 0; paramsOut[0] < 30 /*!feof(input_file)*/; phiSets[5] += 16, phiSets[6] += 40)
		{
			//set next value to become add in pi valuel
			//rcp is equal with 1/x value
			pThread[0] = thread([&]() {
				big_decimal
					&d = *(phiSets + 2),
					&o = *(phiSets + 3),
					&n1 = *(phiSets + 5),
					&n2 = *(phiSets + 6);
				o =  (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 262144;
				o -= (n2 + 1) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 65536;
				o -= (n2 + 1) * (n2 + 3) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 32768;
				o -= (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 4096;
				o -= (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 4096;
				o += (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 1024;
				o -= (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 1024;
				o -=  (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 256;
				o +=  (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 64;
				o += (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 32;
				o +=  (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 17) * (n1 + 7) * (n2 + 19) * 4;
				o +=  (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n1 + 7) * (n2 + 19) * 4;
				o += (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n2 + 19);
				o -=  (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7);
				o /= d * (n2 + 1) * (n2 + 3) * (n1 + 1) * (n2 + 5) * (n2 + 7) * (n2 + 9) * (n1 + 3) * (n2 + 11) * (n2 + 13) * (n1 + 5) * (n2 + 15) * (n2 + 17) * (n1 + 7) * (n2 + 19);
			});
			pThread[1] = thread([&]() {
				big_decimal
					&d = *(phiSets + 2),
					&o = *(phiSets + 4),
					&n1 = *(phiSets + 5),
					&n2 = *(phiSets + 6);
				o =  (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 262144;
				o -= (n2 + 21) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 65536;
				o -=(n2 + 21) * (n2 + 23) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 32768;
				o -= (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 4096;
				o -= (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 4096;
				o += (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 1024;
				o -= (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 1024;
				o -= (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 256;
				o += (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 64;
				o += (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 32;
				o += (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 37) * (n1 + 15) * (n2 + 39) * 4;
				o += (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n1 + 15) * (n2 + 39) * 4;
				o += (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n2 + 39);
				o -= (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15);
				o /= d * 1048576 * (n2 + 21) * (n2 + 23) * (n1 + 9) * (n2 + 25) * (n2 + 27) * (n2 + 29) * (n1 + 11) * (n2 + 31) * (n2 + 33) * (n1 + 13) * (n2 + 35) * (n2 + 37) * (n1 + 15) * (n2 + 39);
			});
			pThread[0].join(); 
			pThread[1].join();

			phiSets[1] = phiSets[3] + phiSets[4];
			phiSets[2] *= 1048576; //2^20
			phiSets[2] *= 1048576; //2^20
			
			double none;
			while (phiSets[1].count_decimal_front(none) < -3)
			{
				paramsOut[1] = phiSets[0].word_at(0);
				paramsOut[2] = long(getc(input_file) - '0');
				if (paramsOut[1] != paramsOut[2])
				{
					cout << endl;
					throw "pi fail";
				}
				phiSets[0] = phiSets[0] - paramsOut[1];
				paramsOut[0]++;
				double cnok = wall_clock();
				digitRate = (digitRate + (cnok - lastTime)) / 2.;
				lastTime = cnok;
				phiSets[0] *= 10;
				phiSets[1] *= 10;
				phiSets[2] /= 10;
			}
			phiSets[0] = phiSets[0] + phiSets[1];
		}
	}
	catch (const char error[])
	{
		cout << error << endl;
		cout << "result is " << paramsOut[1] << " that should be " << paramsOut[2] << endl;
	}
	running = false;
	cout << endl;
	cout << "phi correct : " << paramsOut[0] << endl;
	cout << "time : " << (wall_clock() - start) << " s" << endl;
	fclose(input_file);
	return 0;
}