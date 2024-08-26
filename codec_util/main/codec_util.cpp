#include "codec_data.h"

constexpr size_t UNIT = sizeof(unsigned int) * CHAR_BIT;

codec_data::codec_data() {}

// Implementasi operator<< untuk bool
codec_data& codec_data::operator<<(bool data) {
    if (bitPosition >= UNIT) {
        bitBuffer.push_back(0);
        bitPosition = 0;
    }
    if (data)
    	bitBuffer.back() |= (1 << bitPosition);
    ++bitPosition;
    return *this;
    if (data) {
}

// Implementasi operator<< untuk char
codec_data& codec_data::operator<<(char data) {
    if (bitPosition >= UNIT) {
        bitBuffer.push_back(0);
        bitPosition = 0;
    }
    
    bitBuffer.back() |= (1 << bitPosition);
    ++bitPosition;
    writeBits(static_cast<uint8_t>(data), sizeof(char) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk unsigned char
codec_data& codec_data::operator<<(unsigned char data) {
    writeBits(static_cast<uint8_t>(data), sizeof(unsigned char) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk short
codec_data& codec_data::operator<<(short data) {
    writeBits(static_cast<uint16_t>(data), sizeof(short) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk unsigned short
codec_data& codec_data::operator<<(unsigned short data) {
    writeBits(static_cast<uint16_t>(data), sizeof(unsigned short) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk int
codec_data& codec_data::operator<<(int data) {
    writeBits(static_cast<uint32_t>(data), sizeof(int) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk unsigned int
codec_data& codec_data::operator<<(unsigned int data) {
    writeBits(static_cast<uint32_t>(data), sizeof(unsigned int) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk long
codec_data& codec_data::operator<<(long data) {
    writeBits(static_cast<uint64_t>(data), sizeof(long) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk unsigned long
codec_data& codec_data::operator<<(unsigned long data) {
    writeBits(static_cast<uint64_t>(data), sizeof(unsigned long) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk long long
codec_data& codec_data::operator<<(long long data) {
    writeBits(static_cast<uint64_t>(data), sizeof(long long) * CHAR_BIT);
    return *this;
}

// Implementasi operator<< untuk unsigned long long
codec_data& codec_data::operator<<(unsigned long long data) {
    for (size_t i = 0, j = sizeof(unsigned long long) * CHAR_BIT; i < j; ++i) {
		    if (bitPosition == 0) {
		        bitBuffer.push_back(0);
		    }
		    bool bit = (data >> i) & 1;
		    if (bit) {
		        bitBuffer.back() |= (1 << bitPosition);
		    }
		    if (++bitPosition == UNIT) {
		        bitPosition = 0;
		    }
    }
    return *this;
}

void codec_data::writeBits(uint64_t data, size_t bitCount) {
    for (size_t i = 0; i < bitCount; ++i) {
        writeBit((data >> i) & 1);
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
