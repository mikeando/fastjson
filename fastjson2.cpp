#include "fastjson2.h"

namespace fastjson
{
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

  size_t serialize_inplace( const Token * tok, char * buffer )
  {
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
                memcpy(buffer+1, tok->data.value.ptr, tok->data.value.size);
                buffer[tok->data.value.size+1]='"';
                return tok->data.value.size+2;
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
