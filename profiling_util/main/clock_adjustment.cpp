#include "clock_adjustment.hpp"
#include <cstring>
#include <chrono>
#include <iostream>

struct profiling::clock_adjustment::private_data {
  char *label;
  std::chrono::time_point<std::chrono::steady_clock> safe_time;
  std::chrono::time_point<std::chrono::steady_clock> safe_time_part;
  std::chrono::time_point<std::chrono::steady_clock> temp_time;
};

profiling::clock_adjustment::clock_adjustment(const char *l) {
  data = new private_data;
  data->label = new char[strlen(l)];
  strcpy(data->label, l);
  data->safe_time_part = data->safe_time = std::chrono::steady_clock::now();
  std::cout << "Clocking for " << data->label << " started now!" << std::endl;
}
unsigned long profiling::clock_adjustment::get_clock(const period p) {
  data->temp_time = std::chrono::steady_clock::now();
  unsigned long res = 0;
  switch (p) {
    case hours:
      res = std::chrono::duration_cast<std::chrono::hours>(data->temp_time - data->safe_time_part).count();
      break;
    case minutes:
      res = std::chrono::duration_cast<std::chrono::minutes>(data->temp_time - data->safe_time_part).count();
      break;
    case seconds:
      res = std::chrono::duration_cast<std::chrono::seconds>(data->temp_time - data->safe_time_part).count();
      break;
    case milliseconds:
      res = std::chrono::duration_cast<std::chrono::milliseconds>(data->temp_time - data->safe_time_part).count();
      break;
    default:
    case microseconds:
      res = std::chrono::duration_cast<std::chrono::microseconds>(data->temp_time - data->safe_time_part).count();
      break;
  }
  data->safe_time_part = std::chrono::steady_clock::now();
  return res;
}
profiling::clock_adjustment::~clock_adjustment() {
  data->temp_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(data->temp_time - data->safe_time);
  
  std::cout << "Clocking for " << data->label << " Ended in ";
  unsigned long dc = duration.count();
  
  if (dc >= 3600000000) { //hours
      std::cout << dc / 3600000000 << " hrs ";
      dc %= 3600000000;
  }
  if (dc >= 60000000) {  // minute
      std::cout << dc / 60000000 << " min ";
      dc %= 60000000;
  }
  if (dc >= 1000000) {  // sec
      std::cout << dc / 1000000 << " sec ";
      dc %= 1000000;
  }
  if (dc >= 1000) { // milli sec
      std::cout << dc / 1000 << " ms ";
      dc %= 1000;
  }
  if (dc) {  // micro sec
      std::cout << dc << " us ";
  }
  std::cout << std::endl;
  delete[] data->label;
  delete data;
}