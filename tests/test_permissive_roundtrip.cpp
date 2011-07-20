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

    void round_trip_int_key()
    {
      std::string in_str("{2:\"x\"}");

      saru_assert( fastjson::dom::parse_string(in_str, &token, &chunk, fastjson::mode::ext_any_as_key, &ErrorGetter::on_error, &error_getter ) );
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }

    void round_trip_int_key2()
    {
      std::string in_str("{2:\"x\"}");
      saru_assert( ! fastjson::dom::parse_string(in_str, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert_equal("Unexpected character while parsing dict start", error_getter.error_message );
    }

    void round_trip_int_key3()
    {
      std::string in_str("{1:\"a\",2:\"x\"}");

      if( ! fastjson::dom::parse_string(in_str, &token, &chunk, fastjson::mode::ext_any_as_key, &ErrorGetter::on_error, &error_getter ) )
      {
        saru_error( error_getter.error_message );
      }
      saru_assert_equal( in_str, fastjson::as_string( &token ) ); 
    }
};

int main()
{
  saru::TestLogger logger;

  SARU_TEST( TestFixture::round_trip_int_key, logger);
  SARU_TEST( TestFixture::round_trip_int_key2, logger);
  SARU_TEST( TestFixture::round_trip_int_key3, logger);

  logger.printSummary();

  return logger.allOK()?0:1;
}
