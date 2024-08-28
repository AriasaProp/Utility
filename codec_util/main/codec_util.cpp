#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <climits>

#include "codec_util.hpp"

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
    free(data);
    data = new1;
  } else {
  	memset(reinterpret_cast<char*>(data)+old_size_reserve, 0x0, reserve_byte - old_size_reserve);
  }
}

// public
codec_data::codec_data () : data (calloc(1, 4)), reserve_byte (4), used_byte (0), used_bit (0) {}
codec_data::codec_data (const codec_data &other) : data (calloc (1, other.reserve_byte)), reserve_byte (other.reserve_byte), used_byte (other.used_byte), used_bit (other.used_bit) {
  memcpy (data, other.data, used_byte + (used_bit != 0));
}
codec_data::~codec_data () {
  free (data);
}

template <typename T>
codec_data &codec_data::operator<< (T const &out) {
	T *dt = reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(data) + used_byte);
	used_byte += sizeof(T)
	check_resize(used_byte + (used_bit ? 1 : 0));
	if (used_bit) {
		*dt |= out << used_bit;
		*(dt+1) |= out >> (CHAR_BIT - used_bit);
	} else {
		*dt |= out;
	}
  return *this;
}
template <>
codec_data &codec_data::operator<< <unsigned int> (unsigned int const &out) {
	unsigned int *dt = reinterpret_cast<unsigned char*>(reinterpret_cast<unsigned char*>(data) + used_byte);
	used_byte += sizeof(unsigned int)
	check_resize(used_byte + (used_bit ? 1 : 0));
	if (used_bit) {
		*dt |= out << used_bit;
		*(dt+1) |= out >> (CHAR_BIT - used_bit);
	} else {
		*dt |= out;
	}
  return *this;
}
template <>
codec_data &codec_data::operator<< <bool> (bool const &out) {
	unsigned char *dt = reinterpret_cast<unsigned char*>(data) + used_byte;
	{
		size_t want_bit = used_bit + 1;
		check_resize(used_byte + (want_bit / CHAR_BIT) + ((want_bit % CHAR_BIT) ? 1 : 0));
	}
	
	if (out)
		*dt |= 0x1 << used_bit;
	else
		*dt &= ~(0x1 << used_bit);
	
	if (++used_bit == CHAR_BIT) {
		used_bit = 0;
		++used_byte;
	}
  return *this;
}

size_t codec_data::size_bit () const {
  return used_byte * CHAR_BIT + used_bit;
}

codec_data::reader::reader (void *d, size_t ub, size_t unb) : data (d), used_byte (ub), used_bit (unb), readed_byte (0), readed_bit (0) {}
size_t codec_data::reader::left () const {
  return (readed_byte - used_byte) * CHAR_BIT - (readed_bit - used_bit);
}
codec_data::reader codec_data::begin_read () const {
  return codec_data::reader (data, used_byte, used_bit);
}

template <typename T>
codec_data::reader &operator>> (codec_data::reader &r, T const &d) {

  return r;
}
template <>
codec_data::reader &operator>> (codec_data::reader &r, unsigned int const &d) {

  return r;
}

bool operator== (const codec_data &a, const codec_data &b) {
  return (a.used_byte == b.used_byte) && (a.used_bit == b.used_bit) &&
      (memcmp (a.data, b.data, a.used_byte) == 0) &&
      ((reinterpret_cast<unsigned char *> (a.data)[a.used_byte] & ((0x1 << a.used_bit) - 1)) ==
       (reinterpret_cast<unsigned char *> (b.data)[b.used_byte] & ((0x1 << b.used_bit) - 1)));
}
// print hex for small first
std::ostream &operator<< (std::ostream &c, const codec_data &d) {
  c << "codec: ";
  unsigned char *begin_ = (unsigned char *)d.data;
  unsigned char *end_ = begin_ + d.used_byte + (d.used_bit != 0);
  while (end_ > begin_) {
    c << std::hex << std::setw (2) << std::setfill ('0') << *(--end_);
  }
  return c;
}
