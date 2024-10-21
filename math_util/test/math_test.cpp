extern bool extraction_test (const char *);

bool Math_test (const char *d) {
  bool passed = true;

  passed &= extraction_test (d);

  return passed;
}