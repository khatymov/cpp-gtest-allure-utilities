#include "Allure2Listener.h"

#include "GTestAllureUtilities/AllureAPI.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <random>
#include <sstream>
#include <thread>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace fs = std::filesystem;

namespace systelab::gtest_allure {

static thread_local std::string tl_uuid;
static thread_local int64_t tl_startMs = 0;

static int64_t nowMs() {
  using namespace std::chrono;
  return static_cast<int64_t>(
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count());
}

static const char *statusFromGTest(const ::testing::TestResult &r) {
  if (r.Skipped())
    return "skipped";
  if (r.Failed())
    return "failed";
  return "passed";
}

static std::string hostName() {
#if defined(__unix__) || defined(__APPLE__)
  char buf[256] = {};
  if (::gethostname(buf, sizeof(buf) - 1) == 0)
    return buf;
#endif
  return "unknown-host";
}

static std::string threadLabel() {
  std::ostringstream os;
#if defined(__unix__) || defined(__APPLE__)
  os << ::getpid();
#else
  os << "0";
#endif
  os << "@" << hostName() << ".main(1)";
  return os.str();
}

static std::string stableHashHex(const std::string &s) {
  std::hash<std::string> h;
  std::ostringstream os;
  os << std::hex << static_cast<uint64_t>(h(s));
  return os.str();
}

long long Allure2Listener::nowMs() { return ::systelab::gtest_allure::nowMs(); }

std::string Allure2Listener::generateUuidV4() {
  static thread_local std::mt19937_64 rng{std::random_device{}()};
  std::uniform_int_distribution<uint64_t> dist(
      0, (std::numeric_limits<uint64_t>::max)());

  uint64_t a = dist(rng);
  uint64_t b = dist(rng);

  a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
  b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

  std::ostringstream os;
  os << std::hex << ((a >> 32) & 0xFFFFFFFFULL) << "-"
     << ((a >> 16) & 0xFFFFULL) << "-" << (a & 0xFFFFULL) << "-"
     << ((b >> 48) & 0xFFFFULL) << "-" << (b & 0xFFFFFFFFFFFFULL);
  return os.str();
}

void Allure2Listener::OnTestStart(const ::testing::TestInfo &ti) {
  tl_uuid = generateUuidV4();
  tl_startMs = nowMs();

  AllureAPI::beginTestCase(ti.test_suite_name(), ti.name(), tl_uuid);
}

void Allure2Listener::OnTestEnd(const ::testing::TestInfo &ti) {
  int64_t stopMs = nowMs();
  if (stopMs <= tl_startMs)
    stopMs = tl_startMs + 1;

  const auto &r = *ti.result();

  std::string outputDir = AllureAPI::getOutputFolder();
  if (outputDir.empty())
    outputDir = "allure-results";
  fs::create_directories(outputDir);

  const std::string suite = ti.test_suite_name();
  const std::string method = ti.name();
  const std::string fullName = suite + "." + method;

  const std::string testCaseName = AllureAPI::getCurrentTestCaseName().empty()
                                       ? method
                                       : AllureAPI::getCurrentTestCaseName();

  rapidjson::Document d(rapidjson::kObjectType);
  auto &a = d.GetAllocator();

  d.AddMember("uuid", rapidjson::Value(tl_uuid.c_str(), a), a);
  d.AddMember("historyId", rapidjson::Value(stableHashHex(fullName).c_str(), a),
              a);

  const std::string testCaseId =
      "[gtest]/[suite:" + suite + "]/[test:" + method + "]";
  d.AddMember("testCaseId", rapidjson::Value(testCaseId.c_str(), a), a);
  d.AddMember("testCaseName", rapidjson::Value(testCaseName.c_str(), a), a);
  d.AddMember("fullName", rapidjson::Value(fullName.c_str(), a), a);

  // ---------- Labels ----------
  rapidjson::Value labels(rapidjson::kArrayType);

  auto addLabel = [&](const char *k, const std::string &v) {
    rapidjson::Value o(rapidjson::kObjectType);
    o.AddMember("name", rapidjson::Value(k, a), a);
    o.AddMember("value", rapidjson::Value(v.c_str(), a), a);
    labels.PushBack(o, a);
  };

  addLabel("host", hostName());
  addLabel("thread", threadLabel());
  addLabel("framework", "gtest");
  addLabel("language", "cpp");
  addLabel("package", suite);
  addLabel("testClass", suite);
  addLabel("testMethod", method);
  addLabel("suite", suite);
  addLabel("junit.platform.uniqueid", testCaseId);

  // 🔹 Tags from AllureAPI
  for (const auto &tag : AllureAPI::getTags())
    addLabel("tag", tag);

  d.AddMember("labels", labels, a);

  // ---------- Links ----------
  rapidjson::Value links(rapidjson::kArrayType);
  const auto tms = AllureAPI::getTMSId();
  if (!tms.empty()) {
    rapidjson::Value l(rapidjson::kObjectType);
    l.AddMember("type", "tms", a);
    l.AddMember("name", "TMS", a);
    const auto url = AllureAPI::formatTMSLink(tms);
    l.AddMember(
        "url", rapidjson::Value(url.empty() ? tms.c_str() : url.c_str(), a), a);
    links.PushBack(l, a);
  }
  d.AddMember("links", links, a);


  d.AddMember("name", rapidjson::Value(testCaseName.c_str(), a), a);
  d.AddMember("status", rapidjson::Value(statusFromGTest(r), a), a);
  d.AddMember("stage", "finished", a);
  const auto description = AllureAPI::getDescription();
  d.AddMember("description", description, a);

  // ---------- Steps ----------
  rapidjson::Value steps(rapidjson::kArrayType);
  for (const auto &s : AllureAPI::getSteps()) {
    rapidjson::Value st(rapidjson::kObjectType);
    st.AddMember("name", rapidjson::Value(s.name.c_str(), a), a);
    st.AddMember("status", rapidjson::Value(s.status.c_str(), a), a);
    st.AddMember("stage", "finished", a);
    // st.AddMember("steps", rapidjson::Value(rapidjson::kArrayType), a);
    // st.AddMember("attachments", rapidjson::Value(rapidjson::kArrayType), a);
    // st.AddMember("parameters", rapidjson::Value(rapidjson::kArrayType), a);
    st.AddMember("start", rapidjson::Value().SetInt64(s.startMs), a);
    st.AddMember("stop", rapidjson::Value().SetInt64(s.stopMs), a);
    steps.PushBack(st, a);
  }
  d.AddMember("steps", steps, a);

  // ---- Test-level attachments ----
  rapidjson::Value atts(rapidjson::kArrayType);
  for (const auto &att : AllureAPI::getAttachments()) {
    rapidjson::Value o(rapidjson::kObjectType);
    o.AddMember("name", rapidjson::Value(att.name.c_str(), a), a);
    o.AddMember("source", rapidjson::Value(att.source.c_str(), a), a);
    o.AddMember("type", rapidjson::Value(att.type.c_str(), a), a);
    atts.PushBack(o, a);
  }
  d.AddMember("attachments", atts, a);

  // ---- Test-level parameters ----
  rapidjson::Value params(rapidjson::kArrayType);
  for (const auto &p : AllureAPI::getParameters()) {
    rapidjson::Value o(rapidjson::kObjectType);
    o.AddMember("name", rapidjson::Value(p.name.c_str(), a), a);
    o.AddMember("value", rapidjson::Value(p.value.c_str(), a), a);
    params.PushBack(o, a);
  }
  d.AddMember("parameters", params, a);

//   d.AddMember("attachments", rapidjson::Value(rapidjson::kArrayType), a);
//   d.AddMember("parameters", rapidjson::Value(rapidjson::kArrayType), a);

  d.AddMember("start", rapidjson::Value().SetInt64(tl_startMs), a);
  d.AddMember("stop", rapidjson::Value().SetInt64(stopMs), a);

  const fs::path out = fs::path(outputDir) / (tl_uuid + "-result.json");
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> wr(buf);
  d.Accept(wr);

  std::ofstream f(out, std::ios::binary);
  f << buf.GetString();
  f.close();

  AllureAPI::endTestCase();
}

} // namespace systelab::gtest_allure
