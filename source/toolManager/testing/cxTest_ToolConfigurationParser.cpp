#include "cxTest_ToolConfigurationParser.h"

#include "cxDataLocations.h"
#include "cxToolConfigurationParser.h"
#include <QFile>

void TestToolConfigurationParser::setUp()
{
  mXmlFilePath = cx::DataLocations::getConfigPath()+QString("/tool/Lab/POLARIS_07-198-0838_SW_Pointer_02_AND_02-206-00913_SW_PasRef_01.xml");

  std::string xmlFilePath = mXmlFilePath.toStdString();
  mConfigurationParser = new cx::ToolConfigurationParser(xmlFilePath);
}

void TestToolConfigurationParser::tearDown()
{
  delete mConfigurationParser;
}

void TestToolConfigurationParser::testConstructor()
{
  CPPUNIT_ASSERT_MESSAGE("Xml file does not exist.", QFile::exists(mXmlFilePath));
  CPPUNIT_ASSERT_MESSAGE("Could not make a new configuration parser.", mConfigurationParser);
}

void TestToolConfigurationParser::testGetTracker()
{
  cx::TrackerPtr tracker = mConfigurationParser->getTracker();
  CPPUNIT_ASSERT_MESSAGE("Tracker is null.", tracker);
  CPPUNIT_ASSERT_MESSAGE("Tracker is not of type Polaris.", tracker->getType() == cx::Tracker::TRACKER_POLARIS);
}

void TestToolConfigurationParser::testGetConfiguredTools()
{
  cx::ToolMapPtr toolmap = mConfigurationParser->getConfiguredTools();
  CPPUNIT_ASSERT_MESSAGE("Not the right number of tools.", toolmap->size() == 2);
}
