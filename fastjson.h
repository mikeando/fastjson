#ifndef FASTJSON_H
#define FASTJSON_H

#include <stdint.h>
#include <string>

namespace fastjson
{
  struct JsonElementCount
  {
    JsonElementCount() :
      arrays(0),
      dicts(0),
      strings(0),
      total_string_length(0),
      array_elements(0),
      dict_elements(0)
    {}

    uint32_t arrays;
    uint32_t dicts;
    uint32_t strings;
    uint32_t total_string_length;
    uint32_t array_elements;
    uint32_t dict_elements;
  };

  bool parse_json_counts( const std::string & json_str, JsonElementCount * count );
  bool parse_json_counts( const char * start, const char * end, JsonElementCount * count );
}

#endif
