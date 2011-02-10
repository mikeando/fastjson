#include "fastjson.h"
#include <vector>
#include <iostream>

#define C_MISC      0
#define C_HEX(X)   (0x80 + (X))
#define C_WHITE    0x40
#define C_DIGIT(X) (0x80 + 0x20 + (X))

static uint8_t fj_char_traits[128] =
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
  C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC,     C_MISC
};

bool ishex( unsigned char c )
{
  if(c>=128) return false;
  return fj_char_traits[c] & 0x80;
}

uint8_t hexdigit( unsigned char c )
{
  if(c>=128) return false;
  return fj_char_traits[c] & 0x0F;
}

bool isdigit( unsigned char c )
{
  if(c>=128) return false;
  return fj_char_traits[c] & 0x20;
}

bool iswhite( unsigned char c )
{
  if( c>=128) return false;
  return fj_char_traits[c] & 0x40;
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
    if( cursor==end ) return NULL;

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
  const unsigned char * parse_string(
      const unsigned char * cursor,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * newcursor=cursor;
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
          if(newcursor==end) return NULL;
          //What kind of escape is it?
          switch(*newcursor)
          {
            case 'u': //Unicode escape.
              ++newcursor;
              //We need 4 hex digits after the u
              if(newcursor+4 >= end) return NULL;
              if( ! ( ishex( newcursor[0] ) && ishex( newcursor[1] ) && ishex( newcursor[2] ) && ishex( newcursor[3] ) ) )
                return NULL;
              uint32_t v = ( hexdigit(newcursor[0]) << 12) | ( hexdigit(newcursor[1])<<8 ) | hexdigit(newcursor[2])<<4 | hexdigit(newcursor[3] );
              if( v<0x0080 )
              {
                callback->string_add_ubyte( 0 );
              }
              else if ( v<0x0800 )
              {
                callback->string_add_ubyte( 0 );
                callback->string_add_ubyte( 0 );
              }
              else if ( v<=0xFFFF )
              {
                callback->string_add_ubyte( 0 );
                callback->string_add_ubyte( 0 );
                callback->string_add_ubyte( 0 );
              }
              newcursor+=4;
              break;
            default:
              //TODO: We should check that its a valid escape character
              ++newcursor;
              callback->string_add_ubyte( 0 );
          }
          break;
        default:
          callback->string_add_ubyte( *newcursor );
          ++newcursor;
      }
    }
    return NULL;
  }

  template<typename T>
  const unsigned char * parse_number(
      const unsigned char * start,
      const unsigned char * end,
      T * callback,
      std::vector<ParserState> * state )
  {
    const unsigned char * cursor = ::parse_number( start, end );
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
    if(cursor==end) return NULL;

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
    if(cursor==end) return NULL;

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
    if(cursor==end) return NULL;

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
    if(cursor==end) return NULL;
    if(*cursor!=':') return NULL;
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
    if(cursor==end) return NULL;

    //Have we reached the end of the dictionary?
    if( *cursor=='}' )
    {
        callback->end_dict();
        state->pop_back();
        return cursor+1;
    }

    //Nope.. we'd better be getting a comma then a string then
    if( *cursor!=',') return NULL;
    ++cursor;
    cursor = eat_whitespace(cursor,end);

    if(cursor==end) return NULL;
    if( *cursor!='"' ) return NULL;

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
          return false;
      }

      if(cursor==NULL) return false;
    }

    return state_stack.back().state == state::start_root;
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
      XParser() { context.push_back( Context::root() ); }

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
