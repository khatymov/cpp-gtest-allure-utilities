#include "TestProgramJSONBuilder.h"

#include "Model/TestProgram.h"
#include "Services/Report/ITestSuiteJSONSerializer.h"
#include "Services/System/IFileService.h"
#include "Services/System/IUUIDGeneratorService.h"


namespace systelab { namespace gtest_allure { namespace service {

	TestProgramJSONBuilder::TestProgramJSONBuilder(std::unique_ptr<ITestSuiteJSONSerializer> testSuiteJSONSerializer,
												   std::unique_ptr<IFileService> fileService,
												   std::unique_ptr<IUUIDGeneratorService> uuidGeneratorService)
		:m_testSuiteJSONSerializer(std::move(testSuiteJSONSerializer))
		,m_fileService(std::move(fileService))
		,m_uuidGeneratorService(std::move(uuidGeneratorService))
	{
	}

	void TestProgramJSONBuilder::buildJSONFiles(const model::TestProgram& testProgram) const
	{
		unsigned int nTestSuites = (unsigned int) testProgram.getTestSuitesCount();
		for (unsigned int i = 0; i < nTestSuites; i++)
		{
			const model::TestSuite& testSuite = testProgram.getTestSuite(i);
			const auto& testCases = testSuite.getTestCases();
			for (size_t testCaseIndex = 0; testCaseIndex < testCases.size(); ++testCaseIndex)
			{
				model::TestSuite singleTestCaseSuite = testSuite;
				singleTestCaseSuite.clearTestCases();
				singleTestCaseSuite.addTestCase(testCases[testCaseIndex]);
				std::string testCaseUUID = m_uuidGeneratorService->generateUUID();
				singleTestCaseSuite.setUUID(testCaseUUID);

				std::string testCaseJSONFilepath = testProgram.getOutputFolder() + "/" + testCaseUUID + "-" + "results" + ".json";
				std::string testCaseJSONContent = m_testSuiteJSONSerializer->serialize(singleTestCaseSuite);
				m_fileService->saveFile(testCaseJSONFilepath, testCaseJSONContent);
			}
		}
	}

}}}
