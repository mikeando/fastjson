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

#ifndef FASTSJON2_H
#define FASTSJON2_H

#include <stdint.h>
#include <string>
#include <assert.h>

namespace fastjson
{

struct ArrayEntry;
struct DictEntry;

struct ArrayType
{
  ArrayEntry * ptr;
};

struct DictType
{
  DictEntry * ptr;
};

struct ValueType
{
  char * ptr;
  enum TypeHint { StringHint, NumberHint };
  size_t size;
  TypeHint type_hint;
};

struct Token
{
  enum Type { ArrayToken, DictToken, ValueToken, LiteralTrueToken, LiteralFalseToken, LiteralNullToken };
  Type type;
  union
  {
    ArrayType array;
    DictType  dict;
    ValueType value;
  } data;
};

struct ArrayEntry
{
  Token tok;
  ArrayEntry * next;
};

struct DictEntry
{
  Token key_tok;
  Token value_tok;
  DictEntry * next;
};

//Does not copy v and expects v to live for as long as the Token does.
static inline void init_string_token( Token * tok, char * v, size_t s )
{
  tok->type = fastjson::Token::ValueToken;
  tok->data.value.ptr = v;
  tok->data.value.size = s;
  tok->data.value.type_hint = fastjson::ValueType::StringHint;
}

//Does not copy v and expects v to live for as long as the Token does.
static inline void init_number_token( Token * tok, char * v, size_t s )
{
  tok->type = fastjson::Token::ValueToken;
  tok->data.value.ptr = v;
  tok->data.value.size = s;
  tok->data.value.type_hint = fastjson::ValueType::NumberHint;
}



size_t bytes_required( const Token * tok );

//Returns number of bytes output.
size_t serialize_inplace( const Token * tok, char * buffer );

static inline std::string as_string( const Token * tok )
{
  std::string retval( bytes_required(tok), ' ' );
  serialize_inplace( tok, &retval[0] );
  return retval;
}


static inline uint32_t dict_size( Token * tok)
{
  assert( tok->type == fastjson::Token::DictToken );
  fastjson::DictEntry * child = tok->data.dict.ptr;
  uint32_t count = 0;
  while( child != NULL )
  {
    ++count;
    child = child->next;
  }
  return count;
}

static inline uint32_t array_size( Token * tok)
{
  assert( tok->type == fastjson::Token::ArrayToken );
  fastjson::ArrayEntry * child = tok->data.array.ptr;
  uint32_t count = 0;
  while( child != NULL )
  {
    ++count;
    child = child->next;
  }
  return count;
}

}


#endif
