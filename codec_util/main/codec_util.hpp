#ifndef _CODEC_UTIL
#define _CODEC_UTIL

#include <vector>
#include <cstdint>
#include <cstddef>
#include <iostream>

struct codec_data {
		struct read {
		    read(const codec_data&);
		
		    read& operator>>(bool&);
		    template<typename T>
		    read& operator>>(T&);
		    bool empty() const;
		private:
		    std::vector<unsigned int> bitBuffer;
		    size_t bitPosition = 0;
		    size_t bitIndex = 0;
		};
    codec_data();
    codec_data(const codec_data&);
    codec_data& operator<<(bool);
    template<typename T>
    codec_data& operator<<(const T&);
    
    void writeBits(uint64_t, size_t);
    read getReadStream() const;
    bool operator==(const codec_data& o) const;
    size_t size() const;
    friend std::ostream &operator<<(std::ostream &, const codec_data &);
private:
    std::vector<unsigned int> bitBuffer;
    size_t bitPosition = 0;
};


#endif // _CODEC_UTIL
