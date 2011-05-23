// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson/fastjson.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:


    void test_clone_mega()
    {
      std::string json_string("{\"hello\":[\"world\",123,4.5],\"say\":{\"moo\":\"cow\",\"eep\":null},\"say\":{\"moo\":\"cow\",\"eep\":null},\"say2\":{\"moo\":\"cow\",\"eep\":null},\"say3\":{\"moo\":\"cow\",\"eep\":null},\"say4\":{\"moo\":\"cow\",\"eep\":null},\"say5\":{\"moo\":\"cow\",\"eep\":null},\"say6\":{\"moo\":\"cow\",\"eep\":null}}");

      fastjson::dom::Chunk copy_chunk;
      fastjson::Token copy_token;

      // This scoping ensures that the originating chunk has gone out of scope
      // and has been deleted by the time the copy is used. This means that if
      // it was pointing into the old data in any way it should cause valgrind
      // to complain.
      {
        fastjson::dom::Chunk chunk;
        fastjson::Token token;

        std::string error_message;
        saru_assert( fastjson::dom::parse_string(json_string, &token, &chunk, &error_message ) );

        fastjson::dom::clone_token( &token, &copy_token, &copy_chunk );
      }


      saru_assert( copy_token.type == fastjson::Token::DictToken );
      saru_assert_equal( json_string, fastjson::as_string( &copy_token ) ); 
    }
};

int main()
{
  saru::TestLogger logger;

  SARU_TEST( TestFixture::test_clone_mega, logger);

  logger.printSummary();

  return logger.allOK()?0:1;
}
