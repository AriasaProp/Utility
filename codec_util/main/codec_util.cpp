#include <iomanip>
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
codec_data::codec_data(): data(malloc(2)), reserve_byte(2), used_byte(0), unused_bit(0) {}
codec_data::codec_data(const codec_data &other);data(malloc(other.reserve_byte)), reserve_byte(other.reserve_byte), used_byte(other.used_byte), unused_bit(other.unused_bit) {
	memcpy(data, other.data, used_byte);
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
	
friend std::ostream &operator<<(std::ostream &c, const codec_data &d) const {
	c << "codec: ";
	for(unsigned char *rh = (unsigned char *)data, *end = rh + used_byte; rh < end; ++rh)
		c << std::hex << std::setw(2) << std::setfill('0') << *rh;
	return c;
}
