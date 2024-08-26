#include "codec_util.hpp"

#include <iomanip>
#include <climits>

constexpr size_t UNIT = sizeof(unsigned int) * CHAR_BIT;

codec_data::codec_data() {}
codec_data::codec_data(const codec_data &o): bitBuffer(o.bitBuffer), bitPosition(o.bitPosition) {}

codec_data& codec_data::operator<<(bool data) {
	if (bitPosition >= UNIT) {
		bitBuffer.push_back(0);
		bitPosition = 0;
	}
	if (data)
	bitBuffer.back() |= (1 << bitPosition);
	++bitPosition;
	return *this;
}

codec_data& codec_data::operator<<(const unsigned int &data) {
	if (bitPosition == 0 || bitPosition >= UNIT) {
		bitBuffer.push_back(data);
		bitPosition = 0;
	} else {
		bitBuffer.back() |= (data << bitPosition);
		bitBuffer.push_back(data >> (UNIT - bitPosition));
	}
	return *this;
}

template < typename T >
codec_data& codec_data::operator<<(const T& data) {
	for (size_t i = 0; i < sizeof(T) * CHAR_BIT; ++i) {
		*this << bool((data >> i) & 1);
	}
	return *this;
}

void codec_data::writeBits(uint64_t data, size_t bitCount) {
	for (size_t i = 0; i < bitCount; ++i) {
		*this << bool((data >> i) & 1);
	}
}

codec_data::read codec_data::getReadStream() const {
	return codec_data::read(*this);
}

bool codec_data::operator == (const codec_data& o) const {
	return bitBuffer == o.bitBuffer && bitPosition == o.bitPosition;
}
size_t codec_data::size() const {
	return (bitBuffer.size() - 1) * UNIT + bitPosition;
}
std::ostream &operator<<(std::ostream &o, const codec_data &c) {
	for (unsigned int b : c.bitBuffer)
		o << std::hex << std::setw(8) << std::setfill('0') << b;
	return o;
}
codec_data::read::read(const codec_data &c): bitBuffer(c.bitBuffer), bitPosition(c.bitPosition) {}

template < typename T >
codec_data::read& codec_data::read::operator>>(T& target) {
	target = 0;
	for (size_t i = 0; i < sizeof(T) * CHAR_BIT; ++i) {
		if (bitIndex / UNIT >= bitBuffer.size()) {
			throw "Index out of range";
		}
		if ((bitBuffer[bitIndex / UNIT] & (1 << (bitIndex % UNIT))) != 0) {
			target |= (T(1) << i);
		}
		bitIndex++;
	}
	return *this;
}

bool codec_data::read::empty() const {
	return bitIndex < (bitBuffer.size() - 1) * UNIT + bitPosition;
}