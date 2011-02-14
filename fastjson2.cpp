#include "fastjson2.h"

namespace fastjson
{
  enum DecodeTypes {
  FJ_NOR, // : Doesn't need anything done to it
  FJ_RUC, // : Requires unicode
  FJ_UCC, // : Unicode continuation character
  FJ_UC2, // : Unicode 2 byte start character
  FJ_UC3, // : Unicode 3 byte start character
  FJ_UC4, // : Unicode 4 byte start character
  FJ_ESC, // : Has nice (required) escape
  FJ_ERR, // : Has nice (required) escape
  };

 static const DecodeTypes decode_types[256]
        = { /* 0       1       2       3         4       5       6      7          8       9       A       B         C       D       E       F */
 /* 0 */    FJ_RUC, FJ_RUC, FJ_RUC, FJ_RUC,   FJ_RUC, FJ_RUC, FJ_RUC, FJ_RUC,   FJ_ESC, FJ_ESC, FJ_ESC, FJ_RUC,   FJ_ESC, FJ_ESC, FJ_RUC, FJ_RUC,
 /* 1 */    FJ_RUC, FJ_RUC, FJ_RUC, FJ_RUC,   FJ_RUC, FJ_RUC, FJ_RUC, FJ_RUC,   FJ_RUC, FJ_RUC, FJ_RUC, FJ_RUC,   FJ_RUC, FJ_RUC, FJ_RUC, FJ_RUC,
 /* 2 */    FJ_NOR, FJ_NOR, FJ_ESC, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,
 /* 3 */    FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,

 /* 4 */    FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,
 /* 5 */    FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_ESC, FJ_NOR, FJ_NOR, FJ_NOR,
 /* 6 */    FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,
 /* 7 */    FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_NOR,   FJ_NOR, FJ_NOR, FJ_NOR, FJ_RUC,

 /* 8 */    FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,
 /* 9 */    FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,
 /* A */    FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,
 /* B */    FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,   FJ_UCC, FJ_UCC, FJ_UCC, FJ_UCC,

 /* C */    FJ_ERR, FJ_ERR, FJ_UC2, FJ_UC2,   FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,   FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,   FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,
 /* D */    FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,   FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,   FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,   FJ_UC2, FJ_UC2, FJ_UC2, FJ_UC2,
 /* E */    FJ_UC3, FJ_UC3, FJ_UC3, FJ_UC3,   FJ_UC3, FJ_UC3, FJ_UC3, FJ_UC3,   FJ_UC3, FJ_UC3, FJ_UC3, FJ_UC3,   FJ_UC3, FJ_UC3, FJ_UC3, FJ_UC3,
 /* F */    FJ_UC4, FJ_UC4, FJ_UC4, FJ_UC4,   FJ_UC4, FJ_ERR, FJ_ERR, FJ_ERR,   FJ_ERR, FJ_ERR, FJ_ERR, FJ_ERR,   FJ_ERR, FJ_ERR, FJ_ERR, FJ_ERR,
          };

  unsigned char hex_digit[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E','F' };


  struct Writer
  {
    char * buffer;
    Writer * write( char c )
    {
      *buffer = c;
      ++buffer;
      return this;
    }
  };

  struct Counter
  {
    uint32_t count;
    Counter * write( char c )
    {
      count++;
      return this;
    }
  };

  template<typename CALLBACK>
  void process_invalid_unicode_marker( CALLBACK * w, bool escape_unicode_errors )
  {
    if( escape_unicode_errors )
    {
      w->write('\\');
      w->write('u');
      w->write('F');
      w->write('F');
      w->write('F');
      w->write('D');
    }
    else
    {
      w->write(0xFF);
      w->write(0xFD);
    }
  }

  template<typename CALLBACK>
  void process_inplace( const Token * tok, CALLBACK * w );


  template<typename CALLBACK>
  void process_array( const ArrayEntry * array_ptr, CALLBACK * w )
  {
    if( array_ptr==NULL )
    {
      w->write('[')
       ->write(']');
      return;
    }
    else
    {
      w->write('[');

      //Loop through the elements and add them in.
      const ArrayEntry * child = array_ptr;
      while(child)
      {
        process_inplace( &child->tok, w );
        if( child->next)
        {
          w->write(',');
        }
        child=child->next;
      }
      w->write(']');
      return;
    }
  }

  template<typename CALLBACK>
  void process_dict( const DictEntry * dict_ptr, CALLBACK * w)
  {
    if( dict_ptr==NULL )
    {
      w->write('{');
      w->write('}');
      return;
    }
    else
    {
      w->write('{');
      //Loop through the elements and add them in.
      const DictEntry * child = dict_ptr;
      while(child)
      {
        process_inplace( &child->key_tok, w );
        w->write(':');
        process_inplace( &child->value_tok, w );
        if( child->next)
        {
          w->write(',');
        }
        child=child->next;
      }
      w->write('}');
      return;
    }
  }

  template<typename CALLBACK>
  void process_value( const Token * tok, CALLBACK * w)
  {
    bool escape_unicode_errors = true;
    bool escape_unicode = true;
    switch( tok->data.value.type_hint)
    {
      case ValueType::StringHint:
        {
          if (tok->data.value.ptr==NULL)
          {
            w->write('"');
            w->write('"');
            return;
          }
          else
          {
            //TODO: we need to json escape this string...
            w->write('"');
            unsigned char * start = reinterpret_cast<unsigned char*>( tok->data.value.ptr );
            unsigned char * end = start + tok->data.value.size;
            while( start != end )
            {
              switch( decode_types[*start] )
              {
                case FJ_NOR: // : Doesn't need anything done to it
                  w->write( *start );
                  ++start;
                  break;
                case FJ_RUC: // : 1 byte requiring unicode encoding
                  w->write('\\');
                  w->write('u');
                  w->write('0');
                  w->write('0');
                  w->write(hex_digit[ *start >> 4 ]);
                  w->write(hex_digit[ *start & 0x0F ]);
                  ++start;
                  break;
                case FJ_UCC: // : Unicode continuation character
                  //SHOULD _NEVER_ GET THIS IN A VALID UTF-8 STREAM
                  process_invalid_unicode_marker(w, escape_unicode_errors);
                  ++start;
                  break;
                case FJ_UC2: // : Unicode 2 byte start character
                  if( end-start < 1 )
                  { //PUT IN THE INVALID UNICODE CHARACTER
                    process_invalid_unicode_marker(w, escape_unicode_errors);
                    ++start;
                    break;
                  }

                  //Is the second byte a valid continuation character?
                  if( decode_types[ *(start+1) ] != FJ_UCC )
                  {
                    process_invalid_unicode_marker(w, escape_unicode_errors);
                    ++start;
                    break;
                  }

                  //Decode the unicode..
                  if( escape_unicode )
                  {
                    uint32_t u = ( *start & 0x1F) << 6;
                    u |= ( (*(start+1)) & 0x3F );

                    w->write('\\');
                    w->write('u');
                    w->write(hex_digit[ (u >> 12) & 0x0F]);
                    w->write(hex_digit[ (u >> 8 ) & 0x0F]);
                    w->write(hex_digit[ (u >> 4 ) & 0x0F]);
                    w->write(hex_digit[ (u >> 0 ) & 0x0F]);
                    start+=2;
                  }
                  else
                  {
                    w->write(*start);
                    w->write(*(start+1));
                    start+=2;
                  }
                  break;
                case FJ_UC3: // : Unicode 2 byte start character
                  if( end-start < 2 )
                  { //PUT IN THE INVALID UNICODE CHARACTER
                    process_invalid_unicode_marker(w, escape_unicode_errors);
                    ++start;
                    break;
                  }

                  //Are the second and third bytes valid continuation characters?
                  if(
                      decode_types[ *(start+1) ] != FJ_UCC ||
                      decode_types[ *(start+2) ] != FJ_UCC
                    )
                  {
                    process_invalid_unicode_marker(w, escape_unicode_errors);
                    ++start;
                    break;
                  }

                  //Decode the unicode..
                  if( escape_unicode )
                  {
                    uint32_t u = (*start & 0x0F) << 12;
                    u |= ( (*(start+1)) & 0x3F ) << 6;
                    u |= ( (*(start+2)) & 0x3F ) << 0;
                    w->write('\\');
                    w->write('u');
                    w->write(hex_digit[ (u >> 12) & 0x0F]);
                    w->write(hex_digit[ (u >> 8 ) & 0x0F]);
                    w->write(hex_digit[ (u >> 4 ) & 0x0F]);
                    w->write(hex_digit[ (u >> 0 ) & 0x0F]);
                    start+=3;
                  }
                  else
                  {
                    w->write(*start);
                    w->write(*(start+1));
                    w->write(*(start+2));
                    start+=3;
                  }
                  break;

                case FJ_UC4: // : Unicode 4 byte start character
                  if( end-start < 3 )
                  { //PUT IN THE INVALID UNICODE CHARACTER
                    process_invalid_unicode_marker(w, escape_unicode_errors);
                    ++start;
                    break;
                  }

                  //Are the second, third and fourth bytes valid continuation characters?
                  if(
                      decode_types[ *(start+1) ] != FJ_UCC ||
                      decode_types[ *(start+2) ] != FJ_UCC ||
                      decode_types[ *(start+3) ] != FJ_UCC
                    )
                  {
                    process_invalid_unicode_marker(w, escape_unicode_errors);
                    ++start;
                    break;
                  }

                  //Decode the unicode.. we need a surrogate pair :(
                  if( escape_unicode )
                  {
                    unsigned long uccp = (*start & 0x07) << 18;
                    uccp |= ( static_cast<unsigned char>(*(start+1)) & 0x3F ) << 12;
                    uccp |= ( static_cast<unsigned char>(*(start+2)) & 0x3F ) << 6;
                    uccp |= ( static_cast<unsigned char>(*(start+3)) & 0x3F ) << 0;

                    // Now convert into surrogate pair
                    unsigned long uccp_mod = uccp - 0x10000; // offset from the BMP
                    unsigned long surrogate_high = 0xD800 + ( uccp_mod >> 10 );
                    unsigned long surrogate_low  = 0xDC00 + ( uccp_mod & 0x3FF );

                    w->write('\\');
                    w->write('u');
                    w->write(hex_digit[ (surrogate_high >> 12) & 0x0F]);
                    w->write(hex_digit[ (surrogate_high >> 8 ) & 0x0F]);
                    w->write(hex_digit[ (surrogate_high >> 4 ) & 0x0F]);
                    w->write(hex_digit[ (surrogate_high >> 0 ) & 0x0F]);
                    w->write('\\');
                    w->write('u');
                    w->write(hex_digit[ (surrogate_low >> 12) & 0x0F]);
                    w->write(hex_digit[ (surrogate_low >> 8 ) & 0x0F]);
                    w->write(hex_digit[ (surrogate_low >> 4 ) & 0x0F]);
                    w->write(hex_digit[ (surrogate_low >> 0 ) & 0x0F]);
                    start+=4;
                  }
                  else
                  {
                    w->write(*start);
                    w->write(*(start+1));
                    w->write(*(start+2));
                    w->write(*(start+3));
                    start+=4;
                  }
                  break;
                case FJ_ESC: // : Has nice (required) escape
                  w->write('\\');
                  w->write('?');
                  start+=1;
                  break;
                case FJ_ERR: // : Should not occur in a UTF-8 stream
                  process_invalid_unicode_marker(w, escape_unicode_errors);
                  ++start;
                  break;
              }
            }
            //Return something here?
            w->write('"');
            return;
          }
        }
        break;
      case ValueType::NumberHint:
        {
          if (tok->data.value.ptr==NULL)
          {
            w->write('0');
            return;
          }
          else
          {
            for(unsigned int i=0; i<tok->data.value.size; ++i)
              w->write( tok->data.value.ptr[i] );
            return;
          }
        }
        break;
    }
  }

  template<typename CALLBACK>
  void process_inplace( const Token * tok, CALLBACK * w )
  {

    switch( tok->type )
    {
      case Token::ArrayToken:
        return process_array(tok->data.array.ptr,w);
      case Token::DictToken:
        return process_dict(tok->data.dict.ptr,w);
      case Token::ValueToken:
        return process_value(tok,w);
      case Token::LiteralTrueToken:
        w->write('t'); w->write('r'); w->write('u'); w->write('e'); 
        return;
      case Token::LiteralFalseToken:
        w->write('f'); w->write('a'); w->write('l'); w->write('s'); w->write('e'); 
        return;
      case Token::LiteralNullToken:
        w->write('n'); w->write('u'); w->write('l'); w->write('l'); 
        return;
    }
    return;
  }

  size_t serialize_inplace( fastjson::Token const * tok, char * buffer )
  {
    Writer w;
    w.buffer = buffer;
    process_inplace( tok, &w );
    return w.buffer - buffer;
  }
     
  size_t bytes_required( const Token * tok )
  {
    Counter w;
    w.count = 0;
    process_inplace( tok, &w );
    return w.count;
  }
  
}
