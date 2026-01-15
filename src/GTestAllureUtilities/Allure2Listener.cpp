#include "Allure2Listener.h"
#include "AllureAPI.h"
#include "Model/TestProperty.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace fs = std::filesystem;

namespace systelab::gtest_allure {

static thread_local std::string tl_uuid;
static thread_local int64_t tl_startMs = 0;

static const char* statusFromGTest(const ::testing::TestResult& r)
{
    if (r.Skipped()) return "skipped";
    if (r.Failed())  return "failed";
    return "passed";
}

long long Allure2Listener::nowMs()
{
    using namespace std::chrono;
    return static_cast<long long>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()
    );
}

std::string Allure2Listener::generateUuidV4()
{
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist(0, (std::numeric_limits<uint64_t>::max)());

    uint64_t a = dist(rng);
    uint64_t b = dist(rng);

    // RFC-4122 v4 + variant
    a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL; // version 4
    b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL; // variant 10xx

    std::ostringstream os;
    os << std::hex
       << ((a >> 32) & 0xFFFFFFFFULL) << "-"
       << ((a >> 16) & 0xFFFFULL)     << "-"
       << (a & 0xFFFFULL)             << "-"
       << ((b >> 48) & 0xFFFFULL)     << "-"
       << (b & 0xFFFFFFFFFFFFULL);
    return os.str();
}

void Allure2Listener::OnTestStart(const ::testing::TestInfo& /*testInfo*/)
{
    tl_uuid = generateUuidV4();
    tl_startMs = static_cast<int64_t>(nowMs());
}

void Allure2Listener::OnTestEnd(const ::testing::TestInfo& testInfo)
{
    const int64_t stopMs = static_cast<int64_t>(nowMs());
    const auto& r = *testInfo.result();

    const std::string outputDir = AllureAPI::getOutputFolder();
    fs::create_directories(outputDir);

    rapidjson::Document doc(rapidjson::kObjectType);
    auto& alloc = doc.GetAllocator();

    doc.AddMember("uuid", rapidjson::Value(tl_uuid.c_str(), alloc), alloc);

    const std::string suite = testInfo.test_suite_name();
    const std::string name  = testInfo.name(); // TEST_P instantiations differ here
    const std::string fullName = suite + "." + name;

    doc.AddMember("name", rapidjson::Value(name.c_str(), alloc), alloc);
    doc.AddMember("fullName", rapidjson::Value(fullName.c_str(), alloc), alloc);

    doc.AddMember("status", rapidjson::Value(statusFromGTest(r), alloc), alloc);
    doc.AddMember("stage", rapidjson::Value("finished", alloc), alloc);

    // Fix RapidJSON long long ambiguity:
    doc.AddMember("start", rapidjson::Value().SetInt64(tl_startMs), alloc);
    doc.AddMember("stop",  rapidjson::Value().SetInt64(stopMs),  alloc);

    const auto& suiteLabels = AllureAPI::getTestSuiteLabels();
    const auto& tags = AllureAPI::getTags();
    const auto description = AllureAPI::getDescription();

    const auto itSuiteName = suiteLabels.find(model::test_property::NAME_PROPERTY);
    const std::string suiteLabelValue =
        (itSuiteName != suiteLabels.end()) ? itSuiteName->second : suite;

    rapidjson::Value labels(rapidjson::kArrayType);
    {
        rapidjson::Value l(rapidjson::kObjectType);
        l.AddMember("name", rapidjson::Value("suite", alloc), alloc);
        l.AddMember("value", rapidjson::Value(suiteLabelValue.c_str(), alloc), alloc);
        labels.PushBack(l, alloc);
    }

    for (const auto& [nameKey, value] : suiteLabels)
    {
        if (nameKey == model::test_property::NAME_PROPERTY)
            continue;

        rapidjson::Value l(rapidjson::kObjectType);
        l.AddMember("name", rapidjson::Value(nameKey.c_str(), alloc), alloc);
        l.AddMember("value", rapidjson::Value(value.c_str(), alloc), alloc);
        labels.PushBack(l, alloc);
    }

    for (const auto& tag : tags)
    {
        rapidjson::Value l(rapidjson::kObjectType);
        l.AddMember("name", rapidjson::Value("tag", alloc), alloc);
        l.AddMember("value", rapidjson::Value(tag.c_str(), alloc), alloc);
        labels.PushBack(l, alloc);
    }

    doc.AddMember("labels", labels, alloc);

    if (!description.empty())
    {
        doc.AddMember("description", rapidjson::Value(description.c_str(), alloc), alloc);
    }

    const auto tmsId = AllureAPI::getTMSId();
    if (!tmsId.empty())
    {
        rapidjson::Value links(rapidjson::kArrayType);
        rapidjson::Value l(rapidjson::kObjectType);
        l.AddMember("type", rapidjson::Value("tms", alloc), alloc);
        l.AddMember("name", rapidjson::Value(tmsId.c_str(), alloc), alloc);
        const auto tmsLink = AllureAPI::formatTMSLink(tmsId);
        if (!tmsLink.empty())
            l.AddMember("url", rapidjson::Value(tmsLink.c_str(), alloc), alloc);
        links.PushBack(l, alloc);
        doc.AddMember("links", links, alloc);
    }

    const auto& steps = AllureAPI::getSteps();
    if (!steps.empty())
    {
        rapidjson::Value stepsArray(rapidjson::kArrayType);
        for (const auto& step : steps)
        {
            rapidjson::Value s(rapidjson::kObjectType);
            s.AddMember("name", rapidjson::Value(step.name.c_str(), alloc), alloc);
            s.AddMember("status", rapidjson::Value(step.status.c_str(), alloc), alloc);
            s.AddMember("stage", rapidjson::Value("finished", alloc), alloc);
            s.AddMember("start", rapidjson::Value().SetInt64(step.startMs), alloc);
            s.AddMember("stop",  rapidjson::Value().SetInt64(step.stopMs),  alloc);

            if (!step.parameters.empty())
            {
                rapidjson::Value params(rapidjson::kArrayType);
                for (const auto& param : step.parameters)
                {
                    rapidjson::Value p(rapidjson::kObjectType);
                    p.AddMember("name", rapidjson::Value(param.name.c_str(), alloc), alloc);
                    p.AddMember("value", rapidjson::Value(param.value.c_str(), alloc), alloc);
                    params.PushBack(p, alloc);
                }
                s.AddMember("parameters", params, alloc);
            }

            if (!step.attachments.empty())
            {
                rapidjson::Value attachments(rapidjson::kArrayType);
                for (const auto& attachment : step.attachments)
                {
                    rapidjson::Value a(rapidjson::kObjectType);
                    a.AddMember("name", rapidjson::Value(attachment.name.c_str(), alloc), alloc);
                    a.AddMember("source", rapidjson::Value(attachment.source.c_str(), alloc), alloc);
                    a.AddMember("type", rapidjson::Value(attachment.type.c_str(), alloc), alloc);
                    attachments.PushBack(a, alloc);
                }
                s.AddMember("attachments", attachments, alloc);
            }

            stepsArray.PushBack(s, alloc);
        }
        doc.AddMember("steps", stepsArray, alloc);
    }

    const fs::path outPath = fs::path(outputDir) / (tl_uuid + "-result.json");

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream f(outPath, std::ios::binary);
    f << buffer.GetString();
    f.close();
}

} // namespace systelab::gtest_allure
