#include "fastjson.h"
#include "parser_codes.h"
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

bool ishex( unsigned char c )
{
  if(c>=128) return false;
  return hex_char_traits[c] & 0x80;
}

uint8_t hexdigit( unsigned char c )
{
  if(c>=128) return false;
  return hex_char_traits[c] & 0x0F;
}

bool isdigit( unsigned char c )
{
  if(c>=128) return false;
  return ascii_class[c]==C_DIGIT;
}

//Moves pointer as far as it can while reading a valid number.
//If it fails to parse any characters it returns start
const unsigned char * parse_number( const unsigned char * start, const unsigned char * end )
{
  const unsigned char * cursor = start;
  //Eat the optional sign
  if( cursor==end ) return start;
  if( *cursor=='-' ) { ++cursor; }


  //Eat the integer mantissa
  if( cursor==end ) return start;
  if( *cursor=='0' ) { ++cursor;}
  else if( isdigit(*cursor) )
  {
    do
    {
      ++cursor;
    }
    while( cursor!=end && isdigit(*cursor)  );
  }
  else
  {
    //Its not valid ..
    return start;
  }
  if( cursor==end ) return cursor;

  //Eat the optional fractional mantissa
  if(*cursor=='.')
  {
    ++cursor;
    while( cursor!=end && isdigit(*cursor) )
    {
      ++cursor;
    }
  }
  if( cursor==end ) return cursor;

  //Now eat any exponent part.
  const unsigned char * back_up = cursor; //This part can fail with us having a valid number .. so lets be prepared for that
  if( *cursor=='e' || *cursor=='E' )
  {
    ++cursor;
    if(cursor==end) { return back_up; }

    //Eat the optional sign
    if( *cursor=='+' || *cursor=='-' )
    {
      ++cursor;
    }

    //We require at least one digit in the mantissa
    if( cursor==end || !isdigit(*cursor) ) { return back_up; }
    while( cursor!=end && isdigit(*cursor) )
    {
      ++cursor;
    }
  }

  return cursor;
}

namespace fastjson
{
  bool parse_json_counts( const std::string & json_str, JsonElementCount * count )
  {
    return parse_json_counts( reinterpret_cast<const unsigned char*>(json_str.c_str()), reinterpret_cast<const unsigned char*>(json_str.c_str()) + json_str.size(), count );
  }

  namespace state
  {
    static const int start_root            = 0;

    static const int start_array           = 1;
    static const int continue_array        = 2;
    static const int require_array_element = 3;

    static const int dict_start            = 4;
    static const int dict_read_value       = 5;
    static const int dict_continue         = 6;

    static const int start_string          = 7;

    static const int start_number          = 8;
  }

  struct ParserState
  {
    explicit ParserState( int in_state ) : state(in_state ), subelements(0) {}
    int state;
    uint32_t subelements;
  };

  const unsigned char * eat_whitespace( const unsigned char * start, const unsigned char * end )
  {
    const unsigned char * cursor = start;
    while( cursor!=end && ( ascii_class[*cursor]==C_SPACE || ascii_class[*cursor]==C_WHITE ) )
    {
      ++cursor;
    }
    return cursor;
  }

  int parse_start_object(
    const unsigned char * start,
    const unsigned char * end,
    JsonElementCount * count,
    std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if( cursor==end ) return -1;
    int white_count = cursor-start;

    switch (ascii_class[*cursor])
    {
      case C_LSQRB:
        state->push_back( ParserState(state::start_array) );
        return white_count + 1;
      case C_LCURB:
        state->push_back( ParserState(state::dict_start) );
        return white_count + 1;
      case C_QUOTE:
        state->push_back( ParserState(state::start_string) );
        return white_count + 1;
      case C_DIGIT:
      case C_MINUS:
        state->push_back( ParserState(state::start_number) );
        return white_count + 0;
      default:
        return -1;
        ;
    }
  }


  int parse_json_count_root(
    const unsigned char * cursor,
    const unsigned char * end,
    JsonElementCount * count,
    std::vector<ParserState> * state )
  {
    return parse_start_object(cursor,end,count,state);
  }

  //returns how many characters we've consumed
  int parse_json_count_string(
      const unsigned char * cursor,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    //Lets be greedy and eat up all the characters we can.
    const unsigned char * newcursor=cursor;
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

  //returns how many characters we've consumed
  int parse_json_count_number(
      const unsigned char * cursor,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    const unsigned char * np = parse_number( cursor, end );
    if(np==cursor) return -1;

    count->strings += 1;
    count->total_string_length += (np-cursor);
    state->pop_back();

    return np-cursor;
  }

  //This is only called at the start of an array
  int parse_json_count_array(
      const unsigned char * start,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end) return -1;

    //Have we reached the end of the array?
    if( *cursor==']' )
    {
        //NOTE: The array contains no elements so we don't need to increment array_elements
        // If the array were to have sub elements we'd hit the "]" in state::continue_array instead.
        count->arrays += 1;
        state->pop_back();
        return cursor-start+1;
    }

    //If not we need to read a real element
    state->back().state = state::continue_array;
    state->back().subelements +=1;
    int dp =  parse_start_object( cursor, end, count, state );
    return (dp<0)?dp:((cursor-start)+dp);
  }

  int parse_json_count_array_continue(
      const unsigned char * start,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end) return -1;

    //Have we reached the end of the array?
    if( *cursor==']' )
    {
        count->arrays += 1;
        count->array_elements += state->back().subelements;
        state->pop_back();
        return cursor-start+1;
    }

    //Do we have another element coming?
    if( *cursor==',' )
    {
      ++cursor;
      state->back().state = state::continue_array;
      state->back().subelements +=1;
      int dp =  parse_start_object( cursor, end, count, state );
      return (dp<0)?dp:((cursor-start)+dp);
    }

    // We got something unexpected..
    return -1;
  }


  int parse_json_count_dict(
      const unsigned char * start,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end) return -1;

    //Have we reached the end of the dictionary?
    if( *cursor=='}' )
    {
        count->dicts += 1;
        state->pop_back();
        return (cursor-start)+1;
    }

    //Nope.. we'd better be getting a string then
    if( *cursor=='"' )
    {
      //Transition the state for when we complete the sub-state, then move into a sub state.
      state->back().state = state::dict_read_value;
      state->push_back( ParserState(state::start_string) );
      return (cursor-start)+1;
    }

    //Otherwise something bad is happenening
    return -1;
  }

  int parse_json_count_dict_value(
      const unsigned char * start,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end) return -1;
    if(*cursor!=':') return -1;
    ++cursor;

    //Now we should get an object...
    state->back().state = state::dict_continue;
    state->back().subelements++;
    int dp =  parse_start_object( cursor, end, count, state );
    return (dp<0)?dp:((cursor-start)+dp);
  }

  int parse_json_count_dict_continue(
      const unsigned char * start,
      const unsigned char * end,
      JsonElementCount * count,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end) return -1;

    //Have we reached the end of the dictionary?
    if( *cursor=='}' )
    {
        count->dict_elements += state->back().subelements;
        count->dicts += 1;
        state->pop_back();
        return (cursor-start)+1;
    }

    //Nope.. we'd better be getting a comma then a string then
    if( *cursor!=',') return -1;
    ++cursor;
    cursor = eat_whitespace(cursor,end);

    if(cursor==end) return -1;
    if( *cursor!='"' ) return -1;

    //Transition the state for when we complete the sub-state, then move into a sub state.
    state->back().state = state::dict_read_value;
    state->push_back( ParserState(state::start_string) );
    return (cursor-start)+1;
  }

  bool parse_json_counts( const unsigned char * start, const unsigned char * end, JsonElementCount * count )
  {
    std::vector<ParserState> state_stack;
    state_stack.push_back( ParserState(state::start_root) );

    if(!count) return false;
    if(start==end) return false;

    const unsigned char * cursor = start;
    while( cursor != end )
    {
      int dp=0;
      switch( state_stack.back().state )
      {
        case state::start_root:
          dp = parse_json_count_root( cursor, end, count, &state_stack );
          break;
        case state::start_string:
          dp = parse_json_count_string( cursor, end, count, &state_stack );
          break;
        case state::start_number:
          dp = parse_json_count_number( cursor, end, count, &state_stack );
          break;
        case state::start_array :
          dp = parse_json_count_array( cursor, end, count, &state_stack );
          break;
        case state::continue_array :
          dp = parse_json_count_array_continue( cursor, end, count, &state_stack );
          break;
        case state::dict_start:
          dp = parse_json_count_dict( cursor, end, count, &state_stack );
          break;
        case state::dict_read_value:
          dp = parse_json_count_dict_value( cursor, end, count, &state_stack );
          break;
        case state::dict_continue:
          dp =  parse_json_count_dict_continue( cursor, end, count, &state_stack );
          break;
        default:
          return false;
      }

      if(dp<0) return false;
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
