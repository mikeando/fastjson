// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson.h"
#include "fastjson/dom.h"

struct CustomObject
{
  CustomObject( int i, const std::string & m, int k) : i_(i), m_(m), k_(k) {}
  int i_;
  std::string m_;
  int k_;
};

namespace fastjson { namespace dom {
template<>
struct json_helper<CustomObject>
{
  static bool build( Token * tok, Chunk * chunk, const CustomObject & obj )
  {
    tok->type=Token::DictToken;
    tok->data.dict.ptr=NULL;
    Dictionary d = Dictionary::create_dict(tok,chunk);
    d.add<int>( "foo", obj.i_);
    d.add<std::string>( "message", obj.m_);
    d.add<int>( "count", obj.k_);
    return true;
  }
};
} }

class TestFixture
{
  public:

    void test_document_free_list_counts()
    {
      fastjson::dom::Chunk chunk;
      saru_assert_equal( 0u, chunk.arrays().n_available() );
      chunk.arrays().create_and_add_page( 10 ); //This should give us 10 more array entries
      saru_assert_equal( 10u, chunk.arrays().n_available() );
      chunk.arrays().create_and_add_page( 10 ); //This should give us 10 more array entries
      saru_assert_equal( 20u, chunk.arrays().n_available() );
    }

    void test_document_free_list_basic()
    {
      fastjson::dom::Chunk chunk;
      saru_assert_equal( 0u, chunk.arrays().n_available() );
      chunk.arrays().create_and_add_page( 10 ); //This should give us 10 more array entries
      saru_assert_equal( 10u, chunk.arrays().n_available() );
      fastjson::ArrayEntry * a = chunk.arrays().create_value();
      saru_assert( a );
      saru_assert_equal( 9u, chunk.arrays().n_available() );
      chunk.arrays().destroy( a );
      saru_assert_equal( 10u, chunk.arrays().n_available() );
    }


    void test_empty_dict()
    {
      // First create a dictionary
      fastjson::dom::Chunk chunk;
      fastjson::Token tok;
      fastjson::dom::Dictionary dict = fastjson::dom::Dictionary::create_dict(&tok,&chunk);
  
      // Now lets get it back as a string...
      saru_assert_equal("{}",fastjson::as_string( dict.token() ) );
    }

    void test_dict_add_kv()
    {
      // First create a dictionary
      fastjson::dom::Chunk chunk;
      fastjson::Token tok;
      fastjson::dom::Dictionary dict = fastjson::dom::Dictionary::create_dict(&tok,&chunk);
  
      // Then add a simple kv pair
      dict.add<std::string>("key","value");

      // Now lets get it back as a string...
      saru_assert_equal("{\"key\":\"value\"}",fastjson::as_string( dict.token() ) );
    }

    void test_empty_array()
    {
      // First create an array
      fastjson::dom::Chunk chunk;
      fastjson::Token tok;
      fastjson::dom::Array array = fastjson::dom::Array::create_array(&tok,&chunk);
  
      // Now lets get it back as a string...
      saru_assert_equal("[]",fastjson::as_string( array.token() ) );
    }

    void test_simple_array()
    {
      // First create an array
      fastjson::dom::Chunk chunk;
      fastjson::Token tok;
      fastjson::dom::Array array = fastjson::dom::Array::create_array(&tok,&chunk);
  
      array.add<std::string>("hello");
      array.add<std::string>("world");
  
      // Now lets get it back as a string...
      saru_assert_equal("[\"hello\",\"world\"]",fastjson::as_string( array.token() ) );
    }

    void test_convert_from_vector_of_int()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token tok;
      std::vector<int> rv;
      bool ok = fastjson::dom::json_helper< std::vector<int> >::build( &tok, &chunk, rv );
      saru_assert(ok);

      saru_assert_equal("[]", fastjson::as_string(&tok) );

      rv.push_back(1); 
      rv.push_back(5); 
      rv.push_back(9); 
      fastjson::dom::json_helper< std::vector<int> >::build( &tok, &chunk, rv );
      saru_assert_equal("[1,5,9]", fastjson::as_string(&tok) );
    }

    void test_convert_from_vector_of_string()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token  tok;
      std::vector<std::string> rv;
      bool ok = fastjson::dom::json_helper< std::vector<std::string> >::build( &tok, &chunk, rv );
      saru_assert(ok);

      saru_assert_equal("[]", fastjson::as_string(&tok) );

      rv.push_back("hello"); 
      rv.push_back("funky"); 
      rv.push_back("chicken"); 
      fastjson::dom::json_helper< std::vector<std::string> >::build( &tok, &chunk, rv );
      saru_assert_equal("[\"hello\",\"funky\",\"chicken\"]", fastjson::as_string(&tok) );
    }


    void test_custom_object()
    {
      fastjson::dom::Chunk chunk;
      fastjson::Token tok;
      CustomObject obj(1,"hello",5);
      bool ok= fastjson::dom::json_helper< CustomObject >::build( &tok, &chunk, obj );
      saru_assert( ok );
      saru_assert_equal("{\"foo\":1,\"message\":\"hello\",\"count\":5}", fastjson::as_string(&tok) );
    }

};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::test_document_free_list_counts, logger);
  SARU_TEST( TestFixture::test_document_free_list_basic, logger);
  SARU_TEST( TestFixture::test_empty_dict, logger);
  SARU_TEST( TestFixture::test_dict_add_kv, logger);
  SARU_TEST( TestFixture::test_empty_array, logger);
  SARU_TEST( TestFixture::test_simple_array, logger);

  SARU_TEST( TestFixture::test_convert_from_vector_of_int, logger);
  SARU_TEST( TestFixture::test_convert_from_vector_of_string, logger);
  SARU_TEST( TestFixture::test_custom_object, logger);

  logger.printSummary();

  return logger.allOK()?0:1;
}
