#ifndef _CODEC_UTIL
#define _CODEC_UTIL

#include <vector>
#include <cstdint>

class read_stream;

class codec_data {
private:
    std::vector<unsigned int> bitBuffer;
    size_t bitPosition = 0;

public:
    codec_data();
    codec_data& operator<<(bool data);
    template<typename T>
    codec_data& operator<<(const T& data);
    
    void writeBits(uint64_t data, size_t bitCount);
    read_stream getReadStream() const;
    bool operator==(const codec_data& o) const;
};

class read_stream {
private:
    const codec_data& c;
    size_t bitIndex = 0;

public:
    read_stream(const codec_data& _c);

    template<typename T>
    read_stream& operator>>(T& target);
};

#endif // _CODEC_UTIL
