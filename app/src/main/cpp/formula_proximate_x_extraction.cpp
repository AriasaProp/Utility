#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

using namespace std;

struct ra
{
  public:
	double A, B;
	ra(double a, double b)
	{
		A = a;
		B = b;
	}
};
const unsigned long parm = 11;
const double maxINT = 1e3;
double pi;
ra piChu() {
	pi = 0.0;
	double i, ia;
	for (int j = 0; j <= parm; j++)
	{
		ia = 1;
		for (i = 3 * j + 1; i <= 6 * j; i++)
			ia *= i;
		for (i = 2; i <= j; i++)
			ia /= i * i * i;
		pi += ia * (1359140.90 + 54514013.40 * j) / pow(-262537412640768000.0, j);
	}
	return ra(42688.0*sqrt(10005.0), pi);
}
ra piBBP() {
	pi = 0.0;
	for (unsigned long i = 0; i <= parm; i++) {
		pi += maxINT *  (((16.0 * i + 14.0) * (64.0 * i * i + 88.0 * i + 30.0) - (16.0 * i + 11.0) * (64.0 * i * i + 40.0 * i + 4.0)) / ((64.0 * i * i + 40.0 * i + 4.0) * (64.0 * i * i + 88.0 * i + 30.0)) ) / pow(16.0, i);
	}
	return ra( pi, maxINT);
}
const ra ter[2] = {piChu(), piBBP()};
int main(int argc, char *argv[]) {
	unsigned int ch, result;
	double curr;
	for (int i = 0; i < sizeof(ter) / sizeof(ra); i++) {
		const ra re = ter[i];
		FILE *input_file = fopen("/sdcard/Download/million-digits-of-Pi.txt", "r");
		if (input_file == nullptr)
			return EXIT_FAILURE;
		curr = re.A;
		printf("%02d : ", i);
		while (!feof(input_file) || curr != 0) {
			result = 0;
			while (curr > re.B) {
				curr -= re.B;
				result++;
			}
			ch = getc(input_file) - '0';
			if (ch != result) {
				printf(" end %d[%d]\n", result, ch);
				break;
			}
			printf("%d", result);
			curr *= 10.0;
		}
		fclose(input_file);
	}
}