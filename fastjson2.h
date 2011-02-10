#ifndef FASTSJON2_H
#define FASTSJON2_H

#include <stdint.h>
#include <string>

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


//We over-allocate space for these using malloc.. this way we can access
//outside more than just one entry in values. (This saves us one allocation per-cache)
struct ArrayCache
{
  uint32_t size;
  ArrayCache * next;
  ArrayEntry values[1];
};

struct Document
{
  Token root;
  ArrayCache * arrays;
};

}


#endif
