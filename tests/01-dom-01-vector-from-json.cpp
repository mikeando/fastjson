// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson


#include "saru_cxx.h"
#include "fastjson/core.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:
    void test_create()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;
      saru_assert( fastjson::dom::parse_string("[1,2,3]", &token, &chunk, NULL, NULL ) );

      std::vector<int> values;
      saru_assert( fastjson::dom::json_helper<std::vector<int> >::from_json_value(&token, &values) );
      saru_assert_equal(3, values.size() );
      saru_assert_equal(1, values[0] );
      saru_assert_equal(2, values[1] );
      saru_assert_equal(3, values[2] );
    }

};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_create, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}

