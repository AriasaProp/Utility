#include "matrix.hpp"
#include "simple_clock.hpp"

#include <fstream>

bool matrix_test (std::ofstream &o) {
  o << "Matrix code test" << std::endl;
  try {
    simple_timer_t cl, cls;

    matrix2D ma (2, 2, {1.0f, 2.0f, 3.0f, 4.0f});
    ma.print ();

    matrix2D mb (2, 3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
    mb.print ();

    matrix2D mc (3, 2, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
    mc.print ();

    matrix2D md (3, 3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f});
    md.print ();

    matrix2D mA = ma;
    matrix2D mB;
    mB.identity ();

    o << "Object Initialize : " << cl.end () << std::endl;

    // math  operator
    cl.start ();
    if ((ma + ma) != (mA += ma)) throw "Addition of Matrix was error!";
    o << "Addition : " << cl.end () << std::endl;

    cl.start ();
    if (
        ((mA - ma) != ma) ||
        ((mA -= ma) != ma)) throw "Subtract of Matrix was error!";
    o << "Subtract : " << cl.end () << std::endl;
    cl.start ();
    if ((ma * ma) != (mA *= ma)) throw "Multiply of Matrix was error!";
    o << "Multiply : " << cl.end () << std::endl;
    cl.start ();
    if (
        ((ma / ma) != mB) ||
        ((mA /= ma) == ma)) throw "Division of Matrix was error!";
    o << "Division : " << cl.end () << std::endl;
    o << "Matrix operation : " << cls.end () << std::endl;
  } catch (const char *e) {
    o << "error: " << e << std::endl;
    return false;
  }
  return true;
}