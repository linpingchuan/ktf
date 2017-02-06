#ifndef KTEST_UTEST_H
#define KTEST_UTEST_H
#include <string>
#include <vector>

/* User mode side of extension to the gtest unit test framework:
 *  1) Kernel test support via netlink
 *  2) Standard command line parameters
 *
 * This is now a generic interface.
 * For gtest integration of kernel tests, see ktest_run.{cpp,h}
 */

typedef std::vector<std::string> stringvec;

namespace utest
{

  class KernelTest;

  /* A callback handler to be called for each assertion result */
  typedef void (*test_handler)(int result,  const char* file, int line, const char* report);

  class test_cb
  {
  public:
    virtual ~test_cb() {}
    virtual test_cb* as_test_cb() { return this; }
    virtual void fun(KernelTest* kt) {}
  };

  class KernelTest
  {
  public:
    KernelTest(const std::string&,const char*);
    std::string setname;
    std::string testname;
    std::string name;
    size_t setnum;  /* This test belongs to this set in the kernel */
    size_t testnum; /* This test's index (test number) in the kernel */
    unsigned value;  /* Optional value argument to the test */
    test_cb* user_test;  /* Optional user level wrapper function for the kernel test */
    char* file;
    int line;
  };

  // Set up connection to the kernel test driver:
  // @handle_test contains the test framework's handling code for test assertions */
  bool setup(test_handler handle_test = NULL);

  // Parse command line args (call after gtest arg parsing)
  char** parse_opts(int argc, char** argv);

  /* Query kernel for available tests in index order */
  stringvec& query_testsets();

  stringvec get_testsets();
  std::string get_current_setname();
  stringvec get_test_names();

  KernelTest* find_test(const std::string& setname, const std::string& testname);

  /* "private" - only run from gtest framework */
  void run_test(KernelTest* test);

  /* This is the function to call from a KTEST() wrapper
   * definition when the kernel part of the test should be run:
   */
  void run_kernel_test(KernelTest* kt);

  /* Function for adding a user level test wrapper */
  void add_wrapper(const std::string setname, const std::string testname, test_cb* tcb);
} // end namespace utest


/* Redefine for C++ until we can get it patched - type mismatch by default */
#ifdef nla_for_each_nested
#undef nla_for_each_nested
#endif
#define nla_for_each_nested(pos, nla, rem) \
  for (pos = (struct nlattr*)nla_data(nla), rem = nla_len(nla);	\
             nla_ok(pos, rem); \
             pos = nla_next(pos, &(rem)))

#define KTEST(__setname,__testname) \
  class __setname ## _ ## __testname : public utest::test_cb	\
  {\
  public:\
    __setname ## _ ## __testname() {\
      utest::add_wrapper(#__setname,#__testname,as_test_cb()); \
    }\
    virtual void fun(utest::KernelTest* kt);	\
  }; \
  __setname ## _ ## __testname \
     __setname ## _ ## __testname ## _value;\
  void __setname ## _ ## __testname::fun(utest::KernelTest* kt)

#endif
