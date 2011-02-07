#include "fastjson.h"
#include <vector>
#include <iostream>

namespace fastjson
{
  bool parse_json_counts( const std::string & json_str, JsonElementCount * count )
  {
    return parse_json_counts( json_str.c_str() ,json_str.c_str() + json_str.size(), count );
  }

  namespace state
  {
    static const int start_root            = 0;

    static const int start_array           = 1;
    static const int continue_array        = 2;
    static const int require_array_element = 3;

    static const int dict_start            = 4;
    static const int dict_read_kv_sep      = 5;
    static const int dict_read_sep         = 6;
    static const int dict_read_key         = 7;
    static const int dict_read_value       = 8;

    static const int start_string          = 9;
  }

  struct ParserState
  {
    explicit ParserState( int in_state ) : state(in_state ), subelements(0) {}
    int state;
    uint32_t subelements;
  };

  bool parse_json_count_root(
    const char * cursor,
    const char * end,
    JsonElementCount * count,
    std::vector<ParserState> * state )
  {
    switch (*cursor)
    {
      case '[':
        state->push_back( ParserState(state::start_array) );
        break;
      case'{':
        state->push_back( ParserState(state::dict_start) );
        break;
      case ']':
        return false;
      case '}':
        return false;
      case '"':
        state->push_back( ParserState(state::start_string) );
        break;
      default:
        ;
    }
    return true;
  }

  bool parse_json_count_string(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch(*cursor)
    {
      case '"':
        //Are we ending a string?
        count->strings += 1;
        state->pop_back();
        break;
      default:
        count->total_string_length++;
    }
    return true;
  }

  //This is only called at the start of an array
  bool parse_json_count_array(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      case '[':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::continue_array;
        state->push_back( ParserState(state::start_array) );
        count->array_elements++;
        break;
      case '{':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::continue_array;
        state->push_back( ParserState(state::dict_start) );
        count->array_elements++;
      case ']':
        //NOTE: The array contains no elements so we don't need to increment array_elements
        // If the array were to have sub elements we'd hit the "]" in state::continue_array instead.
        count->arrays += 1;
        state->pop_back();
        break;
      case '}':
        return false;
      case '"':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::continue_array;
        state->push_back( ParserState(state::start_string) );
        count->array_elements++;
        break;
      case ' ':
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_count_array_continue(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      case ' ':
        break;
      case ',':
        state->back().state = state::require_array_element;
        break;
      case ']':
        count->arrays += 1;
        count->array_elements += state->back().subelements;
        state->pop_back();
        break;
      default:
        return false;
    }
    return true;
  }

 bool parse_json_count_require_element(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      case '[':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::continue_array;
        state->back().subelements++;
        state->push_back( ParserState(state::start_array) );
        break;
      case '{':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::continue_array;
        state->back().subelements++;
        state->push_back( ParserState(state::dict_start) );
      case ']':
        return false;
      case '}':
        return false;
      case '"':
        state->back().state = state::continue_array;
        state->back().subelements++;
        state->push_back( ParserState(state::start_string) );
        break;
      case ' ':
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_count_dict(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      //For now disallow arrays and dictionaries as keys
      case '[':
//        state->push_back( ParserState(state::start_array) );
//        break;
        return false;
      case '{':
//        state->push_back( ParserState(state::dict_start) );
//        break;
        return false;
      case ']':
        return false;
      case '}':
        //Its an empty dictionary so we don't need to update the dict_elements count.
        count->dicts += 1;
        state->pop_back();
        break;
      case '"':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::dict_read_kv_sep;
        state->push_back( ParserState(state::start_string) );
        break;
      case ' ':
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_count_dict_kv_sep(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      case ' ':
        break;
      case ':':
        state->back().state = state::dict_read_value;
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_count_dict_value(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      case '[':
        state->back().state = state::dict_read_sep;
        state->back().subelements++;
        state->push_back( ParserState(state::start_array) );
        break;
        return false;
      case '{':
        state->back().state = state::dict_read_sep;
        state->back().subelements++;
        state->push_back( ParserState(state::dict_start) );
        break;
        return false;
      case ']':
        return false;
      case '"':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::dict_read_sep;
        state->back().subelements++;
        state->push_back( ParserState(state::start_string) );
        break;
      case ' ':
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_count_dict_sep(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      case ' ':
        break;
      case ',':  // We git another KV pair coming in
        state->back().state = state::dict_read_key;
        break;
      case '}':
        count->dicts += 1;
        count->dict_elements += state->back().subelements;
        state->pop_back();
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_count_dict_key(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    switch( *cursor )
    {
      //For now disallow arrays and dictionaries as keys
      case '[':
//        state->push_back( ParserState(state::start_array) );
//        break;
        return false;
      case '{':
//        state->push_back( ParserState(state::dict_start) );
//        break;
        return false;
      case ']':
        return false;
      case '"':
        //Transition the state for when we complete the sub-state, then move into a sub state.
        state->back().state = state::dict_read_kv_sep;
        state->push_back( ParserState(state::start_string) );
        break;
      case ' ':
        break;
      default:
        return false;
    }
    return true;
  }

  bool parse_json_counts( const char * start, const char * end, JsonElementCount * count )
  {
    std::vector<ParserState> state_stack;
    state_stack.push_back( ParserState(state::start_root) );

    if(!count) return false;
    if(start==end) return false;

    const char * cursor = start;
    while( cursor != end )
    {
      switch( state_stack.back().state )
      {
        case state::start_root:
          if( ! parse_json_count_root( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::start_string:
          if( ! parse_json_count_string( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::start_array :
          if( ! parse_json_count_array( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::continue_array :
          if( ! parse_json_count_array_continue( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::require_array_element :
          if( ! parse_json_count_require_element( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::dict_start:
          if( ! parse_json_count_dict( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::dict_read_kv_sep:
          if( ! parse_json_count_dict_kv_sep( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::dict_read_sep:
          if( ! parse_json_count_dict_sep( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::dict_read_key:
          if( ! parse_json_count_dict_key( cursor, end, count, &state_stack ) ) return false;
          break;
        case state::dict_read_value:
          if( ! parse_json_count_dict_value( cursor, end, count, &state_stack ) ) return false;
          break;
        default:
          return false;
      }

      ++cursor;
    }

    return state_stack.back().state == state::start_root;
  }

  void print_state_stack( const std::vector<ParserState> & v )
  {
    std::cout<<"=== STATE STACK [ "<<v.size()<<" ] ==="<<std::endl;
    for(unsigned int i=0; i<v.size(); ++i)
    {
      std::cout<<v[i].state<<" : "<<std::endl;
    }
    std::cout<<"========================="<<std::endl;
  }

}
