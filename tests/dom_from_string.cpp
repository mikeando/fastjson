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
      ErrorGetter() : ec(NULL) {};
      ~ErrorGetter(){ delete ec; }
      static void on_error( void * in_this, const fastjson::ErrorContext & ec )
      {
        delete static_cast<ErrorGetter*>(in_this)->ec;
        static_cast<ErrorGetter*>(in_this)->ec = new fastjson::ErrorContext(ec);
      }

      fastjson::ErrorContext * ec;
    };

    fastjson::dom::Chunk chunk;
    fastjson::Token token;
    ErrorGetter error_getter;

    void test_create_from_string()
    {
      std::string json("[]");
      saru_assert( fastjson::dom::parse_string(json, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert( ! error_getter.ec );
      saru_assert( token.type == fastjson::Token::ArrayToken );
    }

    void test_create_from_string_bad()
    {
      std::string json("[");
      saru_assert( ! fastjson::dom::parse_string(json, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert( error_getter.ec );
      saru_assert_equal("Input ended while in non-root state", error_getter.ec->mesg );
    }

    void test_create_from_string_big_and_complex()
    {
      std::string json("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null}}");
      saru_assert( fastjson::dom::parse_string(json, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert( ! error_getter.ec );
      saru_assert( token.type == fastjson::Token::DictToken );
      saru_assert_equal( std::string("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null}}"), fastjson::as_string( &token ) ); 

    }
    void test_create_from_string_mega()
    {
      std::string json("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null},\"say\":{\"moo\":\"cow\",\"eep\":null},\"say2\":{\"moo\":\"cow\",\"eep\":null},\"say3\":{\"moo\":\"cow\",\"eep\":null},\"say4\":{\"moo\":\"cow\",\"eep\":null},\"say5\":{\"moo\":\"cow\",\"eep\":null},\"say6\":{\"moo\":\"cow\",\"eep\":null}}");
      saru_assert( fastjson::dom::parse_string(json, &token, &chunk, 0, &ErrorGetter::on_error, &error_getter ) );
      saru_assert( ! error_getter.ec );
      saru_assert( token.type == fastjson::Token::DictToken );
      saru_assert_equal( json, fastjson::as_string( &token ) ); 
    }
};

int main()
{
  saru::TestLogger logger;

  SARU_TEST( TestFixture::test_create_from_string, logger);
  SARU_TEST( TestFixture::test_create_from_string_bad, logger);
  SARU_TEST( TestFixture::test_create_from_string_big_and_complex, logger);
  SARU_TEST( TestFixture::test_create_from_string_mega, logger);

  logger.printSummary();

  return logger.allOK()?0:1;
}
