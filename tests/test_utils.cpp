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
  static bool build( Token * tok, Document * doc, const CustomObject & obj )
  {
    tok->type=Token::DictToken;
    tok->data.dict.ptr=NULL;
    Dictionary d(doc,tok);
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
      fastjson::dom::Document doc;
      saru_assert_equal( 0u, doc.n_available_array() );
      doc.create_and_add_array_page( 10 ); //This should give us 10 more array entries
      saru_assert_equal( 10u, doc.n_available_array() );
      doc.create_and_add_array_page( 10 ); //This should give us 10 more array entries
      saru_assert_equal( 20u, doc.n_available_array() );
    }

    void test_document_free_list_basic()
    {
      fastjson::dom::Document doc;
      saru_assert_equal( 0u, doc.n_available_array() );
      doc.create_and_add_array_page( 10 ); //This should give us 10 more array entries
      saru_assert_equal( 10u, doc.n_available_array() );
      fastjson::dom::PageEntry<fastjson::ArrayEntry> * a = doc.create_array_entry();
      saru_assert( a );
      saru_assert_equal( 9u, doc.n_available_array() );
      doc.destroy_array( a );
      saru_assert_equal( 10u, doc.n_available_array() );
    }


    void test_empty_dict()
    {
      // First create a dictionary
      fastjson::dom::Document doc;
      fastjson::dom::Dictionary * dict = doc.create_dictionary();
  
      // Now lets get it back as a string...
      saru_assert_equal("{}",fastjson::as_string( dict->token() ) );
    }

    void test_dict_add_kv()
    {
      // First create a dictionary
      fastjson::dom::Document doc;
      fastjson::dom::Dictionary * dict = doc.create_dictionary();
  
      // Then add a simple kv pair
      dict->add<std::string>("key","value");

      // Now lets get it back as a string...
      saru_assert_equal("{\"key\":\"value\"}",fastjson::as_string( dict->token() ) );
    }

    void test_empty_array()
    {
      // First create an array
      fastjson::dom::Document doc;
      fastjson::dom::Array * array = doc.create_free_array();
  
      // Now lets get it back as a string...
      saru_assert_equal("[]",fastjson::as_string( array->token() ) );
    }

    void test_simple_array()
    {
      // First create an array
      fastjson::dom::Document doc;
      fastjson::dom::Array * array = doc.create_free_array();
  
      array->add<std::string>("hello");
      array->add<std::string>("world");
  
      // Now lets get it back as a string...
      saru_assert_equal("[\"hello\",\"world\"]",fastjson::as_string( array->token() ) );
    }

    void test_empty_dict_json_node()
    {
      fastjson::dom::Document doc;
      fastjson::dom::JsonNode n = fastjson::dom::JsonNode::dict( &doc );
      saru_assert_equal("{}", n.as_string() );
    }

    void test_simple_dict_json_node()
    {
      fastjson::dom::Document doc;
      fastjson::dom::JsonNode n = fastjson::dom::JsonNode::dict( &doc );
      n.add<std::string>("h","Hello");
      n.add<std::string>("w","World");
      saru_assert_equal("{\"h\":\"Hello\",\"w\":\"World\"}", n.as_string() );
    }

    void test_empty_array_json_node()
    {
      fastjson::dom::Document doc;
      fastjson::dom::JsonNode n = fastjson::dom::JsonNode::array( &doc );
      saru_assert_equal("[]", n.as_string() );
    }

    void test_simple_array_json_node()
    {
      fastjson::dom::Document doc;
      fastjson::dom::JsonNode n = fastjson::dom::JsonNode::array( &doc );
      n.add<std::string>("Hello");
      n.add<std::string>("World");
      saru_assert_equal("[\"Hello\",\"World\"]", n.as_string() );
    }

    void test_simple_array_json_node2()
    {
      fastjson::dom::Document doc;
      fastjson::dom::JsonNode n = fastjson::dom::JsonNode::array( &doc );
      bool ok = n.add<std::string>("Hello");
      saru_assert(ok);
      ok = n.add<int>(-11);
      saru_assert(ok);
      ok = n.add<float>(2.5);
      saru_assert(ok);

      saru_assert_equal("[\"Hello\",-11,2.5]", n.as_string() );
    }

    void test_nested_array_json()
    {
      fastjson::dom::Document doc;
      fastjson::dom::JsonNode n = fastjson::dom::JsonNode::array( &doc );
      fastjson::dom::JsonNode sn = fastjson::dom::JsonNode::array( &doc );
      bool ok = n.add<fastjson::dom::JsonNode>( sn );

      saru_assert(ok);
      saru_assert_equal("[[]]", n.as_string() );
    }

    void test_convert_from_vector_of_int()
    {
      fastjson::dom::Document doc;
      fastjson::Token * tok = doc.create_free_token();
      std::vector<int> rv;
      bool ok = fastjson::dom::json_helper< std::vector<int> >::build( tok, &doc, rv );
      saru_assert(ok);

      saru_assert_equal("[]", fastjson::as_string(tok) );

      rv.push_back(1); 
      rv.push_back(5); 
      rv.push_back(9); 
      fastjson::dom::json_helper< std::vector<int> >::build( tok, &doc, rv );
      saru_assert_equal("[1,5,9]", fastjson::as_string(tok) );
    }

    void test_convert_from_vector_of_string()
    {
      fastjson::dom::Document doc;
      fastjson::Token * tok = doc.create_free_token();
      std::vector<std::string> rv;
      bool ok = fastjson::dom::json_helper< std::vector<std::string> >::build( tok, &doc, rv );
      saru_assert(ok);

      saru_assert_equal("[]", fastjson::as_string(tok) );

      rv.push_back("hello"); 
      rv.push_back("funky"); 
      rv.push_back("chicken"); 
      fastjson::dom::json_helper< std::vector<std::string> >::build( tok, &doc, rv );
      saru_assert_equal("[\"hello\",\"funky\",\"chicken\"]", fastjson::as_string(tok) );
    }


    void test_custom_object()
    {
      fastjson::dom::Document doc;
      fastjson::Token * tok = doc.create_free_token();
      CustomObject obj(1,"hello",5);
      bool ok= fastjson::dom::json_helper< CustomObject >::build( tok, &doc, obj );
      saru_assert( ok );
      saru_assert_equal("{\"foo\":1,\"message\":\"hello\",\"count\":5}", fastjson::as_string(tok) );
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


  SARU_TEST( TestFixture::test_empty_dict_json_node, logger);
  SARU_TEST( TestFixture::test_simple_dict_json_node, logger);
  SARU_TEST( TestFixture::test_empty_array_json_node, logger);
  SARU_TEST( TestFixture::test_simple_array_json_node, logger);
  SARU_TEST( TestFixture::test_simple_array_json_node2, logger);
  SARU_TEST( TestFixture::test_nested_array_json, logger);
  SARU_TEST( TestFixture::test_convert_from_vector_of_int, logger);
  SARU_TEST( TestFixture::test_convert_from_vector_of_string, logger);
  SARU_TEST( TestFixture::test_custom_object, logger);

  logger.printSummary();

  return logger.allOK()?0:1;
}
