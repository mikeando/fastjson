#ifndef FASTJSON_H
#define FASTJSON_H

#include <stdint.h>
#include <string>

#include <vector>
namespace fastjson
{
  struct JsonElementCount
  {
    public:
      JsonElementCount() :
        arrays(0),
        dicts(0),
        strings(0),
        total_string_length(0)
    {
      context.push_back(Root);
      memset( counts, 0, sizeof(counts) );
    }


      void start_array()  { context.push_back( InArray ); }
      void start_dict()   { context.push_back( InDict );  }
      void start_string() {};
      void start_number() {};

      void end_array() { ++arrays; context.pop_back(); counts[ context.back() ]++; }
      void end_dict() { ++dicts; context.pop_back(); counts[ context.back() ]++; }

      void end_string() { ++strings; counts[ context.back() ]++; } //Should this be changed to be the same as end_number?
      void string_add_ubyte( const unsigned char ) { total_string_length++; }
      void end_number( const unsigned char * start, const unsigned char * end ) { ++strings; counts[ context.back() ]++; total_string_length = ( end-start); };

      uint32_t n_arrays()  const { return arrays; }
      uint32_t n_dicts()   const { return dicts; }
      uint32_t n_strings() const { return strings; }
      uint32_t n_array_elements() const { return counts[InArray]; }
      uint32_t n_dict_elements() const { return counts[InDict]/2; }
      uint32_t n_string_length() const { return total_string_length; }

    protected:
      uint32_t arrays;
      uint32_t dicts;
      uint32_t strings;
      uint32_t total_string_length;

      enum Context { Root, InArray, InDict, NContexts };
      uint32_t counts[NContexts];
      std::vector<Context> context;

    public:
  };

  bool parse_json_counts( const std::string & json_str, JsonElementCount * count );
  bool parse_json_counts( const unsigned char * start, const unsigned char * end, JsonElementCount * count );

  class Document
  {
    public:
    unsigned char * string_store;
  };

  bool parse_doc( const unsigned char * start, const unsigned char * end, Document * doc );
  bool parse_doc( const std::string & json_str, Document * doc );

}

#endif
