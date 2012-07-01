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

#ifndef FASTJSON_DOM_H
#define FASTJSON_DOM_H

#include "fastjson/core.h"
#include "fastjson/utils.h"
#include "fastjson/error.h"

//TODO: Ugly that we need this.. can we remove it later?
#include <vector>
#include <sstream>
#include <string.h>

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

        ~Pager()
        {
          for(unsigned int i=0; i<data.size(); ++i)
          {
            delete [] data[i];
          }
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

        //We dont really use N - but if we ever track in-use entries too we'll need it
        void add_used_page( DATA * page, uint32_t /* N */ )
        {
          data.push_back(page);
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

    //NOTE: These dont clean up the strings... you need to do that your self.
    // For us the chunk handles that.
    struct StringBuffer
    {
      public:
        explicit StringBuffer(unsigned int N )
        {
          buf_ = new char[N];
          size_ = N;
          inuse_ = 0;
        }

        struct in_use {} ;

        StringBuffer( char * buffer, unsigned int N, in_use )
        {
          buf_ = buffer;
          size_ = N;
          inuse_ = N;
        }

        unsigned int available() const { return size_ - inuse_; }
        char * write( const char * s, unsigned int len )
        {
          char * start = buf_ + inuse_;
          memcpy( start, s, len );
          inuse_ += len;
          return start;
        }

        void destroy() { delete [] buf_; buf_=NULL;}

      protected:
        char * buf_;
        unsigned int size_;
        unsigned int inuse_;
    };


    // The idea behind a document is that it can maintain free lists of the unused elements.
    // It can allocate more memory as required, 
    // It can be merged into another document (merging the free lists correctly).
    class Chunk
    {
      public:
        Chunk() : arrays_(), dicts_(), strings_() {}
        ~Chunk()
        {
          for(unsigned int i=0; i<strings_.size(); ++i)
          {
            strings_[i].destroy();
          }
        }


        //TODO: Shift this into an object containing the StringBuffers?
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


        void add_array_page( ArrayEntry * entries, unsigned int N )
        {
          arrays_.add_used_page(entries, N);
        }

        void add_dict_page( DictEntry * entries, unsigned int N )
        {
          dicts_.add_used_page(entries, N);
        }

        void add_string_page( char * buffer, unsigned int L )
        {
          strings_.push_back( StringBuffer(buffer,L, StringBuffer::in_use() ) );
        }

        Pager<ArrayEntry>& arrays() { return arrays_; }
        Pager<DictEntry>& dicts() { return dicts_; }
        std::vector<StringBuffer>& strings() { return strings_; }

      protected:
        Pager<ArrayEntry> arrays_;
        Pager<DictEntry>  dicts_;
        std::vector<StringBuffer>  strings_;
    };

    class Value
    {
      public:
        static Value as_value( Token * tok, Chunk * chunk )
        {
          Value retval;
          assert(tok->type==Token::ValueToken);

          retval.tok_   = tok;
          retval.chunk_ = chunk;
        }

        static Value create_value( Token * tok, Chunk * chunk )
        {
          //Assumes that whatever was there has been cleaned up
          Value retval;
          tok->type = Token::ValueToken;
          tok->dict.ptr = NULL;
          retval.tok_   = tok;
          retval.chunk_ = chunk;
          return retval;
        }


        template<typename T, typename W>
        bool set_numeric( const W & value )
        {
          tok_->value.type_hint = ValueType::NumberHint;
          std::stringstream ss;
          ss<<value;
          std::string s = ss.str();
          //Now allocate enough space for it.
          tok_->value.ptr = chunk_->create_raw_buffer( s.c_str(), s.size() );
          tok_->value.size = s.size();
          return true;
        }

        template<typename T, typename W>
        bool set_string( const W & value )
        {
          tok_->value.type_hint = ValueType::StringHint;
          std::stringstream ss;
          ss<<value;
          std::string s = ss.str();
          //Now allocate enough space for it.
          tok_->value.ptr = chunk_->create_raw_buffer( s.c_str(), s.size() );
          tok_->value.size = s.size();
          return true;
        }

        void set_raw_string( const std::string & s )
        {
          tok_->value.type_hint = ValueType::StringHint;
          tok_->value.ptr = chunk_->create_raw_buffer( s.c_str(), s.size() );
          tok_->value.size = s.size();
        }

        void set_raw_cstring( const char * cstr )
        {
          tok_->value.type_hint = ValueType::StringHint;
          size_t len = strlen(cstr);
          tok_->value.ptr = chunk_->create_raw_buffer( cstr, len );
          tok_->value.size = len;
        }

        const Token * token() const
        {
          return tok_;
        }

        protected:
        Value() : tok_(NULL), chunk_(NULL)
        {
        }

        Token * tok_;
        Chunk  * chunk_;
    };


    template<typename T>
    struct json_helper;

    template<>
    struct json_helper<std::string>
    {
      static bool build( Token * tok, Chunk * chunk, const std::string & value )
      {
        Value v = Value::create_value(tok,chunk);
        v.set_raw_string( value );
        return true; 
      }
      static bool from_json_value( const Token * tok, std::string * s )
      {
        if(!tok || tok->type!=Token::ValueToken ) return false;
        if(tok->value.ptr) { *s = std::string( tok->value.ptr, tok->value.size); return true; }
        *s = "";
        return true;
      }
    };


    template<typename T>
    struct numeric_value_json_helper
    {
      static bool build( Token * tok, Chunk * chunk, T value )
      {
        Value v = Value::create_value(tok,chunk);
        v.set_numeric<T>(value);
        return true; 
      }

      static bool from_json_value( const Token * tok, T * v )
      {
        if(!tok || tok->type!=Token::ValueToken ) return false;
        if( ! tok->value.ptr ) { *v = 0; }
        return utils::try_convert<T>( tok->value.ptr, tok->value.size, v );
      }
    };

    template<> struct json_helper<int>      : public numeric_value_json_helper<int>      {};
    template<> struct json_helper<uint64_t> : public numeric_value_json_helper<uint64_t> {};
    template<> struct json_helper<int64_t> : public numeric_value_json_helper<int64_t> {};
    template<> struct json_helper<float>    : public numeric_value_json_helper<float>    {};
    template<> struct json_helper<double>   : public numeric_value_json_helper<double>   {};

    template<>
    struct json_helper<bool>
    {
      static bool build( Token * tok, Chunk * /* chunk */, bool value )
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
      static bool from_json_value( const Token * token, bool * v )
      {
        if( ! token ) return false;
        if( ! v ) return false;
        if( token->type == Token::LiteralTrueToken ) { *v = true; return true; }
        if( token->type == Token::LiteralFalseToken ) { *v = false; return true; }
        //Lets be lenient in what we accept .. a 0 is false and a 1 is true (same for "0" and "1").
        if( (token->type == Token::ValueToken) && (token->value.size==1) )
        {
          if( token->value.ptr[0] == '0' ) { *v = false; return true; }
          if( token->value.ptr[0] == '1' ) { *v = true; return true; }
        }

        return false;
      }
    };

    class Dictionary
    {
      public:
        static Dictionary as_dict( Token * tok, Chunk * chunk )
        {
          Dictionary retval;
          assert(tok->type==Token::DictToken);
          DictEntry * d = tok->dict.ptr;
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
          return retval;
        }

        static Dictionary create_dict( Token * tok, Chunk * chunk )
        {
          //Assumes that whatever was there has been cleaned up
          Dictionary retval;
          tok->type = Token::DictToken;
          tok->dict.ptr = NULL;
          retval.tok_   = tok;
          retval.chunk_ = chunk;
          retval.end_   = NULL;
          return retval;
        }


        template<typename T, typename W>
        bool add( const std::string & key, const W & value )
        {
          Token tok;

          if( ! json_helper<T>::build( &tok, chunk_, value ) )
          {
            return false;
          }

          return add_token(key,tok);
        }

        bool add_token( const std::string & key, const Token & tok )
        {
          DictEntry * dv = chunk_->dicts().create_value();
          dv->next = NULL;
          Value key_v = Value::create_value( &dv->key_tok, chunk_ );
          key_v.set_raw_string(key);

          dv->value_tok = tok;

          if( end_ )
          {
            end_->next = dv;
          }
          else
          {
            tok_->dict.ptr = dv;
          }
          end_ = dv;
          return true;
        }

        DictEntry * add_child_raw()
        {
          DictEntry * dv = chunk_->dicts().create_value();
          dv->next = NULL;

          if( end_ )
          {
            end_->next = dv;
          }
          else
          {
            tok_->dict.ptr = dv;
          }
          end_ = dv;
          return dv;
        }

        DictEntry * add_child_raw( const std::string & key )
        {
          DictEntry * dv = chunk_->dicts().create_value();
          Value key_v = Value::create_value( &dv->key_tok, chunk_ );
          key_v.set_raw_string(key);
          dv->next = NULL;

          if( end_ )
          {
            end_->next = dv;
          }
          else
          {
            tok_->dict.ptr = dv;
          }
          end_ = dv;
          return dv;
        }

        template<typename T>
        bool get( const std::string & k, T * value )
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                return json_helper<T>::from_json_value( &child->value_tok, value );
              }
            }
            child = child->next;
          }
          return false;
        }

        template<typename T>
        T get_default( const std::string & k, const T & default_value )
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                T rv;
                if( json_helper<T>::from_json_value( &child->value_tok, &rv ) )
                {
                  return rv;
                }
              }
            }
            child = child->next;
          }
          return default_value;
        }

        bool has_child( const std::string & k ) const
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                return true;
              }
            }
            child = child->next;
          }
          return false;
        } 

        bool get_raw( const std::string & k, Token * token )
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                *token = child->value_tok;
                return true;
              }
            }
            child = child->next;
          }
          return false;
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

    class Dictionary_const
    {
      public:
        static Dictionary_const as_dict( const Token * tok )
        {
          Dictionary_const retval;
          assert(tok->type==Token::DictToken);
          retval.tok_   = tok;
          return retval;
        }

        template<typename T>
        bool get( const std::string & k, T * value )
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                return json_helper<T>::from_json_value( &child->value_tok, value );
              }
            }
            child = child->next;
          }
          return false;
        }

        // This breaks the classes contract as someone could make changes
        // to the structures pointed to by the Token, but its the only way to
        // get a raw token out of these easily.
        bool get_raw( const std::string & k, Token * token )
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                *token = child->value_tok;
                return true;
              }
            }
            child = child->next;
          }
          return false;
        } 

        bool has_child( const std::string & k ) const
        {
          DictEntry * child = tok_->dict.ptr;
          while( child )
          {
            //Is the childs key a string value
            if( child->key_tok.type == Token::ValueToken && child->key_tok.value.type_hint == ValueType::StringHint )
            {
              if( std::string(child->key_tok.value.ptr, child->key_tok.value.size) == k )
              {
                return true;
              }
            }
            child = child->next;
          }
          return false;
        } 

        const Token * token() const
        {
          return tok_;
        }

        protected:
        Dictionary_const() : tok_(NULL)
        {
        }

        const Token * tok_;
    };


    class Array
    {
      public:
        static Array as_array( Token * tok, Chunk * chunk )
        {
          Array retval;
          assert(tok->type==Token::ArrayToken);
          ArrayEntry * a = tok->array.ptr;
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

          return retval;
        }

        static Array create_array( Token * tok, Chunk * chunk )
        {
          //Assumes that whatever was there has been cleaned up
          Array retval;
          tok->type = Token::ArrayToken;
          tok->array.ptr = NULL;
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
            tok_->array.ptr = array_entry;
          }
          end_ = array_entry;

          return true;
        }

        ArrayEntry * add_child_raw()
        {
          ArrayEntry * array_entry = chunk_->arrays().create_value();
          array_entry->next = NULL;

          //Hook it int the array
          if( end_ )
          {
            end_->next = array_entry;
          }
          else
          {
            tok_->array.ptr = array_entry;
          }
          end_ = array_entry;

          return array_entry;
        }

        const fastjson::Token * token() const
        {
          return tok_;
        }

        size_t length() const
        {
          fastjson::ArrayEntry * a = tok_->array.ptr;
          size_t retval = 0;

          //Get the real end...
          while(a)
          {
            ++retval;
            a = a->next;
          }
          return retval;
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
      static bool from_json_value( const Token * tok, std::vector<T> * data )
      {
        if(!data) return false;
        if(!tok || tok->type!=Token::ArrayToken ) return false;
        std::vector<T> retval;
        ArrayEntry * child = tok->array.ptr;
        while(child)
        {
          retval.push_back(T());
          if(! json_helper<T>::from_json_value( &child->tok, &retval.back() ) ) return false;
          child = child->next;
        }
        *data = retval;
        return true;
      }
    };

    bool parse_string( const std::string & s, Token * tok, Chunk * chunk, unsigned int parse_mode, UserErrorCallback user_error_callback, void * user_data );
    void clone_token( const Token * in_token, Token * out_token, Chunk * chunk );

  }
}

namespace fastjson { namespace util {
 void set_string( Token * tok, dom::Chunk * chunk, const char * s );
} }

#endif
