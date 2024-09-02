#ifndef _CODEC_UTIL
#define _CODEC_UTIL

#include <cstddef>
#include <iostream>

struct codec_data {
  codec_data ();
  codec_data (size_t);
  codec_data (const codec_data &);
  ~codec_data ();

  size_t size_bit () const;
  size_t size_byte () const;

  struct reader {
    reader (void *, size_t, size_t);
    size_t left () const;

    friend reader &operator>> (reader &, unsigned long &);
    friend reader &operator>> (reader &, unsigned int &);
    friend reader &operator>> (reader &, unsigned short &);
    friend reader &operator>> (reader &, unsigned char &);
    friend reader &operator>> (reader &, char &);
    friend reader &operator>> (reader &, bool &);

  private:
    void *data;
    size_t used_byte, used_bit;
    size_t readed_byte, readed_bit;
  };

  reader begin_read () const;

  friend codec_data &operator<< (codec_data &, unsigned long);
  friend codec_data &operator<< (codec_data &, unsigned int);
  friend codec_data &operator<< (codec_data &, unsigned short);
  friend codec_data &operator<< (codec_data &, unsigned char);
  friend codec_data &operator<< (codec_data &, char);
  friend codec_data &operator<< (codec_data &, bool);

  friend bool operator== (const codec_data &, const codec_data &);
  friend std::ostream &operator<< (std::ostream &, const codec_data &);

private:
  void *data;
  size_t reserve_byte, used_byte, used_bit;
  void check_resize (size_t);
};

#endif // _CODEC_UTIL
