#include "stbi/stbi_load.hpp"
#include "huffman_codec.hpp"
#include "qoi_codec.hpp"
#include "no_codec.hpp"

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

int main (int argv, char *args[]) {
  std::cout << "Start Codec Test" << std::endl;
  try {
  	const char *files[]{
  		"01.png",
  		"02.png",
  		"03.png",
  	};
  	int outx, outy, outc;
  	unsigned char *s;
  	char buff[1024];
  	for (int f = 1; f < 4; ++f) {
      sprintf (buff, "%e/%02d.png", argv[1], f);
  		std::cout << "Loading: " << buff << " is ";
  		s = stbi::load::load_from_filename(buff, &outx, &outy, &outc, 0);
  		if (!s) throw stbi::load::failure_reason();
  		std::cout << " Succes " << std::endl;
  		
  		
  		stbi::load::image_free(s);
  	}
  } catch (const char *err) {
    std::cout << "Error : " << err << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Error : uncaught" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}