extern bool clock_adjustment_test ();

int main (int argv, char *args[]) {
  bool passed = true;

  passed &= clock_adjustment_test ();

  return !passed;
}