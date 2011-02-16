//SARU : Tag dummy
// SARU : CxxLibs -L.. -lfastjson

#include "fastjson.h"
#include <iostream>


void process_json( const char * filename )
{
  
  FILE* fp=fopen(filename,"rb");
  if( fp==0 )
  {
    std::cerr<<"Unable to open file "<<filename<<std::endl;
    return;
  }

  fseek(fp,0,SEEK_END); //go to end
  long len=ftell(fp); //get position at end (length)
  fseek(fp,0,SEEK_SET); //go to beg.
  char * json= new char[len]; //malloc buffer
  fread(json,len,1,fp); //read into buffer
  fclose(fp);

  fastjson::JsonElementCount count;
  fastjson::count_elements( std::string(json,json+len), &count );
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

  bool ok = fastjson::parse_doc( std::string(json,json+len), &doc );
  std::cout<<"Parsed OK = "<<ok<<std::endl;

  delete [] strings;
  delete [] arrays;
  delete [] dicts;

}

int main()
{
  process_json( "ugly.json" );
}
