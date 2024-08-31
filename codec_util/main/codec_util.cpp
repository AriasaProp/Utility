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
template <typename T>
codec_data::reader &operator>> (codec_data::reader &r, T &d) {
  if (r.left () >= sizeof (T) * CHAR_BIT) {
    char *dt = reinterpret_cast<T *> (reinterpret_cast<char *> (r.data) + r.readed_byte);
    d = *dt >> r.readed_bit;
    if (r.readed_bit)
      d |= *(dt + 1) << (CHAR_BIT - r.readed_bit);
    r.readed_byte += sizeof (T);
  }
  return r;
}
template <>
codec_data::reader &operator>><unsigned long> (codec_data::reader &r, unsigned long &d) {
  if (r.left () >= sizeof (unsigned long) * CHAR_BIT) {
    unsigned long *dt = reinterpret_cast<unsigned long *> (reinterpret_cast<char *> (r.data) + r.readed_byte);
    d = *dt >> r.readed_bit;
    if (r.readed_bit)
      d |= *(dt + 1) << (CHAR_BIT - r.readed_bit);
    r.readed_byte += sizeof (unsigned long);
  }
  return r;
}
template <>
codec_data::reader &operator>><unsigned int> (codec_data::reader &r, unsigned int &d) {
  if (r.left () >= sizeof (unsigned int) * CHAR_BIT) {
    unsigned int *dt = reinterpret_cast<unsigned int *> (reinterpret_cast<char *> (r.data) + r.readed_byte);
    d = *dt >> r.readed_bit;
    if (r.readed_bit)
      d |= *(dt + 1) << (CHAR_BIT - r.readed_bit);
    r.readed_byte += sizeof (unsigned int);
  }
  return r;
}
template <>
codec_data::reader &operator>><char> (codec_data::reader &r, char &d) {
  if (r.left () >= CHAR_BIT) {
    char *dt = reinterpret_cast<char *> (r.data);
    d = dt[r.readed_byte++] >> r.readed_bit;
    if (r.readed_bit)
      d |= dt[r.readed_byte] << (CHAR_BIT - r.readed_bit);
  }
  return r;
}
template <>
codec_data::reader &operator>><bool> (codec_data::reader &r, bool &d) {
  if (r.left ()) {
    char *dt = reinterpret_cast<char *> (r.data);
    d = (dt[r.readed_byte] >> r.readed_bit++) & 0x1;
    if (r.readed_bit == CHAR_BIT) {
      r.readed_bit = 0;
      ++r.readed_byte;
    }
  }
  return r;
}

// writing function
template <typename T>
codec_data &operator<< (codec_data &o, T out) {
  o.check_resize (o.used_byte + sizeof (T) + (o.used_bit ? 1 : 0));
  T *dt = reinterpret_cast<T *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += sizeof (T);
  return o;
}
template <>
codec_data &operator<< <unsigned long> (codec_data &o, unsigned long out) {
  o.check_resize (o.used_byte + sizeof (unsigned long) + (o.used_bit ? 1 : 0));
  unsigned long *dt = reinterpret_cast<unsigned long *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += sizeof (unsigned long);
  return o;
}
template <>
codec_data &operator<< <long> (codec_data &o, long out) {
  o.check_resize (o.used_byte + sizeof (long) + (o.used_bit ? 1 : 0));
  long *dt = reinterpret_cast<long *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += sizeof (long);
  return o;
}
template <>
codec_data &operator<< <unsigned int> (codec_data &o, unsigned int out) {
  o.check_resize (o.used_byte + sizeof (unsigned int) + (o.used_bit ? 1 : 0));
  unsigned int *dt = reinterpret_cast<unsigned int *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += sizeof (unsigned int);
  return o;
}
template <>
codec_data &operator<< <int> (codec_data &o, int out) {
  o.check_resize (o.used_byte + sizeof (int) + (o.used_bit ? 1 : 0));
  int *dt = reinterpret_cast<int *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += sizeof (int);
  return o;
}
template <>
codec_data &operator<< <unsigned short> (codec_data &o, unsigned short out) {
  o.check_resize (o.used_byte + 2 + (o.used_bit ? 1 : 0));
  unsigned short *dt = reinterpret_cast<unsigned short *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += 2;
  return o;
}
template <>
codec_data &operator<< <short> (codec_data &o, short out) {
  o.check_resize (o.used_byte + 2 + (o.used_bit ? 1 : 0));
  short *dt = reinterpret_cast<short *> (reinterpret_cast<char *> (o.data) + o.used_byte);
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  o.used_byte += 2;
  return o;
}
template <>
codec_data &operator<< <unsigned char> (codec_data &o, unsigned char out) {
  o.check_resize (o.used_byte + 1 + (o.used_bit ? 1 : 0));
  unsigned char *dt = reinterpret_cast<unsigned char *> (o.data) + o.used_byte;
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  ++o.used_byte;
  return o;
}
template <>
codec_data &operator<< <char> (codec_data &o, char out) {
  o.check_resize (o.used_byte + 1 + (o.used_bit ? 1 : 0));
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  *dt |= out << o.used_bit;
  if (o.used_bit)
    dt[1] |= out >> (CHAR_BIT - o.used_bit);
  ++o.used_byte;
  return o;
}
template <>
codec_data &operator<< <bool> (codec_data &o, bool out) {
  o.check_resize (o.used_byte + 1);
  char *dt = reinterpret_cast<char *> (o.data) + o.used_byte;
  if (out)
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
