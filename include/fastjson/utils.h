/*
 * Copyright (c) 2011, Run With Robots
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the fastjson library nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RUN WITH ROBOTS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MICHAEL ANDERSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
