#include "fastjson/core.h"
#include "fastjson/dom.h"
#include "fastjson.h"

namespace fastjson { namespace dom {

  bool parse_string( const std::string & s, Token * tok, Chunk * chunk, std::string * error_message )
  {
    fastjson::JsonElementCount count;

    // TODO: fastjson::count_elements should return an error message!
    if( ! fastjson::count_elements( s, &count ) ) return false;

    // Allocate up enough space.
    fastjson::Document doc;
    if( count.n_array_elements() > 0 )
    {
      doc.array_store = new fastjson::ArrayEntry[ count.n_array_elements() ];
    }
    else
    {
      doc.array_store = NULL;
    }

    if( count.n_dict_elements() > 0 )
    {
      doc.dict_store = new fastjson::DictEntry[ count.n_dict_elements() ];
    }
    else
    {
      doc.dict_store = NULL;
    }

    if( count.n_string_length() > 0 )
    {
      doc.string_store = new unsigned char[ count.n_string_length() ];
    }
    else
    {
      doc.string_store = NULL;
    }

    //Actually read the stuff into the arrays etc.
    //TODO: parse_doc should return an error message!

    if( ! fastjson::parse_doc( s, &doc ) )
    {
      delete [] doc.array_store;
      delete [] doc.dict_store;
      delete [] doc.string_store;
      return false;
    }

    *tok = doc.root;

    //Add the data to the chunk.

    if( count.n_array_elements()>0)
    {
      chunk->add_array_page( doc.array_store, count.n_array_elements() );
    }
    if( count.n_dict_elements()>0)
    {
      chunk->add_dict_page ( doc.dict_store, count.n_dict_elements() );
    }
    if( count.n_string_length()>0)
    {
      chunk->add_string_page( (char*)doc.string_store, count.n_string_length() );
    }

    return true;
  }

  void clone_token( const Token * in_token, Token * out_token, Chunk * chunk )
  {
    switch( in_token->type )
    {
      case Token::ArrayToken:
        {
          Array a = Array::create_array( out_token, chunk );
          //Add all the children.
          ArrayEntry * child_orig = in_token->data.array.ptr;
          while( child_orig )
          {
            ArrayEntry * child_copy = a.add_child_raw();
            clone_token( &child_orig->tok, &child_copy->tok, chunk );
            child_orig = child_orig->next;
          }
        }
        break;
      case Token::DictToken:
        {
          Dictionary d = Dictionary::create_dict( out_token, chunk );
          //Add all the children.
          DictEntry * child_orig = in_token->data.dict.ptr;
          while( child_orig )
          {
            DictEntry * child_copy = d.add_child_raw();
            clone_token( &child_orig->key_tok, &child_copy->key_tok, chunk );
            clone_token( &child_orig->value_tok, &child_copy->value_tok, chunk );
            child_orig = child_orig->next;
          }
        }
        break;
      case Token::ValueToken:
        {
          out_token->type = Token::ValueToken;
          out_token->data.value.size = in_token->data.value.size;
          out_token->data.value.type_hint = in_token->data.value.type_hint;
          if( in_token->data.value.ptr == NULL )
          {
            out_token->data.value.ptr = NULL;
          }
          else
          {
            out_token->data.value.ptr = chunk->create_raw_buffer( in_token->data.value.ptr, in_token->data.value.size );
          }
        }
        break;
      case Token::LiteralTrueToken:
        out_token->type=Token::LiteralTrueToken;
        break;
      case Token::LiteralFalseToken:
        out_token->type=Token::LiteralFalseToken;
        break;
      case Token::LiteralNullToken:
        out_token->type=Token::LiteralNullToken;
        break;
    }
  }


} }

