#include "clock_adjustment.hpp"
#include <cstring>
#include <chrono>
#include <iostream>
#include <map>

static std::map<void*, size_t> memoryMap;

void* operator new(size_t size) {
    void* ptr = malloc(size);
    memoryMap[ptr] = size;
    return ptr;
}

void operator delete(void* ptr) noexcept {
    free(ptr);
    memoryMap.erase(ptr);
}

struct clock_adjustment::private_data {
  char *label;
  std::chrono::time_point<std::chrono::steady_clock> safe_time;
  std::chrono::time_point<std::chrono::steady_clock> safe_time_part;
  std::chrono::time_point<std::chrono::steady_clock> temp_time;
};

clock_adjustment::clock_adjustment(const char *l) {
  data = new private_data;
  data->label = new char[strlen(l)];
  strcpy(data->label, l);
  data->safe_time_part = data->safe_time = std::chrono::steady_clock::now();
  std::cout << "Clocking for " << data->label << " started now!" << std::endl;
}
unsigned long clock_adjustment::get_clock(const period p) {
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
clock_adjustment::~clock_adjustment() {
  data->temp_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(data->temp_time - data->safe_time);
    
  std::cout << "Clocking for " << data->label << " Ended in ";
  if (duration.count() > 3600000000) {  // Jika lebih dari 1 jam
      std::cout << duration.count() / 3600000000 << " hours" << std::endl;
  } else if (duration.count() > 60000000) {  // Jika lebih dari 1 menit
      std::cout << duration.count() / 60000000 << " minutes" << std::endl;
  } else if (duration.count() > 1000000) {  // Jika lebih dari 1 detik
      std::cout << duration.count() / 1000000 << " seconds" << std::endl;
  } else if (duration.count() > 1000) {  // Jika lebih dari 1 milidetik
      std::cout << duration.count() / 1000 << " milliseconds" << std::endl;
  } else {  // Mikrodetik
      std::cout << duration.count() << " microseconds" << std::endl;
  }
  delete[] data->label;
  delete data;
}