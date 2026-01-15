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
  struct Attachment {
    std::string name;
    std::string source;
    std::string type;
  };

  struct Parameter {
    std::string name;
    std::string value;
  };

  struct Step {
    std::string name;
    std::string status;
    long long startMs{};
    long long stopMs{};

    std::vector<Attachment> attachments;
    std::vector<Parameter> parameters;
  };

  // Allure2Listener support
  static void beginTestCase(const std::string &suiteName,
                            const std::string &gtestName,
                            const std::string &uuid);
  static void endTestCase();

  // ===== Allure2Listener getters =====
  static std::string getOutputFolder();

  static std::string getCurrentTestSuiteName();
  static std::string getCurrentTestCaseName();

  static std::string getTMSId();
  static std::string getTestSuiteEpic();
  static std::string getTestSuiteSeverity();
  static std::string getDescription();
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

  // tags
  static void addTag(const std::string &tag);
  static const std::vector<std::string> &getTags();

  // attachments
  static void addAttachment(const std::string &name, const std::string &type,
                            const std::string &filePath);
  static const std::vector<Attachment> &getAttachments();

  // parameters
  static void addParameter(const std::string &name, const std::string &value);
  static const std::vector<Parameter> &getParameters();

private:
  static void addStep(const std::string &name, bool isAction,
                      std::function<void()>);
  static service::IServicesFactory *getServicesFactory();
  static model::TestCase *getRunningTestCase();

private:
  static model::TestProgram m_testProgram;
  static service::IServicesFactory *m_servicesFactory;
};

} // namespace gtest_allure
} // namespace systelab
