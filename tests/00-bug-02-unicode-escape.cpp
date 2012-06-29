// SARU : Tag fastjson




#include "saru_cxx.h"

#include "../src/fastjson.cpp"

struct ErrorHandler
{
  bool called;
  ErrorHandler() : called(false) {}

  void on_error( const fastjson::ErrorContext & ec ) { called = true; }
};

class TestFixture
{
  public:
    void test_hex_characters()
    {
      unsigned char values[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
        'A', 'B', 'C', 'D', 'E', 'F',
        'a', 'b', 'c', 'd', 'e', 'f', 
      };
      for( unsigned int i=0; i<sizeof(values); ++i )
      {
        std::cerr<<"Checking "<<values[i]<<std::endl;
        saru_assert(  fastjson::internal::ishex( values[i] ) );
      } 


    }
    void test_converts_unicode_correctly()
    {
      unsigned char buffer[] = { '\\', 'u', '0', '0', 'b', '0' };
      uint32_t code_point=0;
      ErrorHandler eh;
      fastjson::read_unicode_escape<ErrorHandler>( buffer, buffer+6, &code_point, &eh );
      saru_assert( ! eh.called );
      saru_assert_equal(176u, code_point);
    }
    void test_converts_unicode_correctly2()
    {
      unsigned char buffer[] = { '\\', 'u', '2', '1', '0', '3' };
      uint32_t code_point=0;
      ErrorHandler eh;
      fastjson::read_unicode_escape<ErrorHandler>( buffer, buffer+6, &code_point, &eh );
      saru_assert( ! eh.called );
      saru_assert_equal(8451, code_point);
    }

};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_hex_characters, logger);
  SARU_TEST( TestFixture::test_converts_unicode_correctly, logger);
  SARU_TEST( TestFixture::test_converts_unicode_correctly2, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}

