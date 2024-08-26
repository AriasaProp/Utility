#ifndef _CODEC_UTIL
#define _CODEC_UTIL

#include <cstddef>
#include <iostream>

struct codec_data {
	codec_data();
	codec_data(const codec_data&);
	~codec_data();
	
	template<typename T>
	codec_data &operator<<(const T&);
	
	struct reader;
	reader begin_read() const;
	
	template<typename T>
	friend reader &operator>>(reader&, const T&);
	
	friend std::ostream &operator<<(std::ostream&, const codec_data&);
	
private:
	void *data;
	size_t reserve_byte, used_byte, unused_bit;
	void check_resize();
};

#endif // _CODEC_UTIL
