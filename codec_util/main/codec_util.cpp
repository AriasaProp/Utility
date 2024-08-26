#include "codec_util.hpp"

constexpr size_t UNIT = sizeof(unsigned int) * CHAR_BIT;

codec_data::codec_data() {}

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

template<typename T>
codec_data& codec_data::operator<<(const T& data) {
    for (size_t i = 0; i < sizeof(T) * CHAR_BIT; ++i) {
        this << bool((data >> i) & 1);
    }
    return *this;
}

void codec_data::writeBits(uint64_t data, size_t bitCount) {
    for (size_t i = 0; i < bitCount; ++i) {
        this << bool((data >> i) & 1);
    }
}

read_stream codec_data::getReadStream() const {
    return read_stream(*this);
}

bool codec_data::operator==(const codec_data& o) const {
    return bitBuffer == o.bitBuffer && bitPosition == o.bitPosition;
}

read_stream::read_stream(const codec_data& _c) : c(_c) {}

template<typename T>
read_stream& read_stream::operator>>(T& target) {
    target = 0;
    for (size_t i = 0; i < sizeof(T) * CHAR_BIT; ++i) {
        if (bitIndex / UNIT >= c.bitBuffer.size()) {
            throw "Index out of range";
        }
        if ((c.bitBuffer[bitIndex / UNIT] & (1 << (bitIndex % UNIT))) != 0) {
            target |= (T(1) << i);
        }
        bitIndex++;
    }
    return *this;
}
