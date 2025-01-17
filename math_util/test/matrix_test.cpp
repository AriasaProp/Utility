#include "clock_adjustment.hpp"
#include "matrix.hpp"

#include <fstream>

bool matrix_test (std::ofstream &o) {
  o << "Matrix code test" << std::endl;
  bool result = true;
  try {
    profiling::clock_adjustment _clock ("Matrix Operator Test");
    matrix2D ma (2, 2, {1.0f, 2.0f, 3.0f, 4.0f});
    ma.print ();

    matrix2D mb (2, 3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
    mb.print ();

    matrix2D mc (3, 2, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f});
    mc.print ();

    matrix2D md (3, 3, {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f});
    md.print ();

    matrix2D mA = ma;

    o << "Object Initialize : " << _clock.get_clock (profiling::clock_adjustment::period::microseconds) << " us " << std::endl;

    // math  operator
    result &= (ma + ma) == (mA += ma);
    if (!result) throw ("Addition of Matrix was error!");
    o << "Addition : " << _clock.get_clock (profiling::clock_adjustment::period::microseconds) << " us " << std::endl;

    result &= (mA - ma) == ma;
    result &= (mA -= ma) == ma;
    if (!result) throw ("Subtract of Matrix was error!");
    o << "Subtract : " << _clock.get_clock (profiling::clock_adjustment::period::microseconds) << " us " << std::endl;

    result &= (ma * ma) == (mA *= ma);
    if (!result) throw ("Multiply of Matrix was error!");
    o << "Multiply : " << _clock.get_clock (profiling::clock_adjustment::period::microseconds) << " us " << std::endl;
    {
      matrix2D mb = ma;
      mb.identity ();
      result &= (ma / ma) == mb;
    }
    result &= (mA /= ma) == ma;
    if (!result) throw ("Division of Matrix was error!");
    o << "Division : " << _clock.get_clock (profiling::clock_adjustment::period::microseconds) << " us " << std::endl;
  } catch (const char *e) {
    o << "error: " << e << std::endl;
    return false;
  }
  return result;
}