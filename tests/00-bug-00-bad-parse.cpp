// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

// =======
// Symptom
// =======
// When parsing this {"method":"ScriptController::run_auth","params":[942211527,"zff",{"z":"456"},null]}
// and putting into a dom::Dictionary object get<std::string>("method",...) was not finding the method.
//



#include "saru_cxx.h"
#include "fastjson/core.h"
#include "fastjson/dom.h"

class TestFixture
{
  public:
    void test_all()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token token;
      saru_assert( fastjson::dom::parse_string("{\"method\":\"ScriptController::run_auth\",\"params\":[942211527,\"zff\",{\"z\":\"456\"},null]}", &token, &chunk, NULL ) );

      saru_assert_equal( fastjson::Token::DictToken, token.type );
      fastjson::dom::Dictionary dict = fastjson::dom::Dictionary::as_dict( &token, &chunk );

      std::string method;
      saru_assert( dict.get<std::string>("method", &method ) );
      saru_assert_equal("ScriptController::run_auth", method );
    }

    void test_json_helper_string_from_json()
    {
      char * buffer="hello";
      fastjson::Token token;
      token.type = fastjson::Token::ValueToken;
      token.data.value.ptr = buffer;
      token.data.value.size = 5;
      token.data.value.type_hint = fastjson::ValueType::StringHint;

      std::string v;
      saru_assert( fastjson::dom::json_helper<std::string>::from_json_value( &token, &v) );
      saru_assert_equal("hello", v);
    }

};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_all, logger);
  SARU_TEST( TestFixture::test_json_helper_string_from_json, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}

