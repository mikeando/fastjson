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
        fastjson::JsonElementCount jse;

        ErrorHelper eh;
        jse.user_error_callback = &ErrorHelper::on_error;
        jse.user_data = &eh;
        jse.mode = fastjson::mode::ext_any_as_key;

        bool ok = fastjson::count_elements( "{2:\"y\"}" , &jse );

        saru_assert(ok);
        saru_assert( !eh.ec_ );
  }

  void number_as_key_ok2()
  {
        fastjson::JsonElementCount jse;

        ErrorHelper eh;
        jse.user_error_callback = &ErrorHelper::on_error;
        jse.user_data = &eh;
        jse.mode = fastjson::mode::ext_any_as_key;

        bool ok = fastjson::count_elements( "{\"a\":\"y\",2:\"y\"}" , &jse );

        saru_assert(ok);
        saru_assert( !eh.ec_ );
  }

  void number_as_key_bad()
  {
        fastjson::JsonElementCount jse;

        ErrorHelper eh;
        jse.user_error_callback = &ErrorHelper::on_error;
        jse.user_data = &eh;
        jse.mode = 0;

        bool ok = fastjson::count_elements( "{2:\"y\"}" , &jse );

        saru_assert(!ok);
        saru_assert( eh.ec_ );
        saru_assert_equal("Unexpected character while parsing dict start", eh.ec_->mesg );
  }

  void number_as_key_bad2()
  {
        fastjson::JsonElementCount jse;

        ErrorHelper eh;
        jse.user_error_callback = &ErrorHelper::on_error;
        jse.user_data = &eh;
        jse.mode = 0;

        bool ok = fastjson::count_elements( "{\"a\":\"y\",2:\"y\"}" , &jse );

        saru_assert(!ok);
        saru_assert( eh.ec_ );
        saru_assert_equal("Unexpected character when looking for dict key", eh.ec_->mesg );
  }
};

int main()
{
  saru::TestLogger logger;
  SARU_TEST( TestFixture::number_as_key_ok, logger);
  SARU_TEST( TestFixture::number_as_key_ok2, logger);
  SARU_TEST( TestFixture::number_as_key_bad, logger);
  SARU_TEST( TestFixture::number_as_key_bad2, logger);
  logger.printSummary();

  return logger.allOK()?0:1;
}
