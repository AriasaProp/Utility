#include "math/QuickMath.hpp"
#include "simple_profiling.hpp"
#include "common.hpp"

#include <iomanip>
#include <ostream>
#include <cstdio>
#include <cmath>

extern std::ostream *output_file;
extern char *text_buffer;
extern const char *data_address;

void QuickMath_test() {
  *output_file << "QuickMath: ";
  simple_timer_t ct, ctm;
	int ch;
	float a, b, curr;
  unsigned long cnt = 0;
  struct data {
    const char *label;
    size_t total;
    float max, avg;
    simple_time_t duration;
  } datas[] = {
    {"sqrt^-1", 0, 0.0f, 0.0f, simple_time_t{0}},
    {"cosine", 0, 0.0f, 0.0f, simple_time_t{0}}
  };
  
  sprintf(text_buffer, "%s/QuickMathTest.txt", data_address);
  FILE *file = fopen(text_buffer, "r");
  if (!file) {
    *output_file << "file I/O error! at " << text_buffer;
    goto test_end;
  }
  try {
    ct.start();
    while((ch = fgetc(file)) != EOF) {
      cnt++;
    	switch (ch) {
    		case 'A': {
          if (!fscanf(file, "%f %f", &a, &b)) throw "format test wrong!";
          UNUSED(fgetc(file));
          ctm.start();
          curr = fabs(QuickMath::inverse_sqrt(a) - b) * 100.0f / b;
          datas[0].duration += ctm.end();
          datas[0].max = MAX(curr, datas[0].max);
          datas[0].avg += curr;
          ++datas[0].total;
    			break;
    		}
    		default:
    			throw "unexpected value";
    	}
    }
    /*
    for (float trig = 0.0f; trig < 6.283185307; trig += 0.05f) {
      ctm.start();
      b = sin(trig);
      curr = fabs(QuickMath::rad_sin(trig) - b) * 100.0f / b;
      datas[1].duration += ctm.end();
      datas[1].max = MAX(curr, datas[1].max);
      datas[1].avg += curr;
      ++datas[1].total;
      ++cnt;
      ctm.start();
      b = cos(trig);
      curr = fabs(QuickMath::rad_cos(trig) - b) * 100.0f / b;
      datas[1].duration += ctm.end();
      datas[1].max = MAX(curr, datas[1].max);
      datas[1].avg += curr;
      ++datas[1].total;
      ++cnt;
    }
    */
    *output_file << cnt << " tests for " << ct.end() << 
    "\n         | test | err avg | err max |   time   |"
    "\n---------|------|---------|---------|----------|";
    for (data d : datas) {
      *output_file << "\n "
        << std::setfill (' ') << std::setw ( 8) << d.label << "|"
        << std::setfill ('0') << std::setw ( 6) << d.total << "|"
        << std::setfill ('0') << std::setw ( 8) << std::setprecision (4) << (d.avg / (float)d.total) << "%|"
        << std::setfill ('0') << std::setw ( 8) << std::setprecision (4) << d.max << "%|"
        << std::setfill (' ') << std::setw (10) << d.duration << "|";
    }
  } catch (const char *msg) {
    *output_file << "Error -> " << msg;
  } catch (...) {
    *output_file << "Error unknown!";
  }
  fclose(file);
test_end:
  *output_file << std::endl;
}