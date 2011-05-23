// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson/fastjson.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:
    void round_trip_easy()
    {
      std::string in_str("\"hello\"");

      fastjson::dom::Chunk chunk;
      fastjson::Token token;
      std::string error_message;

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, &error_message ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void round_trip_hard()
    {
      std::string in_str("\"\\t\\n\\\\\"");
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, &error_message ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void round_trip_tab()
    {
      std::string in_str("\"xx\\txx\"");
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, &error_message ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void read_tab()
    {
      std::string in_str("\"xx\\txx\"");
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, &error_message ) );
      saru_assert_equal( fastjson::Token::ValueToken, token.type );
      saru_assert_equal( 5u, token.data.value.size );
      saru_assert( token.data.value.ptr );
      saru_assert_equal( "xx\txx", std::string( token.data.value.ptr, token.data.value.size ) );
    }

    void write_tab()
    {
      char * buffer = "xx\txx";
      fastjson::Token token;
      token.type = fastjson::Token::ValueToken;
      token.data.value.type_hint = fastjson::ValueType::StringHint;
      token.data.value.size = 5;
      token.data.value.ptr = buffer;

      saru_assert_equal( "\"xx\\txx\"", fastjson::as_string( &token ) ); 
    }
};

int main()
{
  saru::TestLogger logger;

  SARU_TEST( TestFixture::round_trip_easy, logger);
  SARU_TEST( TestFixture::round_trip_hard, logger);
  SARU_TEST( TestFixture::round_trip_tab, logger);
  SARU_TEST( TestFixture::read_tab, logger);
  SARU_TEST( TestFixture::write_tab, logger);

  logger.printSummary();

  return logger.allOK()?0:1;
}
