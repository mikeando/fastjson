#ifndef FASTJSON_DOM_H
#define FASTJSON_DOM_H

#include "fastjson/core.h"

//TODO: Ugly that we need this.. can we remove it later?
#include <vector>
#include <sstream>

namespace fastjson { namespace dom {


    //Requires the DATA type to have a next pointer to DATA
    //We hijack that pointer for use in the free list.
    template<typename DATA>
    class Pager
    {
      public:
        Pager() : free_ptr(NULL), data(), available_count_(0)
      {
      }

        uint32_t n_available() const { return available_count_; }

        void create_and_add_page(uint32_t n)
        {
          DATA * new_page = new DATA[n];
          for(unsigned int i=0; i<n; ++i)
          {
            new_page[i].next = free_ptr;
            free_ptr = new_page + i;
          }
          data.push_back(new_page);
          available_count_+=n;
        }

        DATA * create_value()
        {
          if(available_count_==0)
          {
            //TODO: This should not be hardcoded to 10...
            create_and_add_page(10);
          }

          available_count_--;
          DATA * value = free_ptr;
          free_ptr = value->next;
          value->next = NULL;
          return value;
        }

        void destroy( DATA * a )
        {
          available_count_++;
          a->next = free_ptr;
          free_ptr = a;
        }

      protected:
        DATA * free_ptr;
        std::vector< DATA* > data;
        uint32_t available_count_;
    };

    struct StringBuffer
    {
      public:
        explicit StringBuffer(unsigned int N )
        {
          buf_ = new char[N];
          size_ = N;
          inuse_ = 0;
        }

        unsigned int available() const { return size_ - inuse_; }
        char * write( const char * s, unsigned int len )
        {
          char * start = buf_ + inuse_;
          memcpy( start, s, len );
          inuse_ += len;
          return start;
        }

      protected:
        char * buf_;
        unsigned int size_;
        unsigned int inuse_;
    };

    class Dictionary;
    class Array;

    // The idea behind a document is that it can maintain free lists of the unused elements.
    // It can allocate more memory as required, 
    // It can be merged into another document (merging the free lists correctly).
    class Chunk
    {
      public:
        Chunk() : arrays_(), dicts_(), strings_() {}

        Token create_value_token_from_string( const std::string & v )
        {
          Token tok;
          tok.type=Token::ValueToken;
          tok.data.value.type_hint=ValueType::StringHint;
          unsigned int space_required = v.size();
          tok.data.value.size = space_required;
          tok.data.value.ptr = create_raw_buffer( v.c_str(), space_required );
          return tok;
        }

        Token create_value_token_from_int( int v )
        {
          Token tok;
          tok.type=Token::ValueToken;
          tok.data.value.type_hint=ValueType::NumberHint;
          std::stringstream ss;
          ss<<v;
          std::string s = ss.str();
          unsigned int space_required = s.size();
          tok.data.value.size = space_required;
          tok.data.value.ptr = create_raw_buffer( s.c_str(), space_required );
          return tok;
        }

        Token create_value_token_from_float( float v )
        {
          Token tok;
          tok.type=Token::ValueToken;
          tok.data.value.type_hint=ValueType::NumberHint;
          std::stringstream ss;
          ss<<v;
          std::string s = ss.str();
          unsigned int space_required = s.size();
          tok.data.value.size = space_required;
          tok.data.value.ptr = create_raw_buffer( s.c_str(), space_required );
          return tok;
        }

        Token create_value_token_from_double( double v )
        {
          Token tok;
          tok.type=Token::ValueToken;
          tok.data.value.type_hint=ValueType::NumberHint;
          std::stringstream ss;
          ss<<v;
          std::string s = ss.str();
          unsigned int space_required = s.size();
          tok.data.value.size = space_required;
          tok.data.value.ptr = create_raw_buffer( s.c_str(), space_required );
          return tok;
        }

        Token create_value_token_from_uint64_t( uint64_t v )
        {
          Token tok;
          tok.type=Token::ValueToken;
          tok.data.value.type_hint=ValueType::NumberHint;
          std::stringstream ss;
          ss<<v;
          std::string s = ss.str();
          unsigned int space_required = s.size();
          tok.data.value.size = space_required;
          tok.data.value.ptr = create_raw_buffer( s.c_str(), space_required );
          return tok;
        }

        char * create_raw_buffer( const char * b, unsigned int space_required )
        {
          unsigned int i=0;
          for(i=0; i<strings_.size(); ++i)
          {
            if ( strings_[i].available() >= space_required )
            {
              return strings_[i].write( b, space_required );
            }
          }
          //Should use a minimum for this so that we get less fragmentation?
          strings_.push_back( StringBuffer( space_required ) );
          return strings_.back().write( b, space_required );
        }

        Pager<ArrayEntry>& arrays() { return arrays_; }
        Pager<DictEntry>& dicts() { return dicts_; }
        std::vector<StringBuffer>& strings() { return strings_; }

      protected:
        Pager<ArrayEntry> arrays_;
        Pager<DictEntry>  dicts_;
        std::vector<StringBuffer>  strings_;
    };

    template<typename T>
    struct json_helper;

    template<>
    struct json_helper<std::string>
    {
      static bool build( Token * tok, Chunk * chunk, const std::string & value )
      {
        *tok = chunk->create_value_token_from_string(value);
        return true; 
      }
    };

    template<>
    struct json_helper<int>
    {
      static bool build( Token * tok, Chunk * chunk, int value )
      {
        *tok = chunk->create_value_token_from_int(value);
        return true; 
      }
    };

    template<>
    struct json_helper<uint64_t>
    {
      static bool build( Token * tok, Chunk * chunk, uint64_t value )
      {
        *tok = chunk->create_value_token_from_uint64_t(value);
        return true; 
      }
    };

    template<>
    struct json_helper<float>
    {
      static bool build( Token * tok, Chunk * chunk, float value )
      {
        *tok = chunk->create_value_token_from_float(value);
        return true; 
      }
    };

    template<>
    struct json_helper<double>
    {
      static bool build( Token * tok, Chunk * chunk, double value )
      {
        *tok = chunk->create_value_token_from_double(value);
        return true; 
      }
    };

    template<>
    struct json_helper<bool>
    {
      static bool build( Token * tok, Chunk * chunk, bool value )
      {
        if(value)
        {
          tok->type=Token::LiteralTrueToken;
        }
        else
        {
          tok->type=Token::LiteralFalseToken;
        }
        return true; 
      }
    };

    class Dictionary
    {
      public:
        static Dictionary as_dict( Token * tok, Chunk * chunk )
        {
          Dictionary retval;
          assert(tok->type==Token::DictToken);
          DictEntry * d = tok->data.dict.ptr;
          DictEntry * end = NULL;
          //Get the real end...
          while(d)
          {
            end = d;
            d = d->next;
          }

          retval.tok_   = tok;
          retval.chunk_ = chunk;
          retval.end_   = end;
        }

        static Dictionary create_dict( Token * tok, Chunk * chunk )
        {
          //Assumes that whatever was there has been cleaned up
          Dictionary retval;
          tok->type = Token::DictToken;
          tok->data.dict.ptr = NULL;
          retval.tok_   = tok;
          retval.chunk_ = chunk;
          retval.end_   = NULL;
          return retval;
        }


        template<typename T, typename W>
        bool add( const std::string & key, const W & value )
        {
          DictEntry * dv = chunk_->dicts().create_value();
          dv->next = NULL;
          dv->key_tok = chunk_->create_value_token_from_string(key);
          if( ! json_helper<T>::build( &dv->value_tok, chunk_, value ) )
          {
            return false;
          }

          if( end_ )
          {
            end_->next = dv;
          }
          else
          {
            tok_->data.dict.ptr = dv;
          }
          end_ = dv;
          return true;
        }

        const Token * token() const
        {
          return tok_;
        }

        protected:
        Dictionary() : tok_(NULL), chunk_(NULL), end_(NULL)
        {
        }

        Token * tok_;
        Chunk  * chunk_;
        DictEntry * end_;
    };



    class Array
    {
      public:
        static Array as_array( Token * tok, Chunk * chunk )
        {
          Array retval;
          assert(tok->type==Token::ArrayToken);
          ArrayEntry * a = tok->data.array.ptr;
          ArrayEntry * end = NULL;
          //Get the real end...
          while(a)
          {
            end = a;
            a = a->next;
          }

          retval.tok_   = tok;
          retval.chunk_ = chunk;
          retval.end_   = end;
        }

        static Array create_array( Token * tok, Chunk * chunk )
        {
          //Assumes that whatever was there has been cleaned up
          Array retval;
          tok->type = Token::ArrayToken;
          tok->data.array.ptr = NULL;
          retval.tok_   = tok;
          retval.chunk_ = chunk;
          retval.end_   = NULL;
          return retval;
        }

        template<typename T, typename W>
        bool add( const W & value )
        {
          //Get the spot for the token...
          ArrayEntry * array_entry = chunk_->arrays().create_value();
          array_entry->next = NULL;

          if( ! json_helper<T>::build( &array_entry->tok, chunk_, value ) )
          {
            chunk_->arrays().destroy( array_entry );
            return false;
          }

          //Hook it int the array
          if( end_ )
          {
            end_->next = array_entry;
          }
          else
          {
            tok_->data.array.ptr = array_entry;
          }
          end_ = array_entry;

          return true;
        }

        const fastjson::Token * token() const
        {
          return tok_;
        }

      private:
        Array() : tok_(NULL), chunk_(NULL), end_(NULL)
        {
        }

        Token      * tok_;
        Chunk      * chunk_;
        ArrayEntry * end_;
    };




    template< typename T >
    struct json_helper< std::vector<T> >
    {
      static bool build( Token * tok, Chunk * chunk, const std::vector<T> & v )
      {
        Array a = Array::create_array( tok, chunk );
        for( unsigned int i=0; i<v.size(); ++i)
        {
          a.add<T>( v[i] );
        }
        return true;
      }
    };

  }
}

namespace fastjson { namespace util {
 void set_string( Token * tok, dom::Chunk * chunk, const char * s );
} }

#endif
