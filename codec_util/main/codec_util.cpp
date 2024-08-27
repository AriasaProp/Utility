#include <iomanip>
#include <cstring>
#include <cstdlib>

#include "codec_util.hpp"

//private
void codec_data::check_resize() {
	
	// need to reserve new size
	size_t old_size_reserve = reserve_byte;
	reserve_byte *= 1.8;
	if (!realloc(data, reserve_byte)) {
		void *new1 = malloc(reserve_byte);
		memcpy(new1, data, old_size_reserve);
		data = new1;
	}
}

// public
codec_data::codec_data(): data(malloc(4)), reserve_byte(4), used_byte(0), used_bit(0) {}
codec_data::codec_data(const codec_data &other): data(malloc(other.reserve_byte)), reserve_byte(other.reserve_byte), used_byte(other.used_byte), used_bit(other.used_bit) {
	memcpy(data, other.data, used_byte + (used_bit != 0));
}
codec_data::~codec_data() {
	free(data);
}

template<typename T>
codec_data &codec_data::operator<<(const T &out) {
	
	return *this;
}
template<>
codec_data &codec_data::operator<<<bool>(const bool &out) {
	
	return *this;
}

codec_data::reader::reader(void *d, size_t ub, size_t unb): data(d), used_byte(ub), used_bit(unb), readed_byte(0), readed_bit(0) {}
size_t codec_data::reader::left() const {
	return (readed_byte - used_byte) * CHAR_BIT - (readed_bit - used_bit);
}
codec_data::reader codec_data::begin_read() const {
	return codec_data::reader(data, used_byte, used_bit);
}

template<typename T>
codec_data::reader &operator>>(codec_data::reader &r, const T &d) {
	
	return r;
}

bool operator==(const codec_data &a, const codec_data &b) {
  return (a.used_byte == b.used_byte) && (a.used_bit == b.used_bit) &&
         (memcmp(a.data, b.data, a.used_byte + (a.used_bit != 0)) == 0) &&
         ((a.data[a.used_byte] & ((0x1 << a.used_bit) - 1)) ==
          (b.data[a.used_byte] & ((0x1 << a.used_bit) - 1)));
}
// print hex for small first
std::ostream &operator<<(std::ostream &c, const codec_data &d) {
	c << "codec: ";
	unsigned char *begin_ = (unsigned char *)d.data;
	unsigned char *end_ = begin_ + d.used_byte + (d.used_bit != 0);
	while (end_ > begin_) {
		c << std::hex << std::setw(2) << std::setfill('0') << *(--end_);
	}
	return c;
}
