#include "math/ComplexNumber.hpp"
#include "simple_profiling.hpp"
#include "common.hpp"

#include <ostream>
#include <iostream>

extern std::ostream *output_file;
extern char *text_buffer;
extern const char *data_address;

void ComplexNumber_test(){
  *output_file << "ComplexNumber Test -> ";
  simple_timer_t ct;
  // do calculation
  sprintf(text_buffer, "%s/ComplexNumberTest.txt", data_address);
  FILE *file = fopen(text_buffer, "r");
  if (file) {
		size_t cnt= 0;
    try {
  		ct.start();
  		int ch = 0;
      while((ch = fgetc(file)) != EOF) {
        ++cnt;
      	switch (ch) {
      		case 'A': {
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    UNUSED(fgetc(file));
  			  	if (A+B != C) {
  			  	  *output_file << "result: " << (A+B) << " should be " << C;
  			  		throw text_buffer;
  			  	}
      			break;
      		}
      		case 'B': {
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    UNUSED(fgetc(file));
  			  	if (A-B != C) {
  			  	  *output_file << "result: " << (A-B) << " should be " << C;
  			  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'C': {
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    UNUSED(fgetc(file));
  			  	if ((A*B) != C) {
  			  	  *output_file << "result: " << (A*B) << " should be " << C;
    		  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'D': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    UNUSED(fgetc(file));
  			  	if (A/B != C) {
  			  	  *output_file << "result: " << (A/B) << " should be " << C;
  			  		throw text_buffer;
      		  }
      			break;
      		}
      		case 'E': {
      		  if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber A(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber B(text_buffer);
      	    if (!fscanf(file, " %s", text_buffer))
      	      throw "format test wrong!";
      	    ComplexNumber C(text_buffer);
      	    if (!fscanf(file, " #%[^\n]", text_buffer))
      	      throw "format test wrong!";
      	    UNUSED(fgetc(file));
  			  	if ((A^B) != C) {
  			  	  *output_file << "result: " << (A^B) << " should be " << C;
  			  		throw text_buffer;
      		  }
      			break;
      		}
      		default:
      			throw "unexpected value";
      	}
      }
      *output_file << ct.end();
    } catch (const std::exception *msg) {
      *output_file << "Error " << cnt << " -> " << msg->what();
    } catch (const char *msg) {
      *output_file << "Error " << cnt << " -> " << msg;
    }
    fclose(file);
  } else {
    *output_file << "file I/O error! at " << text_buffer;
  }
  *output_file << "\n";
}