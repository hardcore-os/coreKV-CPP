#ifndef EXAMPLES_TEST_FRAME_WORK_H_
#define EXAMPLES_TEST_FRAME_WORK_H_
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "color.h"

namespace corekv {
#define green redbud::io::state::manual << redbud::io::hfg::green
#define blue redbud::io::state::manual << redbud::io::hfg::blue
#define red redbud::io::state::manual << redbud::io::hfg::red

enum class TestCaseResult { kSuccess = 0, kFailed = 1, kIgnore = 2 };
class SingleTestCase {
 public:
  SingleTestCase(const char* test_case_name, const char* func_name)
      : test_case_name_(test_case_name),func_name_(func_name) {}
  ~SingleTestCase() {} // 注意虚函数，否则会资源泄漏
  virtual void Run() = 0;  //纯虚函数
  // 获取当前类名
  const char* GetCurrTestCaseName() { return test_case_name_; }
  // 获取当前函数名
  const char* GetCurrFuncName() { return func_name_; }
  bool IsSuccess() { return test_case_reulst_ != TestCaseResult::kFailed; }
  void SetExecStatus(TestCaseResult test_case_result) {
    test_case_reulst_ = test_case_result;
  }

 private:
  const char* test_case_name_ = nullptr;
  const char* func_name_ = nullptr;
  TestCaseResult test_case_reulst_ = TestCaseResult::kSuccess;
};
//一个单元测试可能包含多个testcase
class TestUnit final {
 public:
  //单例
  static TestUnit* GetInstance();
  corekv::SingleTestCase* RegisterTestCase(
      std::shared_ptr<SingleTestCase> single_test_case);
  void RunAll();
  SingleTestCase* GetCurrentTestCase() { return current_test_case_; }

 private:
  SingleTestCase* current_test_case_ = nullptr;
  std::vector<std::shared_ptr<SingleTestCase>> test_cases_;
  uint64_t success_count_ = 0;  //通过的
  uint64_t failed_count_ = 0;   //未通过的
};
}  // namespace corekv

#define PASSED std::cout << "[ PASSED ]\n"
// 测试案例的类名，替换为 test_cast_TEST
#define TESTCASE_NAME(testcase_name) testcase_name##_TEST

#define CUSTOM_TEST_CLASS_(testcase_name,test_func_name)                                    \
  class TESTCASE_NAME(testcase_name) final : public corekv::SingleTestCase { \
   public:                                                                   \
    TESTCASE_NAME(testcase_name)                                             \
    (const char* case_name, const char*func_name) : corekv::SingleTestCase(case_name,func_name ){};           \
    virtual void Run();                                                      \
                                                                             \
   private:                                                                  \
    static corekv::SingleTestCase* testcase_;                                \
  };                                                                         \
                                                                             \
  corekv::SingleTestCase* TESTCASE_NAME(testcase_name)::testcase_ =          \
      corekv::TestUnit::GetInstance()->RegisterTestCase(                     \
          std::make_shared<TESTCASE_NAME(testcase_name)>(#testcase_name,#test_func_name));   \
  void TESTCASE_NAME(testcase_name)::Run()

// 简单测试的宏定义
#define TEST(testcase_name, test_func_name) CUSTOM_TEST_CLASS_(testcase_name,test_func_name)

// 运行所有测试案例
#define RUN_ALL_TESTS() corekv::TestUnit::GetInstance()->RunAll()

/*
使用方式如下：
TEST(AddTestDemo)
{
EXPECT_EQ(3, Add(1, 2));
EXPECT_EQ(2, Add(1, 1));
}
上述代码将 { EXPECT_EQ(3, Add(1, 2)); EXPECT_EQ(2, Add(1, 1)); } 接到 Run()
的后面
*/

/*****************************************************************************************/

// 简单测试的宏定义
// 断言 : 宏定义形式为 EXPECT_* ，符合验证条件的，案例测试通过，否则失败
// 使用一系列的宏来封装验证条件，分为以下几大类 :

/*
真假断言
EXPECT_TRUE  验证条件: Condition 为 true
EXPECT_FALSE 验证条件: Condition 为 false

Example:
bool isPrime(int n);         一个判断素数的函数
EXPECT_TRUE(isPrime(2));     通过
EXPECT_FALSE(isPrime(4));    通过
EXPECT_TRUE(isPrime(6));     失败
EXPECT_FALSE(isPrime(3));    失败
*/
#define EXPECT_TRUE(Condition)                                              \
  do {                                                                      \
    if (Condition) {                                                        \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_TRUE succeeded!\n";                    \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_TRUE failed!\n";                         \
    }                                                                       \
  } while (0)

#define EXPECT_FALSE(Condition)                                             \
  do {                                                                      \
    if (!Condition) {                                                       \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_FALSE succeeded!\n";                   \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << "  EXPECT_FALSE failed!\n";                       \
    }                                                                       \
  } while (0)

/*
比较断言
EXPECT_EQ(v1, v2) 验证条件: v1 == v2
EXPECT_NE(v1, v2) 验证条件: v1 != v2
EXPECT_LT(v1, v2) 验证条件: v1 <  v2
EXPECT_LE(v1, v2) 验证条件: v1 <= v2
EXPECT_GT(v1, v2) 验证条件: v1 >  v2
EXPECT_GE(v1, v2) 验证条件: v1 >= v2

Example:
EXPECT_EQ(3, foo());
EXPECT_NE(NULL, pointer);
EXPECT_LT(len, v.size());
*/
#define EXPECT_EQ(v1, v2)                                                   \
  do {                                                                      \
    if (v1 == v2) {                                                         \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_EQ succeeded!\n";                      \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_EQ failed!\n";                           \
      std::cout << red << " Expect:" << v1 << "\n";                         \
      std::cout << red << " Actual:" << v2 << "\n";                         \
    }                                                                       \
  } while (0)

#define EXPECT_NE(v1, v2)                                                   \
  do {                                                                      \
    if (v1 != v2) {                                                         \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_NE succeeded!\n";                      \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_NE failed!\n";                           \
      std::cout << red << " Expect:" << v1 << "\n";                         \
      std::cout << red << " Actual:" << v2 << "\n";                         \
    }                                                                       \
  } while (0)

#define EXPECT_LT(v1, v2)                                                   \
  do {                                                                      \
    if (v1 < v2) {                                                          \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_LT succeeded!\n";                      \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_LT failed!\n";                           \
      std::cout << red << " Expect:" << v1 << "\n";                         \
      std::cout << red << " Actual:" << v2 << "\n";                         \
    }                                                                       \
  } while (0)

#define EXPECT_LE(v1, v2)                                                   \
  do {                                                                      \
    if (v1 <= v2) {                                                         \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_LE succeeded!\n";                      \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_LE failed!\n";                           \
      std::cout << red << " Expect:" << v1 << "\n";                         \
      std::cout << red << " Actual:" << v2 << "\n";                         \
    }                                                                       \
  } while (0)

#define EXPECT_GT(v1, v2)                                                   \
  do {                                                                      \
    if (v1 > v2) {                                                          \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_GT succeeded!\n";                      \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_GT failed!\n";                           \
      std::cout << red << " Expect:" << v1 << "\n";                         \
      std::cout << red << " Actual:" << v2 << "\n";                         \
    }                                                                       \
  } while (0)

#define EXPECT_GE(v1, v2)                                                   \
  do {                                                                      \
    if (v1 >= v2) {                                                         \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kSuccess);                                \
      std::cout << green << " EXPECT_GE succeeded!\n";                      \
    } else {                                                                \
      corekv::TestUnit::GetInstance()->GetCurrentTestCase()->SetExecStatus( \
          corekv::TestCaseResult::kFailed);                                 \
      std::cout << red << " EXPECT_GE failed!\n";                           \
      std::cout << red << " Expect:" << v1 << "\n";                         \
      std::cout << red << " Actual:" << v2 << "\n";                         \
    }                                                                       \
  } while (0)

#endif