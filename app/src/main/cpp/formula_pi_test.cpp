#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>

using namespace std;

typedef uintmax_t ul;
typedef long double ld;

// newton
// fabrice

struct rcp
{
  public:
    ul nominator = 0;
    ul denominator = 0;

    rcp(){}
    rcp(ul n, ul d = 1) : nominator(n), denominator(d) {}

    rcp &operator=(const rcp &x)
    {
        this->nominator = x.nominator;
        this->denominator = x.denominator;
        return *this;
    }
    rcp &operator*=(const rcp &x)
    {
        this->nominator *= x.nominator;
        this->denominator *= x.denominator;
        return *this;
    }
    rcp &operator/=(const rcp &x)
    {
        this->nominator /= x.nominator;
        this->denominator /= x.denominator;
        return *this;
    }
};

int main()
{
    cout.precision(56);
    {
        vector<rcp> rcps;
        ul n = 0, val = 2, dval = 1, c, a, b;
        ul facn = 2;

        do
        {
            // 63*n*n+78*n+22
            // 2 * (3*n)! * (
            //    (6*n+3) * (6*n+5) +
            //    (3*n+1) * (6*n+5) +
            //    (3*n+1) * (3*n+2)
            // )
            // ÷
            // @(6*n+5)

            //
            if (n)
            {
                facn *= n * n * 27 * (n - 1) + 6 * n;
                /*
                val /= (6 * n - 5);
                dval /= (6 * n - 5);
                */
            }
            val = facn * (63 * n * n + 78 * n + 22);
            dval *= (6 * n + 1) * (6 * n + 3) * (6 * n + 5);

            a = val, b = dval;
            while (b)
            {
                c = a;
                a = b;
                b = c % b;
            }
            if (a > 1)
                val /= a, dval /= a;

            cout << "simpled by " << a << endl;
            //cout << val << "/" << dval << endl;
            pi = (pi * dval + val) / dval;
            //cout << pi << endl;
        } while (pi < 4 && n++ < 50);
    }
    cout << "done!" << endl;

    /*
    {
        long double pi = 0;
        ul n = 0;
        do
        {
            pi += 4. / (20 * n + 1) / pow(2, 20 * n);
            pi -= 1. / (20 * n + 3) / pow(2, 20 * n);
            pi -= 1. / (20 * n + 5) / pow(2, 20 * n + 4);
            pi -= 1. / (20 * n + 7) / pow(2, 20 * n + 4);
            pi += 1. / (20 * n + 9) / pow(2, 20 * n + 6);
            pi -= 1. / (20 * n + 11) / pow(2, 20 * n + 8);
            pi += 1. / (20 * n + 13) / pow(2, 20 * n + 10);
            pi += 1. / (20 * n + 15) / pow(2, 20 * n + 14);
            pi += 1. / (20 * n + 17) / pow(2, 20 * n + 14);
            pi -= 1. / (20 * n + 19) / pow(2, 20 * n + 16);

            pi -= 1. / (8 * n + 1) / pow(2, 20 * n + 1);
            pi -= 1. / (8 * n + 3) / pow(2, 20 * n + 6);
            pi += 1. / (8 * n + 5) / pow(2, 20 * n + 11);
            pi += 1. / (8 * n + 7) / pow(2, 20 * n + 16);

            cout << pi << endl;
        } while (++n < 9);
    }
    cout << "done!" << endl;
    */
    return 0;
}