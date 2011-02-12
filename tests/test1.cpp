// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"
#include "fastjson2.h"


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

    void serialize_escaped_unicode_1byte()
    {
      unsigned char buffer[] = { 'a', 0x7F, 'a' };
      char result[] = "xxxxxxxxxxxx";

      tok.type = fastjson::Token::ValueToken;
      tok.data.value.ptr = reinterpret_cast<char*>(buffer);
      tok.data.value.size = 3;
      tok.data.value.type_hint = fastjson::ValueType::StringHint;
      saru_assert(fastjson::serialize_inplace( &tok, result+1 ) );
      saru_assert_equal( std::string("x\"a\\u007Fa\"x"), std::string(result) );
    }

    void serialize_escaped_unicode_2byte()
    {
      unsigned char buffer[] = { 'a', 0xC2, 0xA2, 'a' };
      char result[] = "xxxxxxxxxxxx";

      tok.type = fastjson::Token::ValueToken;
      tok.data.value.ptr = reinterpret_cast<char*>(buffer);
      tok.data.value.size = 4;
      tok.data.value.type_hint = fastjson::ValueType::StringHint;

      saru_assert(fastjson::serialize_inplace( &tok, result+1 ) );
      saru_assert_equal( std::string("x\"a\\u00A2a\"x"), std::string(result) );
    }

    void serialize_escaped_unicode_3byte()
    {
      unsigned char buffer[] = { 'a', 0xE2, 0x82, 0xAC, 'a' };
      char result[] = "xxxxxxxxxxxx";

      tok.type = fastjson::Token::ValueToken;
      tok.data.value.ptr = reinterpret_cast<char*>(buffer);
      tok.data.value.size = 5;
      tok.data.value.type_hint = fastjson::ValueType::StringHint;

      saru_assert(fastjson::serialize_inplace( &tok, result+1 ) );
      saru_assert_equal( std::string("x\"a\\u20ACa\"x"), std::string(result) );
    }

    void serialize_escaped_unicode_4byte()
    {
      unsigned char buffer[] = { 'a', 0xF0, 0x9D, 0x84, 0x9E, 'a' };
      char result[] = "xxxxxxxxxxxxxxxxxx";

      tok.type = fastjson::Token::ValueToken;
      tok.data.value.ptr = reinterpret_cast<char*>(buffer);
      tok.data.value.size = 6;
      tok.data.value.type_hint = fastjson::ValueType::StringHint;

      saru_assert(fastjson::serialize_inplace( &tok, result+1 ) );
      saru_assert_equal( std::string("x\"a\\uD834\\uDD1Ea\"x"), std::string(result) );
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
  SARU_TEST( TestFixture::serialize_escaped_unicode_1byte, logger);
  SARU_TEST( TestFixture::serialize_escaped_unicode_2byte, logger);
  SARU_TEST( TestFixture::serialize_escaped_unicode_3byte, logger);
  SARU_TEST( TestFixture::serialize_escaped_unicode_4byte, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}
