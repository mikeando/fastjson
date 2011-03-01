#ifndef FJ_UTIL_TRY_CONVERT_H
#define FJ_UTIL_TRY_CONVERT_H

#include <iostream>

namespace fastjson { namespace utils {

struct InPlaceStreamBuf : public std::streambuf
{
  InPlaceStreamBuf( char * buf, std::size_t n )
  {
    setg(buf,buf,buf+n);
  }
};


//Alternative to boost::lexical_cast
template<typename T>
bool try_convert( char * buffer, uint32_t size, T * out)
{
  //We read int the tmp so that if it fails we dont modify out.
  T tmp;
  InPlaceStreamBuf buf(buffer,size);
  std::istream istr(&buf);
  bool ok = (istr >> tmp) && istr.eof();
  if(ok)
  {
    *out=tmp;
  }
  return ok;
}

} }

#endif
