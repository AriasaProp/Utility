#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>

#include "codec_util.hpp"

#define MIN(A, B) ((A < B) ? A : B)

// private
// make reserve byte ready for future
void codec_data::check_resize (size_t reserve) {
  if (reserve < reserve_byte) return;
  // need to reserve new size
  size_t old_size_reserve = reserve_byte;
  while (reserve_byte < reserve)
    reserve_byte *= 1.6;
  if (data = realloc (data, reserve_byte)) {
    memset (reinterpret_cast<char *> (data) + old_size_reserve, 0x0, reserve_byte - old_size_reserve);
  } else {
    void *new1 = calloc (1, reserve_byte);
    memcpy (new1, data, old_size_reserve);
    free (data);
    data = new1;
  }
}

// public
codec_data::codec_data () : data (calloc (1, 8)), reserve_byte (8), used_byte (0), used_bit (0) {}
codec_data::codec_data (size_t _reserve) : data (calloc (1, _reserve)), reserve_byte (_reserve), used_byte (0), used_bit (0) {}
codec_data::codec_data (const codec_data &other) : data (calloc (1, other.reserve_byte)), reserve_byte (other.reserve_byte), used_byte (other.used_byte), used_bit (other.used_bit) {
  memcpy (data, other.data, used_byte + (used_bit > 0));
}
codec_data::~codec_data () {
  free (data);
}

size_t codec_data::size_bit () const {
  return used_byte * CHAR_BIT + used_bit;
}
size_t codec_data::size_byte () const {
  return used_byte + (used_bit > 0);
}

codec_data::reader::reader (void *d, size_t ub, size_t unb) : data (d), used_byte (ub), used_bit (unb), readed_byte (0), readed_bit (0) {}
size_t codec_data::reader::left () const {
  return (used_byte - readed_byte) * CHAR_BIT + used_bit - readed_bit;
}
codec_data::reader codec_data::begin_read () const {
  return codec_data::reader (data, used_byte, used_bit);
}

// reading function
codec_data::reader &operator>> (codec_data::reader &o, unsigned long &d) {
  if (o.left () >= sizeof (unsigned long) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    memcpy (&d, dt, sizeof (unsigned long));
    if (o.readed_bit) {
      d >>= o.readed_bit;
      dt += sizeof (unsigned long);
      d |= (((unsigned long)*dt) << (CHAR_BIT - o.readed_bit)) << ((sizeof (unsigned long) - 1) << 3);
    }
    o.readed_byte += sizeof (unsigned long);
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, unsigned int &d) {
  if (o.left () >= sizeof (unsigned int) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    memcpy (&d, dt, sizeof (unsigned int));
    if (o.readed_bit) {
      d >>= o.readed_bit;
      dt += sizeof (unsigned int);
      d |= (((unsigned int)*dt) << (CHAR_BIT - o.readed_bit)) << ((sizeof (unsigned int) - 1) << 3);
    }
    o.readed_byte += sizeof (unsigned int);
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, unsigned short &d) {
  if (o.left () >= sizeof (unsigned short) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    memcpy (&d, dt, sizeof (unsigned short));
    if (o.readed_bit) {
      d >>= o.readed_bit;
      dt += sizeof (unsigned short);
      d |= (((unsigned short)*dt) << (CHAR_BIT - o.readed_bit)) << ((sizeof (unsigned short) - 1) << 3);
    }
    o.readed_byte += sizeof (unsigned short);
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, unsigned char &d) {
  if (o.left () >= CHAR_BIT) {
    unsigned char *dt = reinterpret_cast<unsigned char *> (o.data) + o.readed_byte;
    d = *dt;
    if (o.readed_bit) {
      d >>= o.readed_bit;
      d |= (*(dt + 1) << (CHAR_BIT - o.readed_bit));
    }
    ++o.readed_byte;
    std::cout << "Read: " << int ((*dt >> o.readed_bit) | (o.readed_bit ? *(dt + 1) << (CHAR_BIT - o.readed_bit) : 0))
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, long &d) {
  if (o.left () >= sizeof (long) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    memcpy (&d, dt, sizeof (long));
    if (o.readed_bit) {
      d >>= o.readed_bit;
      dt += sizeof (long);
      d |= (((long)*dt) << (CHAR_BIT - o.readed_bit)) << ((sizeof (long) - 1) << 3);
    }
    o.readed_byte += sizeof (long);
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, int &d) {
  if (o.left () >= sizeof (int) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    memcpy (&d, dt, sizeof (int));
    if (o.readed_bit) {
      d >>= o.readed_bit;
      dt += sizeof (int);
      d |= (((int)*dt) << (CHAR_BIT - o.readed_bit)) << ((sizeof (int) - 1) << 3);
    }
    o.readed_byte += sizeof (int);
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, short &d) {
  if (o.left () >= sizeof (short) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    memcpy (&d, dt, sizeof (short));
    if (o.readed_bit) {
      d >>= o.readed_bit;
      dt += sizeof (short);
      d |= (((short)*dt) << (CHAR_BIT - o.readed_bit)) << ((sizeof (short) - 1) << 3);
    }
    o.readed_byte += sizeof (short);
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, char &d) {
  if (o.left () >= CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (o.data) + o.readed_byte;
    d = *dt;
    if (o.readed_bit) {
      d >>= o.readed_bit;
      ++dt;
      d |= (*dt << (CHAR_BIT - o.readed_bit));
    }
    ++o.readed_byte;
  }
  return o;
}
codec_data::reader &operator>> (codec_data::reader &o, bool &d) {
  if (o.left ()) {
    char *dt = reinterpret_cast<char *> (o.data);
    d = (dt[o.readed_byte] >> o.readed_bit++) & 0x1;
    if (o.readed_bit == CHAR_BIT) {
      o.readed_bit = 0;
      ++o.readed_byte;
    }
  }
  return o;
}

// writing function
codec_data &operator<< (codec_data &o, unsigned long in) {
  o.check_resize (o.used_byte + sizeof (unsigned long) + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    unsigned long shifted = (in << o.used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (unsigned long));
    dt += sizeof (unsigned long);
    *dt = (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (unsigned long));
  }
  o.used_byte += sizeof (unsigned long);
  return o;
}
codec_data &operator<< (codec_data &o, unsigned int in) {
  o.check_resize (o.used_byte + sizeof (unsigned int) + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    unsigned int shifted = (in << o.used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (unsigned int));
    dt += sizeof (unsigned int);
    *dt = char (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (unsigned int));
  }
  o.used_byte += sizeof (unsigned int);
  return o;
}
codec_data &operator<< (codec_data &o, unsigned short in) {
  o.check_resize (o.used_byte + sizeof (unsigned short) + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    unsigned short shifted = (in << o.used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (unsigned short));
    dt += sizeof (unsigned short);
    *dt = (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (unsigned short));
  }
  o.used_byte += sizeof (unsigned short);
  return o;
}
codec_data &operator<< (codec_data &o, unsigned char in) {
  o.check_resize (o.used_byte + 1 + (o.used_bit > 0));
  unsigned char *dt = reinterpret_cast<unsigned char *> (o.data) + o.used_byte;
  *dt |= in << o.used_bit;
  if (o.used_bit)
    *(dt + 1) = in << (CHAR_BIT - o.used_bit);
  ++o.used_byte;
  std::cout << "Write: " << int ((*dt >> o.used_bit) | (o.used_bit ? *(dt + 1) << (CHAR_BIT - o.used_bit) : 0)) return o;
}
codec_data &operator<< (codec_data &o, long in) {
  o.check_resize (o.used_byte + sizeof (long) + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    long shifted = (in << o.used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (long));
    dt += sizeof (long);
    *dt = (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (long));
  }
  o.used_byte += sizeof (long);
  return o;
}
codec_data &operator<< (codec_data &o, int in) {
  o.check_resize (o.used_byte + sizeof (int) + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    int shifted = (in << o.used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (int));
    dt += sizeof (int);
    *dt = char (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (int));
  }
  o.used_byte += sizeof (int);
  return o;
}
codec_data &operator<< (codec_data &o, short in) {
  o.check_resize (o.used_byte + sizeof (short) + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    short shifted = (in << o.used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (short));
    dt += sizeof (short);
    *dt = (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (short));
  }
  o.used_byte += sizeof (short);
  return o;
}
codec_data &operator<< (codec_data &o, char in) {
  o.check_resize (o.used_byte + 1 + (o.used_bit > 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (o.used_bit) {
    *dt |= (in << o.used_bit) & 0xff;
    *(dt + 1) = (in << (CHAR_BIT - o.used_bit)) & 0xff;
  } else {
    *dt = in;
  }
  ++o.used_byte;
  return o;
}
codec_data &operator<< (codec_data &o, bool in) {
  o.check_resize (o.used_byte + 1);
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (in)
    *dt |= 0x1 << o.used_bit;
  else
    *dt &= ~(0x1 << o.used_bit);
  if (++o.used_bit == CHAR_BIT) {
    o.used_bit = 0;
    ++o.used_byte;
  }
  return o;
}

// compare
bool operator== (const codec_data &a, const codec_data &b) {
  return (a.used_byte == b.used_byte) && (a.used_bit == b.used_bit) &&
      (memcmp (a.data, b.data, a.used_byte) == 0) &&
      ((reinterpret_cast<unsigned char *> (a.data)[a.used_byte] & ((0x1 << a.used_bit) - 1)) ==
       (reinterpret_cast<unsigned char *> (b.data)[b.used_byte] & ((0x1 << b.used_bit) - 1)));
}
// printing
const char print_A = 'a' - 10, print_0 = '0';
std::ostream &operator<< (std::ostream &c, const codec_data &d) {
  c << "codec: ";
  char *cur_ = reinterpret_cast<char *> (d.data);
  char *end_ = cur_ + d.used_byte;
  char a, a1, a2;
  while (cur_ < end_) {
    a = *cur_++;
    a1 = a & 0xf;
    a2 = (a >> 4) & 0xf;
    c << char (a1 > 9 ? print_A + a1 : print_0 + a1) << char (a2 > 9 ? print_A + a2 : print_0 + a2);
  }
  if (d.used_bit) {
    a = *cur_ & ((0x1 << d.used_bit) - 1);
    a1 = a & 0xf;
    if (a1) {
      c << char (a1 > 9 ? print_A + a1 : print_0 + a1);
      a2 = (a >> 4) & 0xf;
      if (a2)
        c << char (a2 > 9 ? print_A + a2 : print_0 + a2);
    }
  }
  return c;
}
