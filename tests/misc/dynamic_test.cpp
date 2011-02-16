//SARU : Tag dummy
// SARU : CxxLibs -L.. -lfastjson

#include "fastjson.h"
#include <iostream>

const char * json0 = "{}";
const char * json1 = "{\"hello\":\"world\"}";
const char * json2 = "{  \"hello\"  :  \"world\"  }";
const char * json3 = "{  \"hello\"  :  \"world\" , \"blah\"  :  \"foo\" }";

void process_json( const char * json )
{
  fastjson::JsonElementCount count;
  fastjson::count_elements( json, &count );
  std::cout<<"# strings = "<<count.n_strings()<<std::endl;
  std::cout<<"# dicts = "<<count.n_dicts()<<std::endl;
  std::cout<<"# array = "<<count.n_arrays()<<std::endl;
  std::cout<<"string length = "<<count.n_string_length()<<std::endl;
  std::cout<<"n dict elements = "<<count.n_dict_elements()<<std::endl;
  std::cout<<"n array elements = "<<count.n_array_elements()<<std::endl;

  //Allocate enoiugh space to parse the json properly
  //TODO: Why are my counts out by one?
  unsigned char * strings = new unsigned char[count.n_string_length()];
  fastjson::ArrayEntry * arrays = new fastjson::ArrayEntry[ count.n_array_elements()];
  fastjson::DictEntry * dicts = new fastjson::DictEntry[ count.n_dict_elements()];

  fastjson::Document doc;
  doc.string_store = strings;
  doc.array_store  = arrays;
  doc.dict_store   = dicts;

  bool ok = fastjson::parse_doc( json, &doc );
  std::cout<<"Parsed OK = "<<ok<<std::endl;

  delete [] strings;
  delete [] arrays;
  delete [] dicts;

}

int main()
{
  process_json( json0 );
  process_json( json1 );
  process_json( json2 );
  process_json( json3 );
}
