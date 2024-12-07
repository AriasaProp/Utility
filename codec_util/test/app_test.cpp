#include "stbi/stbi_load.hpp"
#include "image_codec.hpp"

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

int main (int argv, char *args[]) {
  std::cout << "Start Codec Test" << std::endl;
  try {
    int outx, outy, outc;
    unsigned int outbytes;
    unsigned char *s, *ic, *is;
    char buff[1024];
    for (unsigned int f = 0; f <= 4; ++f) {
      sprintf (buff, "%s/%02d.png", args[1], f);
      std::cout << "Test: " << buff << " comp ";
      s = stbi::load::load_from_filename (buff, &outx, &outy, &outc, 0);
      outbytes = outx * outy * outc;
      if (!s) throw stbi::load::failure_reason ();
      image_param img_p {(unsigned int)outx, (unsigned int)outy, (unsigned char)outc};
      ic = image_encode(s, img_p, &outbytes);
      is = image_decode(ic, outbytes, &img_p);

      stbi::load::image_free (s);
      
      if (memcmp(s, is, outx * outy * outc)) {
      	std::cout << " Failure ";
      } else {
      	std::cout << " Succes with " << outbytes << " bytes";
      }
      std::cout << std::endl;
      
      image_free(ic);
      image_free(is);
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