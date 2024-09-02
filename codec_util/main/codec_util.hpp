#ifndef _CODEC_UTIL
#define _CODEC_UTIL

#include <cstddef>
#include <iostream>

struct codec_data {
  codec_data ();
  codec_data (const codec_data &);
  ~codec_data ();

  size_t size_bit () const;
  size_t size_byte () const;

  struct reader {
    reader (void *, size_t, size_t);
    size_t left () const;

//    template <typename T>
    reader &operator>> (unsigned long &);
    reader &operator>> (unsigned int &);
    reader &operator>> (unsigned char &);
    reader &operator>> (char &);
    reader &operator>> (bool &);
//    reader &operator>> (T &);

  private:
    void *data;
    size_t used_byte, used_bit;
    size_t readed_byte, readed_bit;
  };

  reader begin_read () const;

//  template <typename T>
  codec_data &operator<< (unsigned long);
  codec_data &operator<< (unsigned int);
  codec_data &operator<< (unsigned char);
  codec_data &operator<< (char);
  codec_data &operator<< (bool);

  friend bool operator== (const codec_data &, const codec_data &);
  friend std::ostream &operator<< (std::ostream &, const codec_data &);

private:
  void *data;
  size_t reserve_byte, used_byte, used_bit;
  void check_resize (size_t);
};

#endif // _CODEC_UTIL
