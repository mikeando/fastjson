// SARU : Tag fastjson

#include "saru_cxx.h"

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
void init_string_token( Token * tok, char * v, size_t s )
{
  tok->type = fastjson::Token::ValueToken;
  tok->data.value.ptr = v;
  tok->data.value.size = s;
  tok->data.value.type_hint = fastjson::ValueType::StringHint;
}

//Does not copy v and expects v to live for as long as the Token does.
void init_number_token( Token * tok, char * v, size_t s )
{
  tok->type = fastjson::Token::ValueToken;
  tok->data.value.ptr = v;
  tok->data.value.size = s;
  tok->data.value.type_hint = fastjson::ValueType::NumberHint;
}



size_t bytes_required( const Token * tok )
{
  switch( tok->type )
  {
    case Token::ArrayToken:
      if( tok->data.array.ptr==NULL )
      {
        return 2;
      }
      else
      {
        size_t total = 2; //For the opening and closing [] marks
        //Loop through the elements and add them in.
        ArrayEntry * child = tok->data.array.ptr;
        while(child)
        {
          total += bytes_required( &child->tok );
          if( child->next)
            total+=1; //For the ,
          child=child->next;
        }

        return total;
      }
    case Token::DictToken:
      if( tok->data.dict.ptr==NULL )
      {
        return 2;
      }
      else
      {
        size_t total = 2; // For the {}

        //Loop through the elements and add them in.
        DictEntry * child = tok->data.dict.ptr;
        while(child)
        {
          total += bytes_required( &child->key_tok );
          total += 1; // For the :
          total += bytes_required( &child->value_tok );
          if( child->next)
            total +=1 ; // For the ,
          child=child->next;
        }

        return total;
      }
    case Token::ValueToken:
      switch( tok->data.value.type_hint)
      {
        case ValueType::StringHint:
          {
            if (tok->data.value.ptr==NULL)
            {
              return 2;
            }
            else
            {
              //TODO: we need to json escape this string...
              return 2+tok->data.value.size;
            }
          }
          break;
        case ValueType::NumberHint:
          {
            if (tok->data.value.ptr==NULL)
            {
              return 1;
            }
            else
            {
              return tok->data.value.size;
            }
          }
          break;
      }
    case Token::LiteralTrueToken:
      return 4;
    case Token::LiteralFalseToken:
      return 5;
    case Token::LiteralNullToken:
      return 4;
  }
}

//Returns number of bytes output.
size_t serialize_inplace( const Token * tok, char * buffer )
{
  switch( tok->type )
  {
    case Token::ArrayToken:
      if( tok->data.array.ptr==NULL )
      {
        buffer[0]='[';
        buffer[1]=']';
        return 2;
      }
      else
      {
        char * cursor = buffer;
        *cursor='[';
        ++cursor;

        //Loop through the elements and add them in.
        ArrayEntry * child = tok->data.array.ptr;
        while(child)
        {
          cursor += serialize_inplace( &child->tok, cursor );
          if( child->next)
          {
            *cursor=',';
            ++cursor;
          }
            
          child=child->next;
        }
        *cursor=']';
        ++cursor;

        return cursor - buffer;
      }
    case Token::DictToken:
      if( tok->data.dict.ptr==NULL )
      {
        buffer[0]='{';
        buffer[1]='}';
        return 2;
      }
      else
      {
        char * cursor = buffer;
        *cursor='{';
        ++cursor;
        //Loop through the elements and add them in.
        DictEntry * child = tok->data.dict.ptr;
        while(child)
        {
          cursor+=serialize_inplace( &child->key_tok, cursor );
          *cursor=':';
          ++cursor;
          cursor+=serialize_inplace( &child->value_tok, cursor );
          if( child->next)
          {
            *cursor=',';
            ++cursor;
          }
          child=child->next;
        }
        *cursor='}';
        ++cursor;

        return cursor-buffer;
      }
    case Token::ValueToken:
      switch( tok->data.value.type_hint)
      {
        case ValueType::StringHint:
          {
            if (tok->data.value.ptr==NULL)
            {
              buffer[0]='"';
              buffer[1]='"';
              return 2;
            }
            else
            {
              //TODO: we need to json escape this string...
              buffer[0]='"';
              memcpy(buffer+1, tok->data.value.ptr, tok->data.value.size);
              buffer[tok->data.value.size+1]='"';
              return tok->data.value.size+2;
            }
          }
          break;
        case ValueType::NumberHint:
          {
            if (tok->data.value.ptr==NULL)
            {
              buffer[0]='0';
              return 1;
            }
            else
            {
              memcpy(buffer,tok->data.value.ptr, tok->data.value.size);
              return tok->data.value.size;
            }
          }
          break;
      }
    case Token::LiteralTrueToken:
      buffer[0]='t';buffer[1]='r';buffer[2]='u';buffer[3]='e';
      return 4;
    case Token::LiteralFalseToken:
      buffer[0]='f';buffer[1]='a';buffer[2]='l';buffer[3]='s';buffer[4]='e';
      return 5;
    case Token::LiteralNullToken:
      buffer[0]='n';buffer[1]='u';buffer[2]='l';buffer[3]='l';
      return 4;
  }

}

std::string as_string( const Token * tok )
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

class TestFixture
{
  public:
    fastjson::Token tok;
    void count_empty_array()
    {
      tok.type=fastjson::Token::ArrayToken;
      tok.data.array.ptr = NULL;

      saru_assert_equal( 2u, fastjson::bytes_required(&tok) );
    }

    void serialize_inplace_empty_array()
    {
      char buffer[]="xxxxx";
      tok.type=fastjson::Token::ArrayToken;
      tok.data.array.ptr = NULL;
  
      saru_assert( fastjson::serialize_inplace( &tok, buffer+1 ) );
      saru_assert_equal( std::string("x[]xx"), buffer );
    }

    void serialize_empty_array()
    {
      tok.type=fastjson::Token::ArrayToken;
      tok.data.array.ptr = NULL;

      saru_assert_equal( "[]", fastjson::as_string(&tok) );
    }

    void serialize_empty_dict()
    {
      tok.type=fastjson::Token::DictToken;
      tok.data.dict.ptr = NULL;

      saru_assert_equal( "{}", fastjson::as_string(&tok) );
    }

    void serialize_true_literal()
    {
      tok.type=fastjson::Token::LiteralTrueToken;

      saru_assert_equal( "true", fastjson::as_string(&tok) );
    }

    void serialize_false_literal()
    {
      tok.type=fastjson::Token::LiteralFalseToken;

      saru_assert_equal( "false", fastjson::as_string(&tok) );
    }

    void serialize_null_literal()
    {
      tok.type=fastjson::Token::LiteralNullToken;

      saru_assert_equal( "null", fastjson::as_string(&tok) );
    }

    void serialize_empty_string()
    {
      tok.type=fastjson::Token::ValueToken;
      tok.data.value.ptr = NULL;
      tok.data.value.type_hint = fastjson::ValueType::StringHint;

      saru_assert_equal( "\"\"", fastjson::as_string(&tok) );
    }

    void serialize_simple_string()
    {
      fastjson::init_string_token( &tok, "1234", 4 );
      saru_assert_equal( "\"1234\"", fastjson::as_string(&tok) );
    }

    void serialize_array_of_strings()
    {
      //First the strings
      char * s1 = "hello";
      char * s2 = "world";
      //Now the ArrayEntries pointing to these strings
      fastjson::ArrayEntry ae1;
      fastjson::ArrayEntry ae2;

      fastjson::init_string_token( &ae1.tok, s1, strlen(s1) );
      fastjson::init_string_token( &ae2.tok, s2, strlen(s2) );

      //Link them up
      ae1.next = &ae2;
      ae2.next = NULL;

      tok.type=fastjson::Token::ArrayToken;
      tok.data.array.ptr=&ae1;

      saru_assert_equal( "[\"hello\",\"world\"]", fastjson::as_string(&tok) );
    }

    void serialize_nested_arrays()
    {
      fastjson::ArrayEntry ae1;
      fastjson::ArrayEntry ae2;
      fastjson::ArrayEntry ae3;

      //This is ugly!
      ae1.tok.type = fastjson::Token::ArrayToken;
      ae1.tok.data.array.ptr = &ae2;
      ae2.tok.type = fastjson::Token::ArrayToken;
      ae2.tok.data.array.ptr = NULL;
      ae3.tok.type = fastjson::Token::ArrayToken;
      ae3.tok.data.array.ptr = NULL;

      //Link them up
      ae1.next = &ae3;
      ae2.next = NULL;
      ae3.next = NULL;

      tok.type=fastjson::Token::ArrayToken;
      tok.data.array.ptr=&ae1;

      saru_assert_equal( "[[[]],[]]", fastjson::as_string(&tok) );
    }

    void serialize_simple_dict()
    {
      fastjson::DictEntry d1;
      fastjson::DictEntry d2;

      fastjson::init_string_token( &d1.key_tok,   "hello", 5 );
      fastjson::init_string_token( &d1.value_tok, "world", 5 );
      fastjson::init_string_token( &d2.key_tok,   "f", 1 );
      fastjson::init_string_token( &d2.value_tok, "g", 1 );

      d1.next = &d2;
      d2.next = NULL;
      
      tok.type = fastjson::Token::DictToken;
      tok.data.dict.ptr = &d1;

      saru_assert_equal( "{\"hello\":\"world\",\"f\":\"g\"}", fastjson::as_string(&tok) );
    }

    void serialize_number()
    {
      // Yep it takes a string .. the number is just a type-hint that supresses quotes and some otehr stuff.
      fastjson::init_number_token( &tok, "1", 1 );

      saru_assert_equal( "1", fastjson::as_string(&tok) );
    }


};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::count_empty_array, logger);
  SARU_TEST( TestFixture::serialize_inplace_empty_array, logger);

  SARU_TEST( TestFixture::serialize_empty_array, logger);
  SARU_TEST( TestFixture::serialize_empty_dict, logger);
  SARU_TEST( TestFixture::serialize_empty_string, logger);
  SARU_TEST( TestFixture::serialize_simple_string, logger);
  SARU_TEST( TestFixture::serialize_true_literal, logger);
  SARU_TEST( TestFixture::serialize_false_literal, logger);
  SARU_TEST( TestFixture::serialize_null_literal, logger);
  SARU_TEST( TestFixture::serialize_array_of_strings, logger);
  SARU_TEST( TestFixture::serialize_nested_arrays, logger);
  SARU_TEST( TestFixture::serialize_simple_dict, logger);
  SARU_TEST( TestFixture::serialize_number, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}
