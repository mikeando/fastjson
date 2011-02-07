// SARU : Tag fastjson
// SARU : CxxExtraObjects fastjson.cpp

#include "saru_cxx.h"

#include "fastjson.h"

class TestFixture
{
  public:
    fastjson::JsonElementCount count;

    void test_invalid_array()
    {
      bool ok = fastjson::parse_json_counts( "[[]", &count );
      saru_assert( !ok );
    }

    void test_invalid_array2()
    {
      bool ok = fastjson::parse_json_counts( "[]]", &count );
      saru_assert( !ok );
    }

    void test_invalid_dict()
    {
      bool ok = fastjson::parse_json_counts( "{", &count );
      saru_assert( !ok );
    }

    void test_invalid_dict2()
    {
      bool ok = fastjson::parse_json_counts( "}", &count );
      saru_assert( !ok );
    }

    void test_mismatchA()
    {
      bool ok = fastjson::parse_json_counts( "[}", &count );
      saru_assert( !ok );
    }

    void test_mismatchB()
    {
      bool ok = fastjson::parse_json_counts( "{]", &count );
      saru_assert( !ok );
    }

    void count_empty_array()
    {
      std::string json_str( "[]" );
      bool ok = fastjson::parse_json_counts( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 1u, count.arrays );
      saru_assert_equal( 0u, count.dicts );
      saru_assert_equal( 0u, count.total_string_length );
    } 

    void count_empty_dict()
    {
      std::string json_str( "{}" );
      bool ok = fastjson::parse_json_counts( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 0u, count.arrays );
      saru_assert_equal( 1u, count.dicts );
      saru_assert_equal( 0u, count.total_string_length );
    }

    void count_nested_array()
    {
      std::string json_str( "[[]]" );
      bool ok = fastjson::parse_json_counts( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 2u, count.arrays );
      saru_assert_equal( 0u, count.dicts );
      saru_assert_equal( 0u, count.total_string_length );
    }
 
    void count_nested_array2()
    {
      std::string json_str( "[[],[]]" );
      bool ok = fastjson::parse_json_counts( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 3u, count.arrays );
      saru_assert_equal( 0u, count.dicts );
      saru_assert_equal( 0u, count.total_string_length );
      saru_assert_equal( 2u, count.array_elements );
    } 

    //Lets try some strings
    void count_simple_string()
    {
      std::string json_str( "\"AAA\"" );
      bool ok = fastjson::parse_json_counts( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 0u, count.arrays );
      saru_assert_equal( 0u, count.dicts );
      saru_assert_equal( 1u, count.strings );
      saru_assert_equal( 3u, count.total_string_length );
    }

    void count_odd_string()
    {
      std::string json_str( "\"[]{}\"" );
      bool ok = fastjson::parse_json_counts( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 0u, count.arrays );
      saru_assert_equal( 0u, count.dicts );
      saru_assert_equal( 1u, count.strings );
      saru_assert_equal( 4u, count.total_string_length );
    }

    void count_invalid_string_mix()
    {
      saru_assert( ! fastjson::parse_json_counts( "\"[\"]", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "\"{\"}", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "[\"]\"", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "{1:\"}\"", &count ) );
    }

    void count_array_of_strings()
    {
      saru_assert( fastjson::parse_json_counts( "[\"hello\",\" \",\"world\"]", &count ) );
      saru_assert_equal(  3u, count.strings );
      saru_assert_equal( 11u, count.total_string_length );
      saru_assert_equal(  1u, count.arrays );
      saru_assert_equal(  3u, count.array_elements );
    }

    void count_invalid_array()
    {
      saru_assert( ! fastjson::parse_json_counts( "[ x ]", &count ) );
      //Should we be lenient and allow these? Or should taht be optional?
      saru_assert( ! fastjson::parse_json_counts( "[ , ]", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "[ [], ]", &count ) );
    }

    void count_dict_of_strings()
    {
      saru_assert( fastjson::parse_json_counts( "{ \"hello\" : \"world\", \"world\":\"imba\" }", &count ) );
      saru_assert_equal(  4u, count.strings );
      saru_assert_equal( 19u, count.total_string_length );
      saru_assert_equal(  0u, count.arrays );
      saru_assert_equal(  0u, count.array_elements );
      saru_assert_equal(  1u, count.dicts );
      saru_assert_equal(  2u, count.dict_elements );
    }

    void count_invalid_dict()
    {
      saru_assert( ! fastjson::parse_json_counts( "{ x }", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "{ \"f\" }", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "{ \"f\", }", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "{ :\"g\" }", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "{ :\"g\", }", &count ) );

      //Should we be lenient and allow these? Or should that be optional?
      saru_assert( ! fastjson::parse_json_counts( "{ , }", &count ) );
      saru_assert( ! fastjson::parse_json_counts( "{ \"f\":\"g\", }", &count ) );
    }

};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_invalid_array, logger);
  SARU_TEST( TestFixture::test_invalid_array2, logger);
  SARU_TEST( TestFixture::test_invalid_dict, logger);
  SARU_TEST( TestFixture::test_invalid_dict2, logger);
  SARU_TEST( TestFixture::test_mismatchA, logger);
  SARU_TEST( TestFixture::test_mismatchB, logger);
  SARU_TEST( TestFixture::count_empty_array, logger);
  SARU_TEST( TestFixture::count_empty_dict, logger);
  SARU_TEST( TestFixture::count_nested_array, logger);
  SARU_TEST( TestFixture::count_nested_array2, logger);
  SARU_TEST( TestFixture::count_simple_string, logger);
  SARU_TEST( TestFixture::count_odd_string, logger);
  SARU_TEST( TestFixture::count_invalid_string_mix, logger);
  SARU_TEST( TestFixture::count_array_of_strings, logger);
  SARU_TEST( TestFixture::count_invalid_array, logger);
  SARU_TEST( TestFixture::count_dict_of_strings, logger);
  SARU_TEST( TestFixture::count_invalid_dict, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}
