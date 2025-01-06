#include "BigInteger.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct time_md {
  unsigned long t;
  time_md (){};
  time_md (signed a) : t (a){};
  time_md (unsigned long a) : t (a){};
  time_md &operator= (signed a) {
    t = a;
    return *this;
  }
  time_md &operator+= (signed a) {
    t += a;
    return *this;
  }
  time_md &operator+= (time_md &a) {
    t += a.t;
    return *this;
  }
};

struct timing {
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> safe_time;

public:
  void start () {
    safe_time = std::chrono::high_resolution_clock::now ();
  }
  time_md end () {
    unsigned long tm = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::high_resolution_clock::now () - safe_time).count ();
    return time_md (tm);
  }
};

std::ostream &operator<< (std::ostream &o, const time_md &t) {
  static const char *unit[]{"us", "ns", "ms", "s", "m", "h", "D", "Week", "M"};
  static const int td[]{1000, 1000, 1000, 60, 60, 24, 7, 30 / 7, 12};
  unsigned long l = t.t, i = 0;
  std::vector<unsigned long> arr;
  do {
    if (i > 9)
      break;
    arr.push_back (l % td[i]);
  } while ((l /= td[i++]) != 0);
  std::stringstream ss;
  for (auto x = arr.rbegin (), y = std::min (x + 3, arr.rend ()); x < y; ++x) {
    ss << " " << *x << " " << unit[--i];
  }
  o << ss.str ();

  return o;
}

bool BigInteger_test () {
  std::cout << "BigInteger Test report\n";

  bool result = true;
  time_md report[16];
  timing ct, cta;

  std::cout << "\n|   action    ||          time          ||     error     |\n|-------------||------------------------||---------------|\n| Creation    ||";
  // do calculation
  try {
    ct.start ();
    BigInteger
        a = "17109938276655544333234567880088776588224401929\0",
        b = "-790920875098776655544333234567880088776588224401\0";
    std::cout << std::setw (24) << std::setfill (' ') << ct.end ();
    // compare
    cta.start ();
    ct.start ();
    result = (b == b) && !(b == a);
    std::cout << "||       -       |\n|  •  ==     ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    ct.start ();
    result = (b != a) && !(b != b);
    std::cout << "|\n|  •  !=     ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    ct.start ();
    result = (b <= a) && !(a <= b);
    std::cout << "|\n|  •  <=     ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    ct.start ();
    result = (a >= b) && !(b >= a);
    std::cout << "|\n|  •  >=     ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    ct.start ();
    result = (b < a) && !(a < b);
    std::cout << "|\n|  •  <      ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    ct.start ();
    result = (a > b) && !(b > a);
    std::cout << "|\n|  •  >      ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    std::cout << "|\n| Compare     ||" << std::setw (24) << std::setfill (' ') << cta.end ();

    // math
    a = BigInteger ("888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888\0");
    b = BigInteger ("444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444\0");
    BigInteger A = a, B = b;
    cta.start ();
    ct.start ();
    result = ((b + b) == a);
    result &= ((a + (-b)) == b);
    result &= (((-a) + b) == (-b));
    result &= (((-b) + (-b)) == (-a));
    result &= ((B += b) == a);
    A = a, B = -b;
    result &= ((A += B) == b);
    A = -a;
    result &= ((A += b) == B);
    A = -a, B = -b;
    result &= ((B += B) == A);
    std::cout << "||       -       |\n|  •  Add    ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    ct.start ();
    result = ((a - b) == b);
    result &= (((-a) - (-b)) == (-b));
    result &= (((-b) - b) == (-a));
    result &= ((b - (-b)) == a);
    A = a;
    result &= ((A -= b) == b);
    A = -a, B = -b;
    result &= ((A -= B) == B);
    // B = -b;
    result &= ((B -= b) == -a);
    B = b;
    result &= ((B -= (-b)) == a);
    std::cout << "|\n|  •  Sub    ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result = ((a * a) == b);
    result &= (((-a) * (-a)) == b);
    result &= ((a * (-a)) == (-b));
    result &= (((-a) * a) == (-b));
    result &= ((a *= a) == b);
    a = BigInteger ("-101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= ((a *= a) == b);
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("-10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= ((a *= (-a)) == b);
    a = BigInteger ("-101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("-10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= ((a *= (-a)) == b);
    std::cout << "|\n|  •  Mul    ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result = ((b / a) == a);
    result &= ((b / (-a)) == (-a));
    result &= (((-b) / (-a)) == a);
    result &= (((-b) / a) == (-a));
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= ((b /= a) == a);
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= ((b /= (-a)) == (-a));
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= (((-b) /= (-a)) == a);
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result &= (((-b) /= a) == (-a));
    std::cout << "|\n|  •  Div    ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0"), b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020104\0");
    ct.start ();
    result = ((b % a) == 4);
    result &= ((b %= a) == 4);
    std::cout << "|\n|  •  Mod    ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("1152921504606846976\0"), b = 2;
    result = (b.pow (60) == a);
    std::cout << "|\n|  •  ^      ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\0");
    b = BigInteger ("10203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979900010203040506070809101112111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020099989796959493929190898887868584838281807978777675747372717069686766656463626160595857565554535251504948474645444342414039383736353433323130292827262524232221201918171615141312111009080706050403020100\0");
    result = (b.sqrt () == a);
    std::cout << "|\n|  •  √      ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";
    std::cout << "|\n| Math        ||" << std::setw (24) << std::setfill (' ') << cta.end ();

    cta.start ();

    ct.start ();
    a = BigInteger ("1152921504606846976000000000000000000\0"), b = BigInteger ("1000000000000000000\0");
    result = ((b << (size_t)60) == a);
    a = BigInteger ("1152921504606846976000000000000000000\0"), b = BigInteger ("1000000000000000000\0");
    result &= ((b <<= (size_t)60) == a);
    std::cout << "||       -       |\n|  •  <<      ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("1152921504606846976000000000000000000\0"), b = BigInteger ("1000000000000000000\0");
    result = ((a >> (size_t)60) == b);
    a = BigInteger ("1152921504606846976000000000000000000\0"), b = BigInteger ("1000000000000000000\0");
    result &= ((a >>= (size_t)60) == b);
    std::cout << "|\n|  •  >>      ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("8589934592"), b = BigInteger ("120");
    result = ((a & b) == BigInteger (0));
    A = a;
    result = ((A &= b) == BigInteger (0));
    std::cout << "|\n|  •  & (AND) ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    ct.start ();
    a = BigInteger ("8589934592"), b = BigInteger ("120");
    result = ((a | b) == (a + b));
    a = BigInteger ("8589934592"), b = BigInteger ("120");
    A = a;
    result = ((A |= b) == (a + b));
    std::cout << "|\n|  •  | (OR)  ||" << std::setw (24) << std::setfill (' ') << ct.end ();
    std::cout << result ? "||       -       " : "|| result wrong! ";

    std::cout << "|\n| Binary O    ||" << std::setw (24) << std::setfill (' ') << cta.end () << "||       -       |";

  } catch (const char *msg) {
    std::cout << "Error has occure " << msg << std::endl;
    return false;
  }
  return true;
}