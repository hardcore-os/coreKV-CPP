#include "test_frame.h"

namespace corekv {
TestUnit* TestUnit::GetInstance() {
  static TestUnit gTestCase;
  return &gTestCase;
}
corekv::SingleTestCase* TestUnit::RegisterTestCase(
    std::shared_ptr<SingleTestCase> single_test_case) {
  current_test_case_ = single_test_case.get();
  test_cases_.emplace_back(single_test_case);
  return current_test_case_;
}
void TestUnit::RunAll() {
  for (auto& iter : test_cases_) {
    std::cout << blue << "============================================";
    std::cout << green << "\n Run TestCase:" << iter->GetCurrTestCaseName()
              << "." << iter->GetCurrFuncName() << "\n";
    iter->Run();
    if (iter->IsSuccess()) {
      ++success_count_;
      std::cout << green << " Success!!!" << std::endl;
    } else {
      ++failed_count_;
      std::cout << red << " Failed!!!" << std::endl;
    }
    std::cout << green << " End TestCase:" << iter->GetCurrTestCaseName() << "."
              << iter->GetCurrFuncName() << std::endl;
  }
  std::cout << blue << "============================================\n";
  std::cout << green << " Total TestCase : " << success_count_ + failed_count_
            << "\n";
  std::cout << green << " Total Passed : " << success_count_ << "\n";
  std::cout << red << " Total Failed : " << failed_count_ << "\n";
  std::cout << green << " " << success_count_ << " / "
            << failed_count_ + success_count_ << " TestCases passed. ( "
            << success_count_ * 1.0 / (failed_count_ + success_count_) * 100
            << "% )\n";
}
}  // namespace corekv