/*
 * Copyright (c) 2011, Run With Robots
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the fastjson library nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RUN WITH ROBOTS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MICHAEL ANDERSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fastjson.h"
#include <vector>
#include <iostream>

#define C_MISC      0
#define C_HEX(X)   (0x80 + (X))
#define C_WHITE    0x40
#define C_DIGIT(X) (0x80 + 0x20 + (X))

static const uint8_t fj_char_traits[256] =
{
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_WHITE,    C_WHITE,    C_MISC,     C_MISC,     C_WHITE,    C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_WHITE,    C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_DIGIT(0), C_DIGIT(1), C_DIGIT(2), C_DIGIT(3), C_DIGIT(4), C_DIGIT(5), C_DIGIT(6), C_DIGIT(7),
  C_DIGIT(8), C_DIGIT(9), C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_MISC,     C_HEX(10),  C_HEX(11),  C_HEX(12),  C_HEX(13),  C_HEX(14),  C_HEX(15),  C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_MISC,     C_HEX(10),  C_HEX(11),  C_HEX(12),  C_HEX(13),  C_HEX(14),  C_HEX(15),  C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,

  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC
};

static inline bool ishex( unsigned char c )
{
  return fj_char_traits[c] & 0x80;
}

static inline uint8_t hexdigit( unsigned char c )
{
  return fj_char_traits[c] & 0x0F;
}

static inline bool isdigit( unsigned char c )
{
  return fj_char_traits[c] & 0x20;
}

static inline bool iswhite( unsigned char c )
{
  return fj_char_traits[c] & 0x40;
}

//Moves pointer as far as it can while reading a valid number.
//If it fails to parse any characters it returns start
template<typename PARSER>
const unsigned char * parse_number( const unsigned char * start, const unsigned char * end, PARSER * p )
{
  const unsigned char * cursor = start;
  //Eat the optional sign
  if( cursor==end ) { p->on_error(1000, "end of input during number",start,cursor,end); return start; }
  if( *cursor=='-' ) { ++cursor; }


  //Eat the integer mantissa
  if( cursor==end ) { p->on_error(1000, "end of input during number",start,cursor,end); return start; }
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
    p->on_error(1001, "invalid number",start,cursor,end);
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

  static inline const unsigned char * eat_whitespace( const unsigned char * start, const unsigned char * end )
  {
    const unsigned char * cursor = start;
    while( cursor!=end && iswhite(*cursor) )
    {
      ++cursor;
    }
    return cursor;
  }

  // Returns the pointer to the body of the next object and changes to the
  // appropriate parsing state.
  // returns NULL if it fails
  template<typename T>
  const unsigned char * parse_start_object(
    const unsigned char * start,
    const unsigned char * end,
    T * callback,
    std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if( cursor==end )
    {
      callback->on_error(2000, "Expected object, got end of input instead",start,cursor,end);
      return NULL;
    }

    switch (*cursor)
    {
      case '[':
        callback->start_array();
        state->push_back( ParserState(state::start_array) );
        return cursor + 1;
      case '{':
        callback->start_dict();
        state->push_back( ParserState(state::dict_start) );
        return cursor + 1;
      case '"':
        callback->start_string();
        state->push_back( ParserState(state::start_string) );
        return cursor + 1;
      case 't':
        if( end - cursor  < 4 || cursor[1]!='r' || cursor[2]!='u' || cursor[3]!='e' )
        {
          callback->on_error(2001, "Invalid litteral found when expecting object", start, cursor, end);
          return NULL;
        }
        callback->on_true();
        return cursor + 4;
      case 'f':
        if( end - cursor  < 5 || cursor[1]!='a' || cursor[2]!='l' || cursor[3]!='s' || cursor[4]!='e' )
        {
          callback->on_error(2001, "Invalid litteral found when expecting object", start, cursor, end);
          return NULL;
        }
        callback->on_false();
        return cursor + 5;
      case 'n':
        if( end - cursor  < 4 || cursor[1]!='u' || cursor[2]!='l' || cursor[3]!='l' )
        {
          callback->on_error(2001, "Invalid litteral found when expecting object", start, cursor, end);
          return NULL;
        }
        callback->on_null();
        return cursor + 4;
      default:
        ;
    }
    //Maybe its a number?
    if( isdigit(*cursor) || *cursor=='-' )
    {
        callback->start_number();
        state->push_back( ParserState(state::start_number) );
        return cursor;
    }

    callback->on_error(2002, "Invalid data found", start, cursor, end);
    return NULL;
  }


  template<typename T>
  const unsigned char * parse_root(
    const unsigned char * start,
    const unsigned char * end,
    T * callback,
    std::vector<ParserState> * state )
  {
    return parse_start_object(start,end,callback,state);
  }

  template<typename T>
  const unsigned char * read_unicode_escape( const unsigned char * start, const unsigned char * end, uint32_t * val, T * callback )
  {
     //We need at least 6 characters \uxxxx
     if( end-start < 6    ) { callback->on_error(3000,"Insufficient data for unicode escape",start,start,end); return NULL; }
     if( start[0] != '\\' ) { callback->on_error(3001,"Unicode escape must start \\u",start,start,end);        return NULL; }
     if( start[1] != 'u'  ) { callback->on_error(3001,"Unicode escape must start \\u",start,start,end);        return NULL; }
     if( ! ( ishex( start[2] ) && ishex( start[3] ) && ishex( start[4] ) && ishex( start[5] ) ) )
     {
       callback->on_error(3002,"Nonhex character found in unicode escape",start,start,end);
       return NULL;
     }
     *val = ( hexdigit(start[2]) << 12) | ( hexdigit(start[3])<<8 ) | hexdigit(start[4])<<4 | hexdigit(start[5] );
    return start+6;
  }

  template<typename T>
  const unsigned char * parse_string(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * newcursor=start;
    while(newcursor!=end)
    {
      switch(*newcursor)
      {
        case '"':
          //Are we ending the string?
          callback->end_string();
          state->pop_back();
          return newcursor+1;
        case '\\':
          //We've got an escaped character..
          ++newcursor;
          if(newcursor==end)
          {
            callback->on_error(3003, "End of input while parsing escaped character", start, newcursor, end);
            return NULL;
          }
          //What kind of escape is it?
          switch(*newcursor)
          {
            case 'u': //Unicode escape.
              uint32_t v;
              newcursor = read_unicode_escape( newcursor-1, end, &v, callback );
              if( ! newcursor ) return NULL;

              if( v<0x0080 )
              {
                callback->string_add_ubyte( v );
              }
              else if ( v<0x0800 )
              {
                // v = 00000yyy yyxxxxxx
                // two bytes 110yyyyy 10xxxxxx
                callback->string_add_ubyte( 0xC0 | ( ( v >> 6) & 0x1F) );
                callback->string_add_ubyte( 0x80 | ( ( v >> 0) & 0x3F) );
              }
              else if ( v<=0xFFFF )
              {
              //Handling surrogate pairs is tricky
/*
UTF-16 converts these into two 16-bit code units, called a surrogate pair, by
the following scheme: 0x10000 is subtracted from the code point, leaving a 20
bit number in the range 0..0xFFFFF.  The top ten bits (a number in the range
0..0x3FF) are added to 0xD800 to give the first code unit or high surrogate,
which will be in the range 0xD800..0xDBFF.  The low ten bits (also in the range
0..0x3FF) are added to 0xDC00 to give the second code unit or low surrogate,
which will be in the range 0xDC00..0xDFFF.
*/
                /* Are we dealing with the first codepoint of a surrogate pair */
                if( v>=0xD800 && v<=0xDBFF )
                {
                  uint32_t nextv;
                  newcursor = read_unicode_escape( newcursor, end, & nextv, callback );
                  if( ! newcursor ) return NULL;
                  if( nextv>=0xDC00 && nextv<=0xDFFF )
                  {
                    uint32_t vlow = v-0xD800;
                    uint32_t vhigh = nextv-0xDC00;
                    uint32_t unicode_code_value = ( vhigh << 10 ) | vlow;

                    callback->string_add_ubyte( 0xF0 | ( (unicode_code_value >> 18) & 0x07) );
                    callback->string_add_ubyte( 0x80 | ( (unicode_code_value >> 12) & 0x3F) );
                    callback->string_add_ubyte( 0x80 | ( (unicode_code_value >> 6 ) & 0x3F) );
                    callback->string_add_ubyte( 0x80 | ( (unicode_code_value >> 0 ) & 0x3F) );
                  }
                  else
                  {
                    callback->on_error(3004, "Invalid second surrogate pair found when decoing unicode escape", start, newcursor, end);
                    return NULL;
                  }
                }
                /* We should _never_ get a second code point of a surrogate pair here */
                else if( v>=0xDC00 && v<=0xDFFF )
                {
                  callback->on_error(3005, "Invalid second surrogate pair without first surrogate pair when decoding unicode escape", start, newcursor, end);
                  return NULL;
                }
                else
                {
                  // v = zzzzyyyy yyxxxxxx
                  // three bytes 1110zzzz 10yyyyyy 10xxxxxx
                  callback->string_add_ubyte( 0xE0 | ( (v >> 12) & 0x0F) );
                  callback->string_add_ubyte( 0x80 | ( (v >> 6 ) & 0x3F) );
                  callback->string_add_ubyte( 0x80 | ( (v >> 0 ) & 0x3F) );
                }
              }
              break;
            case '"':
              callback->string_add_ubyte( '"' );
              ++newcursor;
              break;
            case '\\':
              callback->string_add_ubyte( '\\' );
              ++newcursor;
              break;
            case '/':
              callback->string_add_ubyte( '/' );
              ++newcursor;
              break;
            case 'b':
              callback->string_add_ubyte( 0x08 );
              ++newcursor;
              break;
            case 'f':
              callback->string_add_ubyte( 0x0C );
              ++newcursor;
              break;
            case 'n':
              callback->string_add_ubyte( 0x0A );
              ++newcursor;
              break;
            case 'r':
              callback->string_add_ubyte( 0x0D );
              ++newcursor;
              break;
            case 't':
              callback->string_add_ubyte( 0x09 );
              ++newcursor;
              break;
            default:
              callback->on_error(3006, "Invalid escape", start, newcursor, end );
              return NULL;
          }
          break;
        default:
          callback->string_add_ubyte( *newcursor );
          ++newcursor;
      }
    }
    callback->on_error( -1000, "Internal error parsing string - should never get here", start,newcursor,end );
    return NULL;
  }

  template<typename T>
  const unsigned char * parse_number(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = ::parse_number( start, end, callback );
    if(cursor==start) return NULL;

    callback->end_number(start,cursor);
    state->pop_back();

    return cursor;
  }

  //This is only called at the start of an array
  template<typename T>
  const unsigned char * parse_array(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end)
    {
      callback->on_error(4001,"End of input while parsing start of array",start,cursor,end);
      return NULL;
    }

    //Have we reached the end of the array?
    if( *cursor==']' )
    {
        //NOTE: The array contains no elements so we don't need to increment array_elements
        // If the array were to have sub elements we'd hit the "]" in state::continue_array instead.
        callback->end_array();
        state->pop_back();
        return cursor+1;
    }

    //If not we need to read a real element
    state->back().state = state::continue_array;
    state->back().subelements +=1;
    return parse_start_object( cursor, end, callback, state );
  }

  template<typename T>
  const unsigned char * parse_array_continue(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end)
    {
      callback->on_error(4002,"End of input while parsing array",start,cursor,end);
      return NULL;
    }

    //Have we reached the end of the array?
    if( *cursor==']' )
    {
        callback->end_array();
        state->pop_back();
        return cursor+1;
    }

    //Do we have another element coming?
    if( *cursor==',' )
    {
      ++cursor;
      state->back().state = state::continue_array;
      state->back().subelements +=1;
      cursor =  parse_start_object( cursor, end, callback, state );
      if(cursor==NULL) return NULL;
      return cursor;
    }

    // We got something unexpected..
    callback->on_error(4003, "Unexpected character while parsing array", start, cursor, end);
    return NULL;
  }


  template<typename T>
  const unsigned char * parse_dict(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end)
    {
      callback->on_error(5001, "End of input while parsing dict start", start, cursor, end);
      return NULL;
    }

    //Have we reached the end of the dictionary?
    if( *cursor=='}' )
    {
        callback->end_dict();
        state->pop_back();
        return cursor+1;
    }

    //Nope.. we'd better be getting a string then
    if( *cursor=='"' )
    {
      callback->start_string();
      //Transition the state for when we complete the sub-state, then move into a sub state.
      state->back().state = state::dict_read_value;
      state->push_back( ParserState(state::start_string) );
      return cursor+1;
    }

    //Otherwise something bad is happenening
    callback->on_error(5002, "Unexpected character while parsing dict start",start,cursor,end);
    return NULL;
  }

  template<typename T>
  const unsigned char * parse_dict_value(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end)
    {
      callback->on_error(5003, "Unexpected end of input while lookig for dict seperator",start,cursor,end);
      return NULL;
    }
    if(*cursor!=':')
    {
      callback->on_error(5004, "Unexpected character while looking for dict seperator",start,cursor,end);
      return NULL;
    }
    ++cursor;

    //Now we should get an object...
    state->back().state = state::dict_continue;
    state->back().subelements++;
    return  parse_start_object( cursor, end, callback, state );
  }

  template<typename T>
  const unsigned char * parse_dict_continue(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = eat_whitespace(start,end);
    if(cursor==end)
    {
      callback->on_error(5005, "Unexpected end of input while reading dict",start,cursor,end);
      return NULL;
    }

    //Have we reached the end of the dictionary?
    if( *cursor=='}' )
    {
        callback->end_dict();
        state->pop_back();
        return cursor+1;
    }

    //Nope.. we'd better be getting a comma then a string then
    if( *cursor!=',')
    {
      callback->on_error(5006, "Unexpected character when looking for comma in dict",start,cursor,end);
      return NULL;
    }
    ++cursor;
    cursor = eat_whitespace(cursor,end);

    if(cursor==end)
    {
      callback->on_error(5007, "Unexpected end of input when looking for dict key",start,cursor,end);
      return NULL;
    }

    if( *cursor!='"' )
    {
      callback->on_error(5008, "Unexpected character when looking for dict key",start,cursor,end);
      return NULL;
    }

    callback->start_string();
    //Transition the state for when we complete the sub-state, then move into a sub state.
    state->back().state = state::dict_read_value;
    state->push_back( ParserState(state::start_string) );
    return cursor+1;
  }

  template<typename T>
  bool parse( const unsigned char * start, const unsigned char * end, T * callback )
  {
    std::vector<ParserState> state_stack;
    state_stack.push_back( ParserState(state::start_root) );

    if(!callback) return false;
    if(start==end) return false;

    const unsigned char * cursor = start;
    while( cursor != end )
    {
      if( state_stack.back().state != state::start_string )
      {
        cursor = eat_whitespace(cursor, end);
        if(cursor==end) break;
      }
      switch( state_stack.back().state )
      {
        case state::start_root:
          cursor = parse_root( cursor, end, callback, &state_stack );
          break;
        case state::start_string:
          cursor = parse_string( cursor, end, callback, &state_stack );
          break;
        case state::start_number:
          cursor = parse_number( cursor, end, callback, &state_stack );
          break;
        case state::start_array :
          cursor = parse_array( cursor, end, callback, &state_stack );
          break;
        case state::continue_array :
          cursor = parse_array_continue( cursor, end, callback, &state_stack );
          break;
        case state::dict_start:
          cursor = parse_dict( cursor, end, callback, &state_stack );
          break;
        case state::dict_read_value:
          cursor = parse_dict_value( cursor, end, callback, &state_stack );
          break;
        case state::dict_continue:
          cursor =  parse_dict_continue( cursor, end, callback, &state_stack );
          break;
        default:
          callback->on_error(-1002,"Inavlid state encountered", start, cursor,end);
          return false;
      }

      if(cursor==NULL) return false;
    }

    if(state_stack.back().state != state::start_root)
    {
      callback->on_error(6001, "Input ended while in non-root state",start,end,end);
      return false;
    }
    return true;
  }

  bool count_elements( const std::string & json_str, JsonElementCount * count )
  {
    return count_elements( reinterpret_cast<const unsigned char*>(json_str.c_str()), reinterpret_cast<const unsigned char*>(json_str.c_str()) + json_str.size(), count );
  }

  bool count_elements( const unsigned char * start, const unsigned char * end, JsonElementCount * count )
  {
    return parse<JsonElementCount>(start,end,count);
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

  struct XParser
  {
      XParser()
      {
        context.reserve(5);
        context.push_back( Context::root() );
      }

      Token * add_child(Token::Type type)
      {
        Context & ctxt = context.back();

        switch( ctxt.type )
        {
          case Root:
            doc->root.type = type;
            cur_tok = &doc->root;
            return cur_tok;
          case InArray:
            //For the first entry we wont have a last_entry set yet.
            if( ! ctxt.c.a.last_entry )
            {
              ctxt.c.a.array->ptr = array_ptr;
            }
            else
            {
              ctxt.c.a.last_entry->next = array_ptr;
            }

            array_ptr->tok.type = type;
            array_ptr->next = NULL;
            cur_tok = &(array_ptr->tok);
            //We're just gonna grab an array entry off our store and use that.
            ctxt.c.a.last_entry = array_ptr;
            ++array_ptr;
            return cur_tok;
          case InDict:
            // We need to behave slightly differently depending on whether we're adding
            // The first of a KV pair, or the second.
            // The very first key entry wont have a last_entry
            if( ctxt.c.d.expecting_key )
            {
              if( ! ctxt.c.d.last_entry )
              {
                ctxt.c.d.dict->ptr = dict_ptr;
              }
              else
              {
                ctxt.c.d.last_entry->next = dict_ptr;
              }

              dict_ptr->key_tok.type = type;
              dict_ptr->next = NULL;
              cur_tok = &(dict_ptr->key_tok);
              ctxt.c.d.last_entry = dict_ptr;
              ++dict_ptr;
              ctxt.c.d.expecting_key = false;
              return cur_tok;
            }
            else
            {
              //Next pointer should have been set when the key was setup, so we dont need to do that now.
              cur_tok = &(ctxt.c.d.last_entry->value_tok);
              cur_tok->type = type;
              ctxt.c.d.expecting_key = true;
              return cur_tok;
            }
        }
        //NOTE: Should never get here
        return NULL;
      }

      void start_array()
      {
        Token * t = add_child(Token::ArrayToken);
        t->data.array.ptr = NULL;
        context.push_back( Context::array( &(t->data.array) ) );
      }

      void start_dict()
      {
        Token * t = add_child(Token::DictToken);
        t->data.dict.ptr = NULL;
        context.push_back( Context::dict( &(t->data.dict) ) );
      }

      void start_string()
      {
        /* Token * t = */ add_child(Token::ValueToken);
        string_start = string_ptr;
      };

      void on_true()
      {
        add_child( Token::LiteralTrueToken );
      };

      void on_false()
      {
        add_child( Token::LiteralFalseToken );
      };

      void on_null()
      {
        add_child( Token::LiteralNullToken );
      };


      void start_number()
      {
        /* Token * t = */ add_child(Token::ValueToken);
        string_start = string_ptr;
      };

      void end_array()
      {
        context.pop_back();
      }

      void end_dict()
      {
        context.pop_back();
      }

      void end_string()
      {
        Token * tok = get_current_token();

        tok->type=fastjson::Token::ValueToken;
        tok->data.value.type_hint=fastjson::ValueType::StringHint;
        tok->data.value.ptr = reinterpret_cast<char*>(string_start);
        tok->data.value.size = string_ptr - string_start;
      }

      void string_add_ubyte( const unsigned char uc )
      {
        *string_ptr = uc;
        ++string_ptr;
      }

      void end_number( const unsigned char * start, const unsigned char * end )
      {
        while(start<end)
        {
          *string_ptr = *start;
          ++start;
          ++string_ptr;
        }

        Token * tok = get_current_token();

        tok->type=fastjson::Token::ValueToken;
        tok->data.value.type_hint=fastjson::ValueType::NumberHint;
        tok->data.value.ptr = reinterpret_cast<char*>(string_start);
        tok->data.value.size = string_ptr - string_start;
      };

      Document * doc;

      //This gets updated whenever we start a new string
      unsigned char * string_start;
      unsigned char * string_ptr;

      ArrayEntry * array_ptr;
      DictEntry  * dict_ptr;

      Token * cur_tok;

      Token * get_current_token()
      {
        return cur_tok;
      }

      void on_error( int errcode, const std::string & mesg, const unsigned char * start_context, const unsigned char * locn, const unsigned char * end_context )
      {
        std::cerr<<"OMG error ["<<errcode<<"] "<<mesg<<std::endl;
      }



    protected:
      bool first_in_array;

      enum ContextType { Root, InArray, InDict };
      struct RootContext {};
      struct ArrayContext { ArrayType * array; ArrayEntry * last_entry; };
      struct DictContext  { DictType * dict; DictEntry * last_entry; bool expecting_key; };
      struct Context
      {
        static Context array( ArrayType * at )
        {
          Context retval;
          retval.type=InArray;
          retval.c.a.array = at;
          retval.c.a.last_entry = NULL;
          return retval;
        }

        static Context dict( DictType * dt )
        {
          Context retval;
          retval.type=InDict;
          retval.c.d.dict = dt;
          retval.c.d.last_entry = NULL;
          retval.c.d.expecting_key = true;
          return retval;
        }

        static Context root()
        {
          Context retval;
          retval.type=Root;
          return retval;
        }

        ContextType type;
        union
        {
          RootContext  r;
          ArrayContext a;
          DictContext  d;
        } c;
      };

      std::vector<Context> context;
  };

  bool parse_doc( const std::string & json_str, Document * doc )
  {
    return parse_doc( reinterpret_cast<const unsigned char*>(json_str.c_str()), reinterpret_cast<const unsigned char*>(json_str.c_str()) + json_str.size(), doc );
  }

  bool parse_doc( const unsigned char * start, const unsigned char * end, Document * doc )
  {
      //This assumes we have enough space...
      XParser p;
      p.doc = doc;
      p.string_ptr = doc->string_store;
      p.array_ptr = doc->array_store;
      p.dict_ptr = doc->dict_store;
      return parse<XParser>( start,end, &p);
  }



}
