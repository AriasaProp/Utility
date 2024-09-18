extern bool BigInteger_test ();
[[maybe_unused]]
extern bool matrix_test ();
[[maybe_unused]]
extern bool ComplexNumber_test ();

extern bool Math_test ();

int main (int argv, char *args[]) {
  bool passed = true;
  passed &= BigInteger_test ();
/*
  passed &= matrix_test ();
  passed &= ComplexNumber_test ();
*/
  passed &= Math_test ();

  return !passed;
}