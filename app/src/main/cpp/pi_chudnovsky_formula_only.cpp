#include <cmath>
#include <iostream>

using namespace std;

double piByChudnovsky(unsigned long n)
{
	double pi = 0.0, i, ia;
	for (int j = 0; j <= n; j++)
	{
		ia = 1;
		for (i = 3 * j + 1; i <= 6 * j; i++)
			ia *= i;
		for (i = 2; i <= j; i++)
			ia /= i * i * i;
		pi += ia * (1359140.90 + 54514013.40 * j) / pow(-640320.0, 3.0 * j);
	}
	return 42688.0 * sqrt(10005) / pi;
}

int main(int argc, char *argv[])
{
	cout.precision(99999999999);
	cout << piByChudnovsky(6) << endl;

	FILE *input_file = fopen("/sdcard/Download/one-million-pi.txt", "r");
	if (!input_file)
	{
		cout << "failed load Pi Sample" << endl;
		return EXIT_FAILURE;
	}
	for (int i = 0; i < 100; i++)
	{
	cout << getc(input_file) - '0';
	if (i == 0)
		cout << ".";
	}
	cout << endl;

	fclose(input_file);
	delete (input_file);
}