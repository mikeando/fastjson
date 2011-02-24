// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:

    void test_create_from_string()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;
      saru_assert( fastjson::dom::parse_string("[]", &token, &chunk, &error_message ) );
      saru_assert( token.type == fastjson::Token::ArrayToken );
    }

    void test_create_from_string_bad()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;
      saru_assert( ! fastjson::dom::parse_string("[", &token, &chunk, &error_message ) );
    }

    void test_create_from_string_big_and_complex()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;
      saru_assert( fastjson::dom::parse_string("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null}}", &token, &chunk, &error_message ) );
      saru_assert( token.type == fastjson::Token::DictToken );
      saru_assert_equal( std::string("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null}}"), fastjson::as_string( &token ) ); 

    }
    void test_create_from_string_mega()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;

      std::string error_message;
      std::string json_string("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null},\"say\":{\"moo\":\"cow\",\"eep\":null},\"say2\":{\"moo\":\"cow\",\"eep\":null},\"say3\":{\"moo\":\"cow\",\"eep\":null},\"say4\":{\"moo\":\"cow\",\"eep\":null},\"say5\":{\"moo\":\"cow\",\"eep\":null},\"say6\":{\"moo\":\"cow\",\"eep\":null}}");
      saru_assert( fastjson::dom::parse_string(json_string, &token, &chunk, &error_message ) );
      saru_assert( token.type == fastjson::Token::DictToken );
      saru_assert_equal( json_string, fastjson::as_string( &token ) ); 
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
