extern bool pi_extraction_test ();


bool Math_test () {
	bool passed = true;
	
	passed &= pi_extraction_test ();
	
	return passed;
}