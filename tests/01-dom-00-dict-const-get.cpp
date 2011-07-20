// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson


#include "saru_cxx.h"
#include "fastjson/core.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:
    void test_get()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;
      saru_assert( fastjson::dom::parse_string("{\"key\":\"value\"}", &token, &chunk, 0, NULL, NULL ) );

      fastjson::dom::Dictionary_const dict = fastjson::dom::Dictionary_const::as_dict(&token);

      std::string value;
      saru_assert( dict.get<std::string>("key", &value ) );
      saru_assert_equal("value", value );
    }

};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_get, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}

