#include "matrix.hpp"
#include "simple_clock.hpp"

#include <ostream>

extern std::ostream *output_file;

bool matrix_test () {
  *output_file << "Matrix code test" << std::endl;
  try {
    simple_timer_t cl, cls;

    matrix2D ma (2, 2, {1.0f, 2.0f, 3.0f, 4.0f});
    *output_file << ma << std::endl;

    matrix2D mb (2, 3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
    *output_file << mb << std::endl;

    matrix2D mc (3, 2, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
    *output_file << mc << std::endl;

    matrix2D md (3, 3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f});
    *output_file << md << std::endl;

    matrix2D mA = ma;
    matrix2D mB;
    mB.identity ();

    *output_file << "Object Initialize : " << cl.end () << std::endl;

    // math  operator
    cl.start ();
    if ((ma + ma) != (mA += ma)) throw "Addition of Matrix was error!";
    *output_file << "Addition : " << cl.end () << std::endl;

    cl.start ();
    if (
        ((mA - ma) != ma) ||
        ((mA -= ma) != ma)) throw "Subtract of Matrix was error!";
    *output_file << "Subtract : " << cl.end () << std::endl;
    cl.start ();
    if ((ma * ma) != (mA *= ma)) throw "Multiply of Matrix was error!";
    *output_file << "Multiply : " << cl.end () << std::endl;
    cl.start ();
    if (
        ((ma / ma) != mB) ||
        ((mA /= ma) == ma)) throw "Division of Matrix was error!";
    *output_file << "Division : " << cl.end () << std::endl;
    *output_file << "Matrix operation : " << cls.end () << std::endl;
  } catch (const char *e) {
    std::cerr << "Matrix code error: " << e << std::endl;
    return false;
  }
  return true;
}