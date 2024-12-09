#include "image_codec.hpp"
#include "stbi/stbi_load.hpp"

#include <chrono>
#include <cstdint>
#include <cstring>
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
    unsigned int outbytes;
    unsigned char *s, *ic, *is;
    int stbix, stbiy, stbic;
    char buff[1024];
    image_param img_p;
    for (unsigned int f = 0; f <= 5; ++f) {
      sprintf (buff, "%s/%02d.png", args[1], f);
      std::cout << "Test: " << buff;
      s = stbi::load::load_from_filename (buff, &stbix, &stbiy, &stbic, 0);
      outbytes = stbix * stbiy * stbic;
      if (!s) throw stbi::load::failure_reason ();
      img_p.width = stbix;
      img_p.height = stbiy;
      img_p.channel = stbic;
      ic = image_encode (s, img_p, &outbytes);
      is = image_decode (ic, outbytes, &img_p);

      if (
          (stbix != img_p.width) ||
          (stbiy != img_p.height) ||
          (stbic != img_p.channel) ||
          memcmp (s, is, img_p.width * img_p.height * img_p.channel)) {
        std::cout << " Failure " << std::endl;
        std::cout << "A info x " << stbix << " y " << stbiy << " ch " << stbic << std::endl;
        std::cout << "B info x " << img_p.width << " y " << img_p.height << " ch " << (int)img_p.channel << std::endl;
      } else {
      	long double ratio = outbytes;
      	ratio /= (stbix * stbiy * stbic);
      	ratio *= 100;
        std::cout << " √ " << ratio << " \% origin";
      }
      std::cout << std::endl;

      stbi::load::image_free (s);
      image_free (ic);
      image_free (is);
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