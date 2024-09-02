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
  // need to reserve new size
  size_t old_size_reserve = reserve_byte;
  while (reserve_byte < reserve)
    reserve_byte *= 1.8;
  if (!realloc (data, reserve_byte)) {
    void *new1 = calloc (1, reserve_byte);
    memcpy (new1, data, old_size_reserve);
    free (data);
    data = new1;
  } else {
    memset (reinterpret_cast<char *> (data) + old_size_reserve, 0x0, reserve_byte - old_size_reserve);
  }
}

// public
codec_data::codec_data () : data (calloc (1, 4)), reserve_byte (4), used_byte (0), used_bit (0) {}
codec_data::codec_data (const codec_data &other) : data (calloc (1, other.reserve_byte)), reserve_byte (other.reserve_byte), used_byte (other.used_byte), used_bit (other.used_bit) {
  memcpy (data, other.data, used_byte + (used_bit != 0));
}
codec_data::~codec_data () {
  free (data);
}

size_t codec_data::size_bit () const {
  return used_byte * CHAR_BIT + used_bit;
}
size_t codec_data::size_byte () const {
  return used_byte + (used_bit ? 1 : 0);
}

codec_data::reader::reader (void *d, size_t ub, size_t unb) : data (d), used_byte (ub), used_bit (unb), readed_byte (0), readed_bit (0) {}
size_t codec_data::reader::left () const {
  return (used_byte - readed_byte) * CHAR_BIT + used_bit - readed_bit;
}
codec_data::reader codec_data::begin_read () const {
  return codec_data::reader (data, used_byte, used_bit);
}

// reading function
/*
template <typename T>
codec_data::reader &codec_data::reader::operator>> (T &d) {
  if (left () >= sizeof (T) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (data) + readed_byte;
    d = 0;
    memcpy (&d, dt, sizeof (T));
    if (readed_bit) {
      d >>= readed_bit;
      d |= (*(dt + sizeof (T)) >> (CHAR_BIT - readed_bit)) << ((sizeof (T) - 1) << 3);
    }
    readed_byte += sizeof (T);
  }
  return *this;
}
*/
codec_data::reader &codec_data::reader::operator>> (unsigned long &d) {
  if (left () >= sizeof (unsigned long) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (data) + readed_byte;
    d = 0;
    memcpy (&d, dt, sizeof (unsigned long));
    if (readed_bit) {
      d >>= readed_bit;
      d |= (*(dt + sizeof (unsigned long)) >> (CHAR_BIT - readed_bit)) << ((sizeof (unsigned long) - 1) << 3);
    }
    readed_byte += sizeof (unsigned long);
  }
  return *this;
}
codec_data::reader &codec_data::reader::operator>> (unsigned int &d) {
  if (left () >= sizeof (unsigned int) * CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (data) + readed_byte;
    d = 0;
    memcpy (&d, dt, sizeof (unsigned int));
    if (readed_bit) {
      d >>= readed_bit;
      d |= (*(dt + sizeof (unsigned int)) >> (CHAR_BIT - readed_bit)) << ((sizeof (unsigned int) - 1) << 3);
    }
    readed_byte += sizeof (unsigned int);
  }
  return *this;
}
codec_data::reader &codec_data::reader::operator>> (unsigned char &d) {
  if (left () >= CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (data) + readed_byte;
    d = 0;
    memcpy (&d, dt, 1);
    if (readed_bit) {
      d >>= readed_bit;
      d |= (*(dt + 1) >> (CHAR_BIT - readed_bit));
    }
    ++readed_byte;
  }
  return *this;
}
codec_data::reader &codec_data::reader::operator>> (char &d) {
  if (left () >= CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (data) + readed_byte;
    d = *dt;
    if (readed_bit) {
      d >>= readed_bit;
      d |= (*(dt + 1) >> (CHAR_BIT - readed_bit));
    }
    ++readed_byte;
  }
  return *this;
}
codec_data::reader &codec_data::reader::operator>> (bool &d) {
  if (left ()) {
    char *dt = reinterpret_cast<char *> (data);
    d = (dt[readed_byte] >> readed_bit++) & 0x1;
    if (readed_bit == CHAR_BIT) {
      readed_bit = 0;
      ++readed_byte;
    }
  }
  return *this;
}

// writing function
/*
template <typename T>
codec_data &codec_data::operator<< (T in) {
  char *dt = reinterpret_cast<char *> (data) + used_byte;
  used_byte += sizeof (T);
  check_resize (used_byte + (used_bit ? 1 : 0));
  if (used_bit) {
    T shifted = in << used_bit;
    memcpy (dt, &shifted, sizeof (T));
    dt += sizeof (T);
    *dt = (in >> (CHAR_BIT - used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (T));
  }
  return *this;
}
*/
codec_data &codec_data::operator<< (unsigned long in) {
  char *dt = reinterpret_cast<char *> (data) + used_byte;
  used_byte += sizeof (unsigned long);
  check_resize (used_byte + (used_bit ? 1 : 0));
  if (used_bit) {
    unsigned long shifted = (in << used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (unsigned long));
    dt += sizeof (unsigned long);
    *dt = (in >> (CHAR_BIT - used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (unsigned long));
  }
  return *this;
}
codec_data &codec_data::operator<< (unsigned int in) {
  char *dt = reinterpret_cast<char *> (data) + used_byte;
  used_byte += sizeof (unsigned int);
  check_resize (used_byte + (used_bit ? 1 : 0));
  if (used_bit) {
    unsigned int shifted = (in << used_bit) | *dt;
    memcpy (dt, &shifted, sizeof (unsigned int));
    dt += sizeof (unsigned int);
    *dt = (in >> (CHAR_BIT - used_bit)) & 0xff;
  } else {
    memcpy (dt, &in, sizeof (unsigned int));
  }
  return *this;
}
codec_data &codec_data::operator<< (unsigned char in) {
  char *dt = reinterpret_cast<char *> (data) + used_byte;
  ++used_byte;
  check_resize (used_byte + (used_bit ? 1 : 0));
  if (used_bit) {
    dt |= (in << used_bit) & 0xff;
    *(dt + 1) = (in >> (CHAR_BIT - used_bit)) & 0xff;
  } else {
    dt = in;
  }
  return *this;
}
codec_data &codec_data::operator<< (char in) {
  char *dt = reinterpret_cast<char *> (data) + used_byte;
  ++used_byte;
  check_resize (used_byte + (used_bit ? 1 : 0));
  if (used_bit) {
    dt |= (in << used_bit) & 0xff;
    *(dt + 1) = (in >> (CHAR_BIT - used_bit)) & 0xff;
  } else {
    dt = in;
  }
  return *this;
}
codec_data &codec_data::operator<< (bool in) {
  check_resize (used_byte + 1);
  char *dt = reinterpret_cast<char *> (data) + used_byte;
  if (in)
    *dt |= 0x1 << used_bit;
  else
    *dt &= ~(0x1 << used_bit);
  if (++used_bit == CHAR_BIT) {
    used_bit = 0;
    ++used_byte;
  }
  return *this;
}

// compare
bool operator== (const codec_data &a, const codec_data &b) {
  return (a.used_byte == b.used_byte) && (a.used_bit == b.used_bit) &&
      (memcmp (a.data, b.data, a.used_byte) == 0) &&
      ((reinterpret_cast<unsigned char *> (a.data)[a.used_byte] & ((0x1 << a.used_bit) - 1)) ==
       (reinterpret_cast<unsigned char *> (b.data)[b.used_byte] & ((0x1 << b.used_bit) - 1)));
}

// printing
// print hex for small first
static inline void tohexsafe (std::ostream &c, unsigned char a) {
  // big
  unsigned char a1 = a >> 4;
  if (a1 >= 10)
    c << ('a' + (a1 - 10));
  else
    c << ('0' + a1);
  // small
  unsigned char a2 = a & 0xf;
  if (a2 >= 10)
    c << ('a' + (a2 - 10));
  else
    c << ('0' + a2);
}
std::ostream &operator<< (std::ostream &c, const codec_data &d) {
  c << "codec: ";
  unsigned char *begin_ = (unsigned char *)d.data;
  unsigned char *end_ = begin_ + d.used_byte + (d.used_bit != 0);
  if (d.used_bit)
    tohexsafe (c, *(end_--) & ((0x1 << d.used_bit) - 1));
  while (end_ > begin_) {
    tohexsafe (c, *--end_);
  }
  return c;
}
