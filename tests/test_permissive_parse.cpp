// SARU : Tag fastjson
// SARU : CxxLibs -L.. -lfastjson

#include "saru_cxx.h"

#include "fastjson/fastjson.h"

struct TestFixture
{
  struct ErrorHelper
  {
    ErrorHelper() : ec_(NULL) {}
    ~ErrorHelper() { delete ec_; }
    static void on_error(
        void* in_this,
        const fastjson::ErrorContext & ec )
    {
      
      ErrorHelper * eh = static_cast<ErrorHelper*>(in_this);
      delete eh->ec_;
      eh->ec_ = new fastjson::ErrorContext(ec);
    }

    fastjson::ErrorContext * ec_;
  };

  void number_as_key_ok()
  {
        unsigned char buffer[] = "xx";
        fastjson::DictEntry  dict_entries[1];

        fastjson::Document doc;
        doc.mode = fastjson::mode::ext_any_as_key;

        doc.string_store = buffer; //Two single char entries
        doc.array_store = NULL;
        doc.dict_store  = dict_entries;

        ErrorHelper eh;
        doc.user_error_callback = &ErrorHelper::on_error;
        doc.user_data = &eh;

        bool ok = fastjson::parse_doc( "{2:\"y\"}" , &doc );
        saru_assert(ok);
        saru_assert( !eh.ec_ );
  }

  void number_as_key_bad()
  {
        unsigned char buffer[] = "xx";
        fastjson::DictEntry  dict_entries[1];

        fastjson::Document doc;
        doc.mode = 0;

        doc.string_store = buffer; //Two single char entries
        doc.array_store = NULL;
        doc.dict_store  = dict_entries;

        ErrorHelper eh;
        doc.user_error_callback = &ErrorHelper::on_error;
        doc.user_data = &eh;

        //Create this first so it will exist after we get our error messages.
        std::string json("{2:\"y\"}");

        bool ok = fastjson::parse_doc( json, &doc );
        saru_assert(!ok);

        saru_assert( eh.ec_ );
        saru_assert_equal("Unexpected character while parsing dict start", eh.ec_->mesg );
  }
};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::number_as_key_ok, logger);
  SARU_TEST( TestFixture::number_as_key_bad, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}
