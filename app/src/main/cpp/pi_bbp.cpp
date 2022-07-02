/***************************************************************
 * Computing pi with BBP formula.
 **************************************************************/
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class Bbp
{
    // Declaration
    int     d;                          // Digits to compute
    double  pi;                         // Pi
    char    pi_hex[14];                 // Pi(Hex)
    clock_t t0, t1;                     // Time
    double  S(int);                     // Compute S
    long    compModExp(int, int, int);  // Computer Modular Exponentiation
    void    convHex(double, char[]);    // Convert Pi to Hex-string

public:
    Bbp(int);            // Constructor
    void compPi();       // Compute PI
};

/*
 * Constructor
 */
Bbp::Bbp(int d)
{
    cout << "**** PI Computation ( digit: " << d << " )" << endl;
    this->d = d - 1;
}

/*
 * Compute PI
 */
void Bbp::compPi()
{
    // Time (start)
    t0 = clock();

    // Compute Pi
    pi = 4.0 * S(1) - 2.0 * S(4) - S(5) - S(6);
    pi = pi - (int)pi + 1.0;
    convHex(pi, pi_hex);
    printf("FRACTION  : %.15f\n",   pi);
    printf("HEX DIGITS: %10.10s\n", pi_hex);

    // Time (end of computation)
    t1 = clock();
    cout << "( TIME: " << (double)(t1 - t0) / CLOCKS_PER_SEC
         << " seconds )" << endl;
}

/*
 * Compute S
 */
double Bbp::S(int j)
{
    double s = 0.0;        // Summation of Total, Left
    double t;              // Each term of right summation
    int    r;              // Denominator
    int    k;              // Loop index
    double EPS = 1.0e-17;  // Loop-exit accuration of the right summation

    // Left Sum (0 ... d)
    for (k = 0; k <= d; k++) {
        r = 8 * k + j;
        t = (double)compModExp(16, d - k, r);
        t /= r;
        s += t - (int)t;
        s -= (int)s;
    }

    // Right sum (d + 1 ...)
    while (1) {
        r = 8 * k + j;
        t = pow(16.0, (double)(d - k));
        t /= (double)r;
        if (t < EPS) break;
        s += t;
        s -= (int)s;
        k ++;
    }

    return s;
}

/*
 * Compute Modular Exponentiation
 */
long Bbp::compModExp(int b, int e, int m)
{
    long ans;

    if (e == 0) return 1;

    ans = compModExp(b, e / 2, m);
    ans = (ans * ans) % m;
    if ((e % 2) == 1) ans = (ans * b) % m;

    return ans;
}

/*
 * Convert Pi to Hex-strings
 */
void Bbp::convHex(double x, char chx[]) {
    double y;
    int i;
    const char hx[] = "0123456789ABCDEF";

    y = fabs(x);
    for (i = 0; i < 16; i++) {
        y = 16.0 * (y - floor(y));
        chx[i] = hx[(int)(y)];
    }
}

int main()
{
    try
    {
        // Instantiation
        Bbp objMain(999);

        // Compute PI
        objMain.compPi();
    }
    catch (...) {
        cout << "ERROR!" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
