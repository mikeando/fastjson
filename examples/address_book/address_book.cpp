#include <string>
#include <map>
#include <vector>
#include <fstream>

#include "fastjson/core.h"
#include "fastjson/dom.h"

struct Address
{
  std::string name;
  std::string address;
  std::map<std::string,std::string> phone_numbers;
};

struct AddressBook
{
  std::vector<Address> addresses;
};



namespace fastjson { namespace dom {
    template<>
    struct json_helper<AddressBook>
    {
      static bool build( Token * tok, Chunk * chunk , const AddressBook & value )
      {
        return json_helper<std::vector<Address> >::build(tok,chunk,value.addresses);
      }
      static bool from_json_value( const Token * token, AddressBook * book )
      {
        if( ! book ) return false;
        return json_helper<std::vector<Address> >::from_json_value(token, &book->addresses );
      }
    };

    template<>
    struct json_helper<Address>
    {
      static bool build( Token * tok, Chunk * chunk , const Address & value )
      {
        Dictionary dict = Dictionary::create_dict(tok,chunk);
        dict.add<std::string>( "name", value.name ); 
        dict.add<std::string>( "address", value.address ); 
        dict.add<std::map<std::string,std::string> >( "phone", value.phone_numbers ); 
        return true;
      }

      static bool from_json_value( const Token * token, Address * address )
      {
        if(!token) return false;
        if(!address) return false;
        Dictionary_const dict = Dictionary_const::as_dict(token);
        if( ! dict.get<std::string>("name",&address->name) ) return false;
        if( ! dict.get<std::string>("address",&address->address) ) return false;
        if( ! dict.get<std::map<std::string,std::string> >("phone",&address->phone_numbers) ) return false;
        return true;
      }
    };

    template<>
    struct json_helper< std::map<std::string,std::string> >
    {
      static bool build( Token * tok, Chunk * chunk , const std::map<std::string,std::string> & value )
      {
        Dictionary dict = Dictionary::create_dict(tok,chunk);
        for( std::map<std::string,std::string>::const_iterator it = value.begin();
             it!=value.end();
             ++it )
        {
          dict.add<std::string>( it->first, it->second );
        }
        return true;
      }

      static bool from_json_value( const Token * token, std::map<std::string,std::string> * value )
      {
        if(!token) return false;
        if(!value) return false;
        if( token->type != Token::DictToken ) return false;
        DictEntry * e = token->dict.ptr;
        while( e )
        {
          std::string k;
          std::string v;
          //NOTE: We're not failing if we can't convert a key pair, we're just ignoring it.
          if(
              json_helper<std::string>::from_json_value( &e->key_tok, &k ) &&
              json_helper<std::string>::from_json_value( &e->value_tok, &v )  
            )
          {
            (*value)[k]=v;
          }
          e = e->next;
        }
        
        return true;
      }
    };
} }

void on_json_error( void *, const fastjson::ErrorContext& ec)
{
  std::cerr<<"ERROR: "<<ec.mesg<<std::endl;
}

bool add_to_address_book( AddressBook * book, const std::string & filename )
{
  std::ifstream f( filename.c_str(), std::ios::binary );
  if(!f)
  {
    std::cerr<<"Unable to open file \""<<filename<<"\""<<std::endl;
    return false;
  }

  //Read the entire file into an array.
  f.seekg(0,std::ios::end);
  std::streampos length = f.tellg();
  f.seekg(0,std::ios::beg);

  std::string buffer(length,(char)0);
  f.read(&buffer[0],length);

  fastjson::Token root;
  fastjson::dom::Chunk chunk;
  
  if( ! fastjson::dom::parse_string( buffer, &root, &chunk, 0, &on_json_error, NULL ) )
  {
    return false;
  }

  fastjson::dom::json_helper<AddressBook>::from_json_value( &root, book );

  return true;
}

bool write_address_book( AddressBook * book, const std::string & filename )
{
  std::ofstream f( filename.c_str(), std::ios::binary );
  if(!f)
  {
    std::cerr<<"Unable to open file \""<<filename<<"\""<<std::endl;
    return false;
  }

  fastjson::dom::Chunk chunk;
  fastjson::Token token;

  fastjson::dom::json_helper<AddressBook>::build(&token,&chunk,*book);

  //Write to file
  f<<fastjson::as_string(&token);
  return true;
}

int main()
{
  AddressBook book;
  add_to_address_book(&book, "book.json");
  for(unsigned int i=0; i<book.addresses.size() ; ++i)
  {
    book.addresses[i].name += "XXX";
  }
  write_address_book(&book, "book.mod.json");
}
