#include "AllureAPI.h"

#include "Model/TestProperty.h"
#include "Services/EventHandlers/ITestStepEndEventHandler.h"
#include "Services/EventHandlers/ITestStepStartEventHandler.h"
#include "Services/GoogleTest/IGTestStatusChecker.h"
#include "Services/Property/ITestCasePropertySetter.h"
#include "Services/Property/ITestSuitePropertySetter.h"
#include "Services/ServicesFactory.h"

#include <chrono>
#include <mutex>

namespace {
std::mutex g_mutex;

std::string g_outputFolder = "allure-results";
std::string g_tmsId;
std::string g_tmsPattern;
std::string g_epic;
std::string g_severity;
std::map<std::string, std::string> g_suiteLabels;

// per-test (thread-local)
thread_local std::string tl_suite;
thread_local std::string tl_case;
thread_local std::string tl_uuid;
thread_local std::vector<systelab::gtest_allure::AllureAPI::Step> tl_steps;

static long long nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}
} // namespace

namespace systelab {
namespace gtest_allure {

model::TestProgram AllureAPI::m_testProgram = model::TestProgram();
service::IServicesFactory *AllureAPI::m_servicesFactory =
    new service::ServicesFactory(m_testProgram);

void AllureAPI::beginTestCase(const std::string &suiteName,
                              const std::string &gtestName,
                              const std::string &uuid) {
  tl_steps.clear();
  tl_uuid = uuid;

  // suite can be overridden globally by setTestSuiteName if you have it;
  // here we just use what listener passes.
  tl_suite = suiteName;

  // case name can be overridden by user calling setTestCaseName inside test
  if (tl_case.empty())
    tl_case = gtestName;
}

void AllureAPI::endTestCase() {
  tl_case.clear();
  tl_suite.clear();
  tl_uuid.clear();
  tl_steps.clear();
}

std::unique_ptr<::testing::TestEventListener> AllureAPI::buildListener() {
  return getServicesFactory()->buildGTestEventListener();
}

model::TestProgram &AllureAPI::getTestProgram() { return m_testProgram; }

void AllureAPI::setTestProgramName(const std::string &name) {
  m_testProgram.setName(name);
}

void AllureAPI::setOutputFolder(const std::string &outputFolder) {
  {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_outputFolder = outputFolder;
  }
  m_testProgram.setOutputFolder(outputFolder);
}

void AllureAPI::setTMSLinksPattern(const std::string &tmsLinkPattern) {
  {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_tmsPattern = tmsLinkPattern;
  }
  m_testProgram.setTMSLinksPattern(tmsLinkPattern);
}

void AllureAPI::setFormat(model::Format format) {
  m_testProgram.setFormat(format);
}

void AllureAPI::setTMSId(const std::string &value) {
  {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_tmsId = value;
  }
  auto testSuitePropertySetter =
      getServicesFactory()->buildTestSuitePropertySetter();
  testSuitePropertySetter->setProperty(model::test_property::TMS_ID_PROPERTY,
                                       value);
}

void AllureAPI::setTestSuiteName(const std::string &name) {
  setTestSuiteLabel(model::test_property::NAME_PROPERTY, name);
}

void AllureAPI::setTestSuiteDescription(const std::string &description) {
  setTestSuiteLabel(model::test_property::FEATURE_PROPERTY, description);
}

void AllureAPI::setTestSuiteEpic(const std::string &epic) {
  {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_epic = epic;
  }
  setTestSuiteLabel(model::test_property::EPIC_PROPERTY, epic);
}

void AllureAPI::setTestSuiteSeverity(const std::string &severity) {
  {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_severity = severity;
  }
  setTestSuiteLabel(model::test_property::SEVERITY_PROPERTY, severity);
}

void AllureAPI::setTestSuiteLabel(const std::string &name,
                                  const std::string &value) {
  {
    std::lock_guard<std::mutex> lk(g_mutex);
    g_suiteLabels[name] = value;
  }
  auto testSuitePropertySetter =
      getServicesFactory()->buildTestSuitePropertySetter();
  testSuitePropertySetter->setProperty(name, value);
}

void AllureAPI::setTestCaseName(const std::string &name) {
  auto testCasePropertySetter =
      getServicesFactory()->buildTestCasePropertySetter();
  testCasePropertySetter->setProperty(model::test_property::NAME_PROPERTY,
                                      name);
}

void AllureAPI::addAction(const std::string &name,
                          std::function<void()> actionFunction) {
  addStep(name, true, actionFunction);
}

void AllureAPI::addExpectedResult(const std::string &name,
                                  std::function<void()> verificationFunction) {
  addStep(name, false, verificationFunction);
}

void AllureAPI::addStep(const std::string &name, bool isAction,
                        std::function<void()> stepFunction) {
  Step s;
  s.name = name;
  s.startMs = nowMs();

  auto stepStartEventHandler =
      getServicesFactory()->buildTestStepStartEventHandler();
  stepStartEventHandler->handleTestStepStart(name, isAction);

  try {
    stepFunction();

    auto statusChecker = getServicesFactory()->buildGTestStatusChecker();
    auto currentStatus = statusChecker->getCurrentTestStatus();

    // Best-effort mapping:
    // If your IGTestStatusChecker returns something else, adjust here.
    // We'll use "passed" unless the test is failing at this point.
    s.status = (::testing::Test::HasFailure() ? "failed" : "passed");

    auto stepEndEventHandler =
        getServicesFactory()->buildTestStepEndEventHandler();
    stepEndEventHandler->handleTestStepEnd(currentStatus);
  } catch (...) {
    // Exceptions inside steps -> "broken" in Allure terms
    s.status = "broken";
    s.stopMs = nowMs();
    tl_steps.push_back(std::move(s));
    throw;
  }

  s.stopMs = nowMs();
  tl_steps.push_back(std::move(s));
}

std::string AllureAPI::getOutputFolder() {
  std::lock_guard<std::mutex> lk(g_mutex);
  return g_outputFolder;
}

std::string AllureAPI::getCurrentTestSuiteName() { return tl_suite; }

std::string AllureAPI::getCurrentTestCaseName() { return tl_case; }

std::string AllureAPI::getTMSId() {
  std::lock_guard<std::mutex> lk(g_mutex);
  return g_tmsId;
}

std::string AllureAPI::getTestSuiteEpic() {
  std::lock_guard<std::mutex> lk(g_mutex);
  return g_epic;
}

std::string AllureAPI::getTestSuiteSeverity() {
  std::lock_guard<std::mutex> lk(g_mutex);
  return g_severity;
}

const std::map<std::string, std::string> &AllureAPI::getTestSuiteLabels() {
  return g_suiteLabels;
}

const std::vector<AllureAPI::Step> &AllureAPI::getSteps() { return tl_steps; }

std::string AllureAPI::formatTMSLink(const std::string &tmsId) {
  std::lock_guard<std::mutex> lk(g_mutex);
  if (g_tmsPattern.empty())
    return {};

  const std::string needle = "{}";
  const auto pos = g_tmsPattern.find(needle);
  if (pos == std::string::npos)
    return g_tmsPattern;

  return g_tmsPattern.substr(0, pos) + tmsId +
         g_tmsPattern.substr(pos + needle.size());
}

service::IServicesFactory *AllureAPI::getServicesFactory() {
  auto configuredServicesFactoryInstance =
      service::ServicesFactory::getInstance();
  if (configuredServicesFactoryInstance) {
    return configuredServicesFactoryInstance;
  } else {
    return m_servicesFactory;
  }
}

} // namespace gtest_allure
} // namespace systelab
