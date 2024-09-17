extern bool pi_extraction_test (const char *);

bool Math_test (const char *o) {
  bool passed = true;

  passed &= pi_extraction_test (o);

  return passed;
}