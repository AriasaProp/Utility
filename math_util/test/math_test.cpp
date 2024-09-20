extern bool pi_extraction_test (char *);

bool Math_test (char *d) {
  bool passed = true;

  passed &= pi_extraction_test (d);

  return passed;
}