FastJson
========

FastJson is a library for reading and writing json in C++. Its designed to be fast and light, yet complete.

How to use FastJson
-------------------

FastJson has an odd but functional interface. We will just look at the high-level interface here.

First FastJson uses two constructs Tokens and Chunks. A Token is like a node in the JSON, however the Token does not
own any of the memory that it references. The owner of this memory is (usually) a Chunk.

So parsing a json string looks like this:

    fastjson::Token token;
    fastjson::dom::Chunk chunk;
    std::string error_message;
    if( ! fastjson::dom::parse_string("{}", &token, &chunk, &error_message) )
    {
      std::cerr<<"ERROR : "<<error_message<<std::endl;
      return;
    }

Writing one back to a string is even easier

    std::string s = fastjson::as_string( token );

Manipulating or interogating the JSON tree is a little uglier - for this there are some helper classes in the dom namespace

    fastjson::dom::Dictionary dict = fastjson::dom::Dictionary::as_dict( &token, &chunk );
    //Get the id field
    int id;
    if( dict.get<int>("an_id", &id) )
    {
      std::cerr<<"NO id found"<<std::endl;
      return;
    }
    double f = id+0.5;
    dict.add<double>( "something", &f );

To properly navigate a deeper tree you need to use some of the lower-level constructs, covered in the next section.
Some of this will be hidden when I work out the neatest way.


FastJson under the covers
-------------------------
First some things to note:

1. FastJson uses an analyse then allocate approach, meaning that while it reads over the json twice, it only allocates its data in large blocks. This makes it fast.
2. FastJson values are stored as strings with a type hint. This means large numbers can be read easily without fastjson caring about precission or length, this is the users concern.
3. Since we allocate in big chunks, we need something to do our memory management for us .. thats where the Chunk objects come in.

Now some words about tokens. A token is just a discriminated union type. It can either be an ArrayType, DictType, ValueType, or one of the Litterals (Null, False, True). Most of these types contain further pointers to data or more tokens.

This means traversing an Array in json looks like this:

    Token token = ...
    if( token.type != Token::ArrayToken ) { ... }
    ArrayEntry * child = token.data.array.ptr;
    while(child)
    {
       ...
       child = child->next;
    }

Traversing a dictionary is similar.






 


   



