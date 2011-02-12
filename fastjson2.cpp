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

 DecodeTypes decode_types[256]
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

  size_t bytes_required( const Token * tok )
  {
    switch( tok->type )
    {
      case Token::ArrayToken:
        if( tok->data.array.ptr==NULL )
        {
          return 2;
        }
        else
        {
          size_t total = 2; //For the opening and closing [] marks
          //Loop through the elements and add them in.
          ArrayEntry * child = tok->data.array.ptr;
          while(child)
          {
            total += bytes_required( &child->tok );
            if( child->next)
              total+=1; //For the ,
            child=child->next;
          }

          return total;
        }
      case Token::DictToken:
        if( tok->data.dict.ptr==NULL )
        {
          return 2;
        }
        else
        {
          size_t total = 2; // For the {}

          //Loop through the elements and add them in.
          DictEntry * child = tok->data.dict.ptr;
          while(child)
          {
            total += bytes_required( &child->key_tok );
            total += 1; // For the :
            total += bytes_required( &child->value_tok );
            if( child->next)
              total +=1 ; // For the ,
            child=child->next;
          }

          return total;
        }
      case Token::ValueToken:
        switch( tok->data.value.type_hint)
        {
          case ValueType::StringHint:
            {
              if (tok->data.value.ptr==NULL)
              {
                return 2;
              }
              else
              {
                //TODO: we need to json escape this string...
                return 2+tok->data.value.size;
              }
            }
            break;
          case ValueType::NumberHint:
            {
              if (tok->data.value.ptr==NULL)
              {
                return 1;
              }
              else
              {
                return tok->data.value.size;
              }
            }
            break;
        }
      case Token::LiteralTrueToken:
        return 4;
      case Token::LiteralFalseToken:
        return 5;
      case Token::LiteralNullToken:
        return 4;
    }
    return 0;
  }

  unsigned char * write_invalid_unicode_marker( unsigned char * buffer, bool escape_unicode_errors )
  {
    if( escape_unicode_errors )
    {
      buffer[0] = '\\';
      buffer[1] = 'u';
      buffer[2] = 'F';
      buffer[3] = 'F';
      buffer[4] = 'F';
      buffer[5] = 'D';
      return buffer+6;
    }
    else
    {
      buffer[0] = 0xFF;
      buffer[1] = 0xFD;
      return buffer+2;
    }
  }


  size_t serialize_inplace( const Token * tok, char * buffer )
  {
    bool escape_unicode_errors = true;
    bool escape_unicode = true;

    switch( tok->type )
    {
      case Token::ArrayToken:
        if( tok->data.array.ptr==NULL )
        {
          buffer[0]='[';
          buffer[1]=']';
          return 2;
        }
        else
        {
          char * cursor = buffer;
          *cursor='[';
          ++cursor;

          //Loop through the elements and add them in.
          ArrayEntry * child = tok->data.array.ptr;
          while(child)
          {
            cursor += serialize_inplace( &child->tok, cursor );
            if( child->next)
            {
              *cursor=',';
              ++cursor;
            }

            child=child->next;
          }
          *cursor=']';
          ++cursor;

          return cursor - buffer;
        }
      case Token::DictToken:
        if( tok->data.dict.ptr==NULL )
        {
          buffer[0]='{';
          buffer[1]='}';
          return 2;
        }
        else
        {
          char * cursor = buffer;
          *cursor='{';
          ++cursor;
          //Loop through the elements and add them in.
          DictEntry * child = tok->data.dict.ptr;
          while(child)
          {
            cursor+=serialize_inplace( &child->key_tok, cursor );
            *cursor=':';
            ++cursor;
            cursor+=serialize_inplace( &child->value_tok, cursor );
            if( child->next)
            {
              *cursor=',';
              ++cursor;
            }
            child=child->next;
          }
          *cursor='}';
          ++cursor;

          return cursor-buffer;
        }
      case Token::ValueToken:
        switch( tok->data.value.type_hint)
        {
          case ValueType::StringHint:
            {
              if (tok->data.value.ptr==NULL)
              {
                buffer[0]='"';
                buffer[1]='"';
                return 2;
              }
              else
              {
                //TODO: we need to json escape this string...
                buffer[0]='"';
                unsigned char * b_ptr = reinterpret_cast<unsigned char*>( buffer+1 );
                unsigned char * start = reinterpret_cast<unsigned char*>( tok->data.value.ptr );
                unsigned char * end = start + tok->data.value.size;
                while( start != end )
                {
                  switch( decode_types[*start] )
                  {
                    case FJ_NOR: // : Doesn't need anything done to it
                      *b_ptr = *start;
                      ++b_ptr;
                      ++start;
                      break;
                    case FJ_RUC: // : 1 byte requiring unicode encoding
                      b_ptr[0] = '\\';
                      b_ptr[1] = 'u';
                      b_ptr[2] = '0';
                      b_ptr[3] = '0';
                      b_ptr[4] = hex_digit[ *start >> 4 ];
                      b_ptr[5] = hex_digit[ *start & 0x0F ];
                      b_ptr+=6;
                      ++start;
                      break;
                    case FJ_UCC: // : Unicode continuation character
                      //SHOULD _NEVER_ GET THIS IN A VALID UTF-8 STREAM
                      b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                      ++start;
                      break;
                    case FJ_UC2: // : Unicode 2 byte start character
                      if( end-start < 1 )
                      { //PUT IN THE INVALID UNICODE CHARACTER
                        b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                        ++start;
                        break;
                      }

                      //Is the second byte a valid continuation character?
                      if( decode_types[ *(start+1) ] != FJ_UCC )
                      {
                        b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                        ++start;
                        break;
                      }

                      //Decode the unicode..
                      if( escape_unicode )
                      {
                        uint32_t u = ( *start & 0x1F) << 6;
                        u |= ( (*(start+1)) & 0x3F );

                        b_ptr[0] = '\\';
                        b_ptr[1] = 'u';
                        b_ptr[2] = hex_digit[ (u >> 12) & 0x0F];
                        b_ptr[3] = hex_digit[ (u >> 8 ) & 0x0F];
                        b_ptr[4] = hex_digit[ (u >> 4 ) & 0x0F];
                        b_ptr[5] = hex_digit[ (u >> 0 ) & 0x0F];
                        b_ptr+=6;
                        start+=2;
                      }
                      else
                      {
                        b_ptr[0] = *start;
                        b_ptr[1] = *(start+1);
                        b_ptr+=2;
                        start+=2;
                      }
                      break;
                    case FJ_UC3: // : Unicode 2 byte start character
                      if( end-start < 2 )
                      { //PUT IN THE INVALID UNICODE CHARACTER
                        b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                        ++start;
                        break;
                      }

                      //Are the second and third bytes valid continuation characters?
                      if(
                          decode_types[ *(start+1) ] != FJ_UCC ||
                          decode_types[ *(start+2) ] != FJ_UCC
                        )
                      {
                        b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                        ++start;
                        break;
                      }

                      //Decode the unicode..
                      if( escape_unicode )
                      {
                        uint32_t u = (*start & 0x0F) << 12;
                        u |= ( (*(start+1)) & 0x3F ) << 6;
                        u |= ( (*(start+2)) & 0x3F ) << 0;
                        b_ptr[0] = '\\';
                        b_ptr[1] = 'u';
                        b_ptr[2] = hex_digit[ (u >> 12) & 0x0F];
                        b_ptr[3] = hex_digit[ (u >> 8 ) & 0x0F];
                        b_ptr[4] = hex_digit[ (u >> 4 ) & 0x0F];
                        b_ptr[5] = hex_digit[ (u >> 0 ) & 0x0F];
                        b_ptr+=6;
                        start+=3;
                      }
                      else
                      {
                        b_ptr[0] = *start;
                        b_ptr[1] = *(start+1);
                        b_ptr[2] = *(start+2);
                        b_ptr+=3;
                        start+=3;
                      }
                      break;

                    case FJ_UC4: // : Unicode 4 byte start character
                      if( end-start < 3 )
                      { //PUT IN THE INVALID UNICODE CHARACTER
                        b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
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
                        b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                        ++start;
                        break;
                      }

                      //Decode the unicode.. we need a surrogate pair :(
                      if( escape_unicode )
                      {
                        b_ptr[0] = '\\';
                        b_ptr[1] = 'u';
                        b_ptr[2] = '?';
                        b_ptr[3] = '?';
                        b_ptr[4] = '?';
                        b_ptr[5] = '?';
                        b_ptr[0] = '\\';
                        b_ptr[1] = 'u';
                        b_ptr[2] = '?';
                        b_ptr[3] = '?';
                        b_ptr[4] = '?';
                        b_ptr[5] = '?';
                        b_ptr+=12;
                        start+=4;
                      }
                      else
                      {
                        b_ptr[0] = *start;
                        b_ptr[1] = *(start+1);
                        b_ptr[2] = *(start+2);
                        b_ptr[3] = *(start+3);
                        b_ptr+=4;
                        start+=4;
                      }
                      break;
                    case FJ_ESC: // : Has nice (required) escape
                      b_ptr[0]='\\';
                      b_ptr[1]='?';
                      b_ptr+=2;
                      start+=1;
                      break;
                    case FJ_ERR: // : Should not occur in a UTF-8 stream
                      b_ptr = write_invalid_unicode_marker(b_ptr, escape_unicode_errors);
                      ++start;
                      break;
                  }
                }
                //Return something here?
                *b_ptr='"';
                ++b_ptr;
                return b_ptr - reinterpret_cast<unsigned char*>(buffer);
              }
            }
            break;
          case ValueType::NumberHint:
            {
              if (tok->data.value.ptr==NULL)
              {
                buffer[0]='0';
                return 1;
              }
              else
              {
                memcpy(buffer,tok->data.value.ptr, tok->data.value.size);
                return tok->data.value.size;
              }
            }
            break;
        }
      case Token::LiteralTrueToken:
        buffer[0]='t';buffer[1]='r';buffer[2]='u';buffer[3]='e';
        return 4;
      case Token::LiteralFalseToken:
        buffer[0]='f';buffer[1]='a';buffer[2]='l';buffer[3]='s';buffer[4]='e';
        return 5;
      case Token::LiteralNullToken:
        buffer[0]='n';buffer[1]='u';buffer[2]='l';buffer[3]='l';
        return 4;
    }
    return 0;
  }
  
}
