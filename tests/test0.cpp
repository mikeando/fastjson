// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson/fastjson.h"

class TestFixture
{
  public:
    fastjson::JsonElementCount count;

    void test_invalid_array()
    {
      bool ok = fastjson::count_elements( "[[]", &count );
      saru_assert( !ok );
    }

    void test_invalid_array2()
    {
      bool ok = fastjson::count_elements( "[]]", &count );
      saru_assert( !ok );
    }

    void test_invalid_dict()
    {
      bool ok = fastjson::count_elements( "{", &count );
      saru_assert( !ok );
    }

    void test_invalid_dict2()
    {
      bool ok = fastjson::count_elements( "}", &count );
      saru_assert( !ok );
    }

    void test_mismatchA()
    {
      bool ok = fastjson::count_elements( "[}", &count );
      saru_assert( !ok );
    }

    void test_mismatchB()
    {
      bool ok = fastjson::count_elements( "{]", &count );
      saru_assert( !ok );
    }

    void count_empty_array()
    {
      std::string json_str( "[]" );
      bool ok = fastjson::count_elements( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 1u, count.n_arrays() );
      saru_assert_equal( 0u, count.n_dicts() );
      saru_assert_equal( 0u, count.n_string_length() );
    }

    void count_empty_dict()
    {
      std::string json_str( "{}" );
      bool ok = fastjson::count_elements( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 0u, count.n_arrays() );
      saru_assert_equal( 1u, count.n_dicts() );
      saru_assert_equal( 0u, count.n_string_length() );
    }

    void count_nested_array()
    {
      std::string json_str( "[[]]" );
      bool ok = fastjson::count_elements( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 2u, count.n_arrays() );
      saru_assert_equal( 0u, count.n_dicts() );
      saru_assert_equal( 0u, count.n_string_length() );
    }

    void count_nested_array2()
    {
      std::string json_str( "[[],[]]" );
      bool ok = fastjson::count_elements( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 3u, count.n_arrays() );
      saru_assert_equal( 0u, count.n_dicts() );
      saru_assert_equal( 0u, count.n_string_length() );
      saru_assert_equal( 2u, count.n_array_elements() );
    }

    //Lets try some strings
    void count_simple_string()
    {
      std::string json_str( "\"AAA\"" );
      bool ok = fastjson::count_elements( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 0u, count.n_arrays() );
      saru_assert_equal( 0u, count.n_dicts() );
      saru_assert_equal( 1u, count.n_strings() );
      saru_assert_equal( 3u, count.n_string_length() );
    }

    void count_odd_string()
    {
      std::string json_str( "\"[]{}\"" );
      bool ok = fastjson::count_elements( json_str, &count );
      saru_assert( ok );
      saru_assert_equal( 0u, count.n_arrays() );
      saru_assert_equal( 0u, count.n_dicts() );
      saru_assert_equal( 1u, count.n_strings() );
      saru_assert_equal( 4u, count.n_string_length() );
    }

    void count_invalid_string_mix()
    {
      saru_assert( ! fastjson::count_elements( "\"[\"]", &count ) );
      saru_assert( ! fastjson::count_elements( "\"{\"}", &count ) );
      saru_assert( ! fastjson::count_elements( "[\"]\"", &count ) );
      saru_assert( ! fastjson::count_elements( "{1:\"}\"", &count ) );
    }

    void count_array_of_strings()
    {
      saru_assert( fastjson::count_elements( "[\"hello\",\" \",\"world\"]", &count ) );
      saru_assert_equal(  3u, count.n_strings() );
      saru_assert_equal( 11u, count.n_string_length() );
      saru_assert_equal(  1u, count.n_arrays() );
      saru_assert_equal(  3u, count.n_array_elements() );
    }

    void count_invalid_array()
    {
      saru_assert( ! fastjson::count_elements( "[ x ]", &count ) );
      //Should we be lenient and allow these? Or should taht be optional?
      saru_assert( ! fastjson::count_elements( "[ , ]", &count ) );
      saru_assert( ! fastjson::count_elements( "[ [], ]", &count ) );
    }

    void count_dict_of_strings()
    {
      saru_assert( fastjson::count_elements( "{ \"hello\" : \"world\", \"world\":\"imba\" }", &count ) );
      saru_assert_equal(  4u, count.n_strings() );
      saru_assert_equal( 19u, count.n_string_length() );
      saru_assert_equal(  0u, count.n_arrays() );
      saru_assert_equal(  0u, count.n_array_elements() );
      saru_assert_equal(  1u, count.n_dicts() );
      saru_assert_equal(  2u, count.n_dict_elements() );
    }

    void count_invalid_dict()
    {
      saru_assert( ! fastjson::count_elements( "{ x }", &count ) );
      saru_assert( ! fastjson::count_elements( "{ \"f\" }", &count ) );
      saru_assert( ! fastjson::count_elements( "{ \"f\", }", &count ) );
      saru_assert( ! fastjson::count_elements( "{ :\"g\" }", &count ) );
      saru_assert( ! fastjson::count_elements( "{ :\"g\", }", &count ) );

      //Should we be lenient and allow these? Or should that be optional?
      saru_assert( ! fastjson::count_elements( "{ , }", &count ) );
      saru_assert( ! fastjson::count_elements( "{ \"f\":\"g\", }", &count ) );
    }


    void count_complex_strings()
    {
      saru_assert( fastjson::count_elements( "\"\\u0000\"", &count ) ); //unicode null "\u0000";
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(1u, count.n_string_length() ); //Should be encoded as {0x00} which is one char.
    }

    void count_1byte_utf8()
    {
      //Simple 1-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {0x7F}
      saru_assert( fastjson::count_elements( "\"\\u007F\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(1u, count.n_string_length() );
    }

    void count_2byte_utf8_min()
    {
      //Simple 2-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {0xC2,0x80}
      saru_assert( fastjson::count_elements( "\"\\u0080\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(2u, count.n_string_length() );
    }

    void count_2byte_utf8_typical()
    {
      //Simple 2-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {0xC2,0xA2}
      saru_assert( fastjson::count_elements( "\"\\u00A2\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(2u, count.n_string_length() );
    }

    void count_2byte_utf8_max()
    {
      //Simple 2-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {0xDF,0xBF}
      saru_assert( fastjson::count_elements( "\"\\u07FF\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(2u, count.n_string_length() );
    }

    void count_3byte_utf8_min()
    {
      //Simple 2-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {E0, A0, 80}
      saru_assert( fastjson::count_elements( "\"\\u0800\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(3u, count.n_string_length() );
    }

    void count_3byte_utf8_typical()
    {
      //Simple 2-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {E2, 82, AC}
      saru_assert( fastjson::count_elements( "\"\\u20AC\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(3u, count.n_string_length() );
    }

    void count_3byte_utf8_max()
    {
      //Simple 3-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {EF, BF, BF}
      saru_assert( fastjson::count_elements( "\"\\uFFFF\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(3u, count.n_string_length() );
    }

    void count_utf8_pair_nonsurrogate()
    {
      //Two 3-byte utf-8 character using the \u esacpe
      //This should translate to the utf-8 value of {E2, 82, AC, E2, 82, AC }
      saru_assert( fastjson::count_elements( "\"\\u20AC\\u20AC\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(6u, count.n_string_length() );
    }

    void count_evil_gclef_string()
    {
      //This is an evil G clef U+1D11E character in its UTF-16 surrogate-pair form
      //This should translate to the utf-8 value of {0xF0,0x9D,0x84,0x9E}
      saru_assert( fastjson::count_elements( "\"\\uD834\\uDD1E\"", &count ) );
      saru_assert_equal(1u, count.n_strings() );
      saru_assert_equal(4u, count.n_string_length() );
    }

    void count_invalid_escape_u()
    {
      saru_assert( ! fastjson::count_elements( "\"\\u\"", &count ) );
      saru_assert( ! fastjson::count_elements( "\"\\u0\"", &count ) );
      saru_assert( ! fastjson::count_elements( "\"\\u00\"", &count ) );
      saru_assert( ! fastjson::count_elements( "\"\\u000\"", &count ) );
      saru_assert(   fastjson::count_elements( "\"\\u0000\"", &count ) );
    }

    void count_invalid_utf8()
    {
      saru_assert(false);
    }

    void count_just_stupid()
    {
      saru_assert( ! fastjson::count_elements( "x", &count ) );
    }

    void count_number_simple_int()
    {
      saru_assert( fastjson::count_elements( "1234", &count ) );
      saru_assert_equal( 1u, count.n_strings() );
      saru_assert_equal( 4u, count.n_string_length() );
    }

    void count_number_neg_int()
    {
      saru_assert( fastjson::count_elements( "-1234", &count ) );
      saru_assert_equal( 1u, count.n_strings() );
      saru_assert_equal( 5u, count.n_string_length() );
    }

    void count_number_simple_float()
    {
      saru_assert( fastjson::count_elements( "1234.1234", &count ) );
      saru_assert_equal( 1u, count.n_strings() );
      saru_assert_equal( 9u, count.n_string_length() );
    }

    void count_bare_litteral_true()
    {
      saru_assert( fastjson::count_elements( "true", &count ) );
      saru_assert_equal( 0, count.n_strings() );
      saru_assert_equal( 0u, count.n_array_elements() );
      saru_assert_equal( 0u, count.n_dict_elements() );
    }

    void count_bare_litteral_false()
    {
      saru_assert( fastjson::count_elements( "false", &count ) );
      saru_assert_equal( 0, count.n_strings() );
      saru_assert_equal( 0u, count.n_array_elements() );
      saru_assert_equal( 0u, count.n_dict_elements() );
    }

    void count_bare_litteral_null()
    {
      saru_assert( fastjson::count_elements( "null", &count ) );
      saru_assert_equal( 0, count.n_strings() );
      saru_assert_equal( 0u, count.n_array_elements() );
      saru_assert_equal( 0u, count.n_dict_elements() );
    }

    void count_litteral_true()
    {
      saru_assert( fastjson::count_elements( "[true]", &count ) );
      saru_assert_equal( 0, count.n_strings() );
      saru_assert_equal( 1u, count.n_array_elements() );
      saru_assert_equal( 0u, count.n_dict_elements() );
    }

    void count_litteral_false()
    {
      saru_assert( fastjson::count_elements( "[false]", &count ) );
      saru_assert_equal( 0, count.n_strings() );
      saru_assert_equal( 1u, count.n_array_elements() );
      saru_assert_equal( 0u, count.n_dict_elements() );
    }

    void count_litteral_null()
    {
      saru_assert( fastjson::count_elements( "[null]", &count ) );
      saru_assert_equal( 0, count.n_strings() );
      saru_assert_equal( 1u, count.n_array_elements() );
      saru_assert_equal( 0u, count.n_dict_elements() );
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
  SARU_TEST( TestFixture::count_complex_strings, logger);
  SARU_TEST( TestFixture::count_invalid_escape_u, logger);
  SARU_TEST( TestFixture::count_1byte_utf8, logger);
  SARU_TEST( TestFixture::count_2byte_utf8_min, logger);
  SARU_TEST( TestFixture::count_2byte_utf8_typical, logger);
  SARU_TEST( TestFixture::count_2byte_utf8_max, logger);
  SARU_TEST( TestFixture::count_3byte_utf8_min, logger);
  SARU_TEST( TestFixture::count_3byte_utf8_typical, logger);
  SARU_TEST( TestFixture::count_3byte_utf8_max, logger);
  SARU_TEST( TestFixture::count_utf8_pair_nonsurrogate, logger);
  SARU_TEST( TestFixture::count_evil_gclef_string, logger);
  SARU_TEST( TestFixture::count_just_stupid, logger);
  SARU_TEST( TestFixture::count_number_simple_int, logger);
  SARU_TEST( TestFixture::count_number_neg_int, logger);
  SARU_TEST( TestFixture::count_number_simple_float, logger);
  SARU_TEST( TestFixture::count_bare_litteral_true, logger);
  SARU_TEST( TestFixture::count_bare_litteral_false, logger);
  SARU_TEST( TestFixture::count_bare_litteral_null, logger);
  SARU_TEST( TestFixture::count_litteral_true, logger);
  SARU_TEST( TestFixture::count_litteral_false, logger);
  SARU_TEST( TestFixture::count_litteral_null, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}
