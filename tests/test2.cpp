// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"
#include "fastjson.h"

class TestFixture
{
  public:
    void test_string_buffer()
    {
      unsigned char buffer[] = "xxxxxxxxxxxx"; 
      fastjson::Document doc;
      doc.string_store = buffer+1;

      bool ok = fastjson::parse_doc( "[\"hello\",\"world\"]", & doc );
      saru_assert(ok);

      saru_assert_equal( std::string("xhelloworldx"), std::string( reinterpret_cast<const char*>(buffer) ) );
    }

    void test_number_buffer()
    {
      unsigned char buffer[] = "xxxxxxxxxxxx";
      fastjson::Document doc;
      doc.string_store = buffer+1;

      bool ok = fastjson::parse_doc( "[12345,67890]", & doc );
      saru_assert(ok);

      saru_assert_equal( std::string("x1234567890x"), std::string( reinterpret_cast<const char*>(buffer) ) );
    }
};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_string_buffer, logger);
  SARU_TEST( TestFixture::test_number_buffer, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}

