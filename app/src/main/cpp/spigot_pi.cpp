#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
//bigger primitive type
typedef uintmax_t ul;
typedef long double ld;
//smallest primitive type
typedef unsigned char uc;

int spigot_main(void)
{
    ul start = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    ul restart = start;
    std::cout.precision(10);
    std::FILE *file = std::fopen("piDigits.txt", "r");
    if (!file)
    {
        std::cout << "failed load pi samples" << std::endl;
        return EXIT_FAILURE;
    }
    ul endCount = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - restart;
    std::cout << "file load need " << endCount << " s" << std::endl;
    restart = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    const ul len = 3333222;//16711567; //can be more than this for more correct pi digits
    //safe digit for temp result, predigit, nines, result, check;
    ul *const A = new ul[len];
    if (!A)
    {
        std::cout << "failed allocate an memory" << std::endl;
        return EXIT_FAILURE;
    }
    try
    {
        bool ungrowth = true;
        uc tempResult, predigit, nines, result, check;
        ul i, w, x, correct;
        // first iteration
        {
            ul valA[10] = {0, 2, 2, 4, 3, 10, 1, 13, 12, 1};
            memcpy(A, valA, 10 * sizeof(ul));
            std::fill(A + 10, A + (len - 1), 20);
            predigit = 3;
        }
        endCount = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - restart;
        std::cout << "memory alloc and init need " << endCount << " s" << std::endl;
        restart = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

        // next iteration is in loop
        while (!std::feof(file))
        {
            tempResult = x = 0;
            i = len - 1;
            w = 2 * len - 1;
            do
            {
                x += 10 * A[i];
                tempResult = x / w;
                A[i] = x % w;
                x = tempResult * i;
                --i, w -= 2;
            } while (i);
            x += 10 * A[0];
            A[0] = tempResult % 10;
            tempResult = x / 10;

            if (tempResult == 9)
            {
                ++nines;
                continue;
            }
            ungrowth = tempResult != 10;
            result = predigit + !ungrowth;
            nines++;
            do
            {
                check = uc(std::getc(file) - '0');
                if (result != check)
                {
                    std::cout << std::endl;
                    std::cout << char('0' + result) << " != " << char('0' + check) << std::endl;
                    throw "pi wrong with result";
                }
                endCount = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - restart;
                std::cout << std::flush << '\r' << std::flush << " corrected : " << std::setw(10) << std::setfill('0') << ++correct << " rate : " << std::setw(3) << std::setfill('0') << (correct / endCount) << " digits/sec" << std::setw(5) << std::setfill(' ');
                result = ungrowth * 9;
            } while (--nines);
            predigit = ungrowth * tempResult;
        }

        throw "end of file correction";
    }
    catch (const char *report)
    {
        std::cout << std::endl;
        std::cout << report << std::endl;
    }
    delete[] A;
    std::fclose(file);
    delete (file);
    endCount = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() - start;
    std::cout << "all process need " << endCount << " s" << std::endl;
    return EXIT_SUCCESS;
}