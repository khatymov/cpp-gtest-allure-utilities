#pragma once

#include "Model/Format.h"
#include "Model/TestProgram.h"

#include <functional>
#include <gtest/gtest.h>
#include <memory>

#include <map>
#include <string>
#include <vector>

namespace systelab {
namespace gtest_allure {

namespace service {
class IServicesFactory;
}

class AllureAPI {
public:
  struct Step {
    std::string name;
    std::string status; // "passed"/"failed"/"broken"/"skipped"
    long long startMs{};
    long long stopMs{};
  };

  // Allure2Listener support
  static void beginTestCase(const std::string &suiteName,
                            const std::string &gtestName,
                            const std::string &uuid);
  static void endTestCase();

  // ===== Allure2Listener getters =====
  // ===== Allure2Listener getters =====
  static std::string getOutputFolder();

  static std::string getCurrentTestSuiteName();
  static std::string getCurrentTestCaseName();

  static std::string getTMSId();
  static std::string getTestSuiteEpic();
  static std::string getTestSuiteSeverity();
  static const std::map<std::string, std::string> &getTestSuiteLabels();

  static const std::vector<Step> &getSteps();
  static std::string formatTMSLink(const std::string &tmsId);

  static std::unique_ptr<::testing::TestEventListener> buildListener();

  static model::TestProgram &getTestProgram();
  static void setTestProgramName(const std::string &);
  static void setOutputFolder(const std::string &);
  static void setTMSLinksPattern(const std::string &);
  static void setFormat(model::Format format);

  static void setTMSId(const std::string &);
  static void setTestSuiteName(const std::string &);
  static void setTestSuiteDescription(const std::string &);
  static void setTestSuiteEpic(const std::string &);
  static void setTestSuiteSeverity(const std::string &);
  static void setTestSuiteLabel(const std::string &name,
                                const std::string &value);

  static void setTestCaseName(const std::string &);
  static void addAction(const std::string &name, std::function<void()>);
  static void addExpectedResult(const std::string &name, std::function<void()>);

private:
  static void addStep(const std::string &name, bool isAction,
                      std::function<void()>);
  static service::IServicesFactory *getServicesFactory();

private:
  static model::TestProgram m_testProgram;
  static service::IServicesFactory *m_servicesFactory;
};

} // namespace gtest_allure
} // namespace systelab
