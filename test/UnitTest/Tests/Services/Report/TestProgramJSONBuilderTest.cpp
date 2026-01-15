#include "stdafx.h"
#include "GTestAllureUtilities/Services/Report/TestProgramJSONBuilder.h"

#include "Model/TestSuite.h"
#include "Model/TestProgram.h"

#include "TestUtilities/Mocks/Services/System/MockFileService.h"
#include "TestUtilities/Mocks/Services/Report/MockTestSuiteJSONSerializer.h"
#include "TestUtilities/Mocks/Services/System/MockUUIDGeneratorService.h"


using namespace testing;
using namespace systelab::gtest_allure;
using namespace systelab::gtest_allure::test_utility;

namespace systelab { namespace gtest_allure_utilities { namespace unit_test {

	class TestProgramJSONBuilderTest : public testing::Test
	{
		void SetUp()
		{
			m_testProgram = buildTestProgram();
			auto testSuiteJSONSerializer = buildTestSuiteJSONSerializer();
			auto fileService = buildFileService();
			auto uuidGeneratorService = buildUUIDGeneratorService();

			m_service = std::make_unique<service::TestProgramJSONBuilder>(std::move(testSuiteJSONSerializer),
																		 std::move(fileService),
																		 std::move(uuidGeneratorService));
		}

		std::unique_ptr<model::TestProgram> buildTestProgram()
		{
			m_testProgramName = "MyTestProgram";
			m_outputFolder = "TestProgramJSONBuilderTest/Reports";
			m_testSuiteUUIDs = { "UUID1", "UUID2", "UUID3" };
			m_testSuiteTestCaseCount = { 2, 1, 0 };

			auto testProgram = std::make_unique<model::TestProgram>();
			testProgram->setName(m_testProgramName);
			testProgram->setOutputFolder(m_outputFolder);

			for (size_t suiteIndex = 0; suiteIndex < m_testSuiteUUIDs.size(); ++suiteIndex)
			{
				model::TestSuite testSuite;
				testSuite.setUUID(m_testSuiteUUIDs[suiteIndex]);

				for (unsigned int testCaseIndex = 0; testCaseIndex < m_testSuiteTestCaseCount[suiteIndex]; ++testCaseIndex)
				{
					model::TestCase testCase;
					testCase.setName("TestCase_" + std::to_string(suiteIndex) + "_" + std::to_string(testCaseIndex));
					testSuite.addTestCase(testCase);
				}

				testProgram->addTestSuite(testSuite);
			}

			return testProgram;
		}

		std::unique_ptr<service::ITestSuiteJSONSerializer> buildTestSuiteJSONSerializer()
		{
			auto testSuiteJSONSerializer = std::make_unique<MockTestSuiteJSONSerializer>();
			m_testSuiteJSONSerializer = testSuiteJSONSerializer.get();

			ON_CALL(*m_testSuiteJSONSerializer, serialize(_)).WillByDefault(Invoke(
				[](const model::TestSuite& testSuite) -> std::string
				{
					return std::string("Serialized") + testSuite.getUUID();
				}
			));

			return testSuiteJSONSerializer;
		}

		std::unique_ptr<service::IFileService> buildFileService()
		{
			auto fileService = std::make_unique<MockFileService>();
			m_fileService = fileService.get();
			return fileService;
		}

		std::unique_ptr<service::IUUIDGeneratorService> buildUUIDGeneratorService()
		{
			auto uuidGeneratorService = std::make_unique<MockUUIDGeneratorService>();
			m_uuidGeneratorService = uuidGeneratorService.get();
			return uuidGeneratorService;
		}

	protected:
		std::unique_ptr<service::TestProgramJSONBuilder> m_service;
		MockTestSuiteJSONSerializer* m_testSuiteJSONSerializer;
		MockFileService* m_fileService;
		MockUUIDGeneratorService* m_uuidGeneratorService;

		std::unique_ptr<model::TestProgram> m_testProgram;
		std::string m_testProgramName;
		std::string m_outputFolder;
		std::vector <std::string> m_testSuiteUUIDs;
		std::vector<unsigned int> m_testSuiteTestCaseCount;
		std::vector<std::string> m_expectedTestCaseUUIDs = { "CaseUUID-1", "CaseUUID-2", "CaseUUID-3" };
	};


	TEST_F(TestProgramJSONBuilderTest, testBuildJSONFilesSavesAFileForEachTestCase)
	{
		size_t expectedFileCount = 0;
		for (auto count : m_testSuiteTestCaseCount)
		{
			expectedFileCount += count;
		}
		ASSERT_EQ(m_expectedTestCaseUUIDs.size(), expectedFileCount);

		::testing::InSequence seq;
		for (const auto& testCaseUUID : m_expectedTestCaseUUIDs)
		{
			EXPECT_CALL(*m_uuidGeneratorService, generateUUID())
				.WillOnce(Return(testCaseUUID));

			std::string expectedFilepath = m_outputFolder + "/" + testCaseUUID + "-" + "results" + ".json";
			std::string expectedFileContent = "Serialized" + testCaseUUID;
			EXPECT_CALL(*m_fileService, saveFile(expectedFilepath, expectedFileContent));
		}

		m_service->buildJSONFiles(*m_testProgram);
	}

	TEST_F(TestProgramJSONBuilderTest, testBuildJSONFilesDoesNotSaveAFileWhenSuiteHasNoTestCases)
	{
		EXPECT_CALL(*m_fileService, saveFile(_, _)).Times(0);
		EXPECT_CALL(*m_uuidGeneratorService, generateUUID()).Times(0);

		model::TestProgram emptyTestProgram;
		m_service->buildJSONFiles(emptyTestProgram);
	}

}}}
