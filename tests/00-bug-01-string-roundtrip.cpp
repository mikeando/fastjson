// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson/fastjson.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:
    struct ErrorGetter
    {
      static void on_error( void * in_this, const fastjson::ErrorContext & ec )
      {
        static_cast<ErrorGetter*>(in_this)->error_message = ec.mesg;
      }

      std::string error_message;
    };

    ErrorGetter error_getter;
    fastjson::dom::Chunk chunk;
    fastjson::Token token;


    void round_trip_easy()
    {
      std::string in_str("\"hello\"");

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void round_trip_hard()
    {
      std::string in_str("\"\\t\\n\\\\\"");

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void round_trip_tab()
    {
      std::string in_str("\"xx\\txx\"");

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void read_tab()
    {
      std::string in_str("\"xx\\txx\"");

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert_equal( fastjson::Token::ValueToken, token.type );
      saru_assert_equal( 5u, token.value.size );
      saru_assert( token.value.ptr );
      saru_assert_equal( "xx\txx", std::string( token.value.ptr, token.value.size ) );
    }

    void write_tab()
    {
      char * buffer = "xx\txx";
      token.type = fastjson::Token::ValueToken;
      token.value.type_hint = fastjson::ValueType::StringHint;
      token.value.size = 5;
      token.value.ptr = buffer;

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
