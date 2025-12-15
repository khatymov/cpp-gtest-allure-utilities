#pragma once

#include <gtest/gtest.h>
#include <string>


namespace systelab::gtest_allure {

class Allure2Listener final : public ::testing::EmptyTestEventListener
{
public:
    Allure2Listener() = default;
    ~Allure2Listener() override = default;

    void OnTestStart(const ::testing::TestInfo& testInfo) override;
    void OnTestEnd(const ::testing::TestInfo& testInfo) override;

private:
    static std::string generateUuidV4();
    static long long nowMs();
};

} // namespace systelab::gtest_allure
