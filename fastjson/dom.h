#ifndef FASTJSON_DOM_H
#define FASTJSON_DOM_H

#include "fastjson/core.h"

namespace fastjson { namespace dom {

    template<typename DATA>
    class PageEntry
    {
      public:
        union 
        {
          PageEntry<DATA> * free_ptr;
          DATA d;
        } d;
    };

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
          PageEntry<DATA> * new_page = new PageEntry<DATA>[n];
          for(unsigned int i=0; i<n; ++i)
          {
            new_page[i].d.free_ptr = free_ptr;
            free_ptr = new_page + i;
          }
          data.push_back(new_page);
          available_count_+=n;
        }

        PageEntry<DATA> * create_value()
        {
          if(available_count_==0)
          {
            //TODO: This should not be hardcoded to 10...
            create_and_add_page(10);
          }

          available_count_--;
          PageEntry<DATA> * value = free_ptr;
          free_ptr = value->d.free_ptr;
          return value;
        }

        void destroy( PageEntry<DATA> * a )
        {
          available_count_++;
          a->d.free_ptr = free_ptr;
          free_ptr = a;
        }

      protected:
        PageEntry<DATA> * free_ptr;
        std::vector< PageEntry<DATA>* > data;
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
    class Document
    {
      public:
        Document() : array_pager() {}

        uint32_t n_available_array() const { return array_pager.n_available(); }
        void create_and_add_array_page(uint32_t n)
        {
          return array_pager.create_and_add_page(n);
        }

        PageEntry<ArrayEntry> * create_array_entry()
        {
          return array_pager.create_value();
        }

        void destroy_array( PageEntry<ArrayEntry> * a )
        {
          array_pager.destroy( a );
        }

        uint32_t n_available_dict() const { return dict_pager.n_available(); }
        void create_and_add_dict_page(uint32_t n)
        {
          return dict_pager.create_and_add_page(n);
        }

        PageEntry<DictEntry> * create_dict_entry()
        {
          return dict_pager.create_value();
        }

        void destroy_dict( PageEntry<DictEntry> * a )
        {
          dict_pager.destroy( a );
        }

        Token create_value_token_from_string( const std::string & v )
        {
          Token tok;
          tok.type=Token::ValueToken;
          tok.data.value.type_hint=ValueType::StringHint;
          unsigned int space_required = v.size();
          tok.data.value.size = space_required;
          {
            unsigned int i=0;
            for(i=0; i<strings.size(); ++i)
            {
              if ( strings[i].available() >= space_required )
              {
                tok.data.value.ptr = strings[i].write( v.c_str(), space_required );
                break;
              }
            }
            if( i==strings.size() )
            {
              //Should use a minimum for this so that we get less fragmentation?
              strings.push_back( StringBuffer( space_required ) );
            }
          }
          tok.data.value.ptr = strings.back().write( v.c_str(), space_required );

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
          {
            unsigned int i=0;
            for(i=0; i<strings.size(); ++i)
            {
              if ( strings[i].available() >= space_required )
              {
                tok.data.value.ptr = strings[i].write( s.c_str(), space_required );
                break;
              }
            }
            if( i==strings.size() )
            {
              //Should use a minimum for this so that we get less fragmentation?
              strings.push_back( StringBuffer( space_required ) );
              tok.data.value.ptr = strings.back().write( s.c_str(), space_required );
            }
          }

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
          {
            unsigned int i=0;
            for(i=0; i<strings.size(); ++i)
            {
              if ( strings[i].available() >= space_required )
              {
                tok.data.value.ptr = strings[i].write( s.c_str(), space_required );
                break;
              }
            }
            if( i==strings.size() )
            {
              //Should use a minimum for this so that we get less fragmentation?
              strings.push_back( StringBuffer( space_required ) );
              tok.data.value.ptr = strings.back().write( s.c_str(), space_required );
            }
          }

          return tok;
        }

        Dictionary * create_dictionary();
        Array * create_free_array();
        Token * create_free_token();

        Token root;
      protected:
        Pager<Token>      free_tokens;
        Pager<ArrayEntry> array_pager;
        Pager<DictEntry>  dict_pager;
        std::vector<StringBuffer>  strings;
    };

    template<typename T>
    struct json_helper;

    template<>
    struct json_helper<std::string>
    {
      static bool build( Token * tok, Document * doc, const std::string & value )
      {
        *tok = doc->create_value_token_from_string(value);
        return true; 
      }
    };

    template<>
    struct json_helper<int>
    {
      static bool build( Token * tok, Document * doc, int value )
      {
        *tok = doc->create_value_token_from_int(value);
        return true; 
      }
    };

    template<>
    struct json_helper<float>
    {
      static bool build( Token * tok, Document * doc, float value )
      {
        *tok = doc->create_value_token_from_float(value);
        return true; 
      }
    };

    class Dictionary
    {
      public:
        Dictionary(Document * doc, Token * tok ) : tok_(tok), doc_(doc), end_(NULL)
        {
        }

        template<typename T, typename W>
        bool add( const std::string & key, const W & value )
        {
          PageEntry<DictEntry> * dv = doc_->create_dict_entry();
          dv->d.d.next = NULL;
          dv->d.d.key_tok = doc_->create_value_token_from_string(key);
          if( ! json_helper<T>::build( &dv->d.d.value_tok, doc_, value ) )
          {
            return false;
          }

          if( end_ )
          {
            end_->d.d.next = &(dv->d.d);
          }
          else
          {
            tok_->data.dict.ptr = &(dv->d.d);
          }
          end_ = dv;
          return true;
        }

        const fastjson::Token * token() const
        {
          return tok_;
        }

        fastjson::Token * tok_;
        Document * doc_;
        PageEntry<DictEntry> * end_;
    };

    Dictionary * Document::create_dictionary()
    {
      PageEntry<Token> * tok = free_tokens.create_value();
      tok->d.d.type = Token::DictToken;
      tok->d.d.data.dict.ptr = NULL;
      return new Dictionary( this, &tok->d.d );
    }
  



    class Array
    {
      public:
        Array(Document * doc, Token * tok ) : tok_(tok), doc_(doc), end_(NULL)
        {
        }

        template<typename T, typename W>
        bool add( const W & value )
        {
          //Get the spot for the token...
          PageEntry<ArrayEntry> * array_entry = doc_->create_array_entry();
          array_entry->d.d.next = NULL;

          if( ! json_helper<T>::build( &array_entry->d.d.tok, doc_, value ) )
          {
            doc_->destroy_array( array_entry );
            return false;
          }

          //Hook it int the array
          if( end_ )
          {
            end_->d.d.next = &(array_entry->d.d);
          }
          else
          {
            tok_->data.array.ptr = &(array_entry->d.d);
          }
          end_ = array_entry;

          return true;
        }

        const fastjson::Token * token() const
        {
          return tok_;
        }

        fastjson::Token * tok_;
        Document * doc_;
        PageEntry<ArrayEntry> * end_;
    };

    Array * Document::create_free_array()
    {
      Token * token = create_free_token();
      token->type = Token::ArrayToken;
      token->data.array.ptr = NULL;
      return new Array( this, token );
    }

    Token * Document::create_free_token()
    {
      PageEntry<Token> * tok = free_tokens.create_value();
      return &(tok->d.d);
    }

    struct JsonNode
    {
      JsonNode( Document * doc ) : doc_(doc) {}

      template<typename T, typename W>
        bool add( const std::string & key, const W & value )
        {
          if( type != JN_Dict ) { return false; }
          return data.dict->add<T>(key,value);
        }

      template<typename T, typename W>
        bool add( const W & value )
        {
          if( type != JN_Array ) { return false; }
          return data.array->add<T>(value);
        }

      template<typename T>
        T get( const std::string & key ) const
        {
        }

      template<typename T>
        bool try_convert( T& t) const
      {
        return false;
      }

      void add_raw_dict_entry( const std::string & key, const JsonNode & n )
      {
      }

      void append_array_entry( const JsonNode & n )
      {
      }

      JsonNode get_child( const std::string & key ) const;


      static JsonNode dict( Document * doc )
      {
        JsonNode n(doc);
        n.type = JN_Dict;
        n.data.dict = doc->create_dictionary();
        return n;
      }

      static JsonNode array( Document * doc )
      {
        JsonNode n(doc);
        n.type = JN_Array;
        n.data.array = doc->create_free_array();
        return n;
      }

      std::string as_string()
      {
        return fastjson::as_string( token() );
      }

      const Token * token() const
      {
        switch( type )
        {
          case JN_Array:
            return data.array->token();
          case JN_Dict:
            return data.dict->token();
          default:
            return NULL;
        }
      }

      enum { JN_Array, JN_Dict } type;
      union { Array* array; Dictionary * dict; } data;

      Document * doc_;
    };


    //TODO: How do we avoid this causing leaks and memory unhappiness
    template<>
    struct json_helper<JsonNode>
    {
      static bool build( Token * tok, Document * doc, const JsonNode & node )
      {
        *tok = *node.token();
        return true; 
      }
    };

    template< typename T >
    struct json_helper< std::vector<T> >
    {
      static bool build( Token * tok, Document * doc, const std::vector<T> & v )
      {
        tok->type = Token::ArrayToken;
        tok->data.array.ptr  = NULL;
        Array a( doc, tok );
        for(unsigned int i=0; i<v.size(); ++i)
        {
          if( ! a.add<T>( v[i] ) ) return false;
        }
        return true;
      }
    };

  }
}

#endif
