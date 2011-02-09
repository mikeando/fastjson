#include "fastjson.h"
#include <vector>
#include <iostream>

#define C_NOT_HEX 0
#define C_HEX(X) (0x80 + (X))

static uint8_t hex_char_traits[128] =
{
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,

  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_HEX(0),  C_HEX(1),  C_HEX(2),  C_HEX(3),  C_HEX(4) , C_HEX(5),  C_HEX(6),  C_HEX(7),
  C_HEX(8),  C_HEX(9),  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,

  C_NOT_HEX, C_HEX(10), C_HEX(11), C_HEX(12), C_HEX(13), C_HEX(14), C_HEX(15), C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,

  C_NOT_HEX, C_HEX(10), C_HEX(11), C_HEX(12), C_HEX(13), C_HEX(14), C_HEX(15), C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX,
  C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX, C_NOT_HEX
};

bool ishex( char c )
{
  unsigned char uc = static_cast<unsigned char>(c);
  if(uc>=128) return false;
  return hex_char_traits[uc] & 0x80;
}

uint8_t hexdigit( char c )
{
  unsigned char uc = static_cast<unsigned char>(c);
  return hex_char_traits[uc] & 0x0F;
}

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

  //returns how many characters we've consumed
  int parse_json_count_string(
      const char * cursor,
      const char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    //Lets be greedy and eat up all the characters we can.
    const char * newcursor=cursor;
    while(newcursor!=end)
    {
      switch(*newcursor)
      {
        case '"':
          //Are we ending the string?
          count->strings += 1;
          state->pop_back();
          return (newcursor-cursor)+1;
        case '\\':
          //We've got an escaped character..
          ++newcursor;
          if(newcursor==end) return -1;
          //What kind of escape is it?
          switch(*newcursor)
          {
            case 'u': //Unicode escape.
              ++newcursor;
              //We need 4 hex digits after the u
              if(newcursor+4 >= end) return -1;
              if( ! ( ishex( newcursor[0] ) && ishex( newcursor[1] ) && ishex( newcursor[2] ) && ishex( newcursor[3] ) ) )
                return -1;
              uint32_t v = ( hexdigit(newcursor[0]) << 12) | ( hexdigit(newcursor[1])<<8 ) | hexdigit(newcursor[2])<<4 | hexdigit(newcursor[3] );
              if( v<0x0080 )
              {
                count->total_string_length++;
              }
              else if ( v<0x0800 )
              {
                count->total_string_length+=2;
              }
              else if ( v<=0xFFFF )
              {
                count->total_string_length+=3;
              }
              newcursor+=4;
              break;
            default:
              //TODO: We should check that its a valid escape character
              ++newcursor;
              count->total_string_length++;
          }
          break;
        default:
          count->total_string_length++;
          ++newcursor;
      }
    }
    return -1;
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
      int dp=0;
      switch( state_stack.back().state )
      {
        case state::start_root:
          if( ! parse_json_count_root( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::start_string:
          dp = parse_json_count_string( cursor, end, count, &state_stack );
          if (dp<=0) return false;
          break;
        case state::start_array :
          if( ! parse_json_count_array( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::continue_array :
          if( ! parse_json_count_array_continue( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::require_array_element :
          if( ! parse_json_count_require_element( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::dict_start:
          if( ! parse_json_count_dict( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::dict_read_kv_sep:
          if( ! parse_json_count_dict_kv_sep( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::dict_read_sep:
          if( ! parse_json_count_dict_sep( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::dict_read_key:
          if( ! parse_json_count_dict_key( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        case state::dict_read_value:
          if( ! parse_json_count_dict_value( cursor, end, count, &state_stack ) ) return false;
          dp=1;
          break;
        default:
          return false;
      }

      cursor+=dp;
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
