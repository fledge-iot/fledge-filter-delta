#include <gtest/gtest.h>
#include <plugin_api.h>
#include <config_category.h>
#include <filter_plugin.h>
#include <filter.h>
#include <string.h>
#include <string>
#include <rapidjson/document.h>
#include <reading.h>
#include <reading_set.h>
#include "helper.h"

using namespace std;
using namespace rapidjson;


extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	void plugin_ingest(void *handle,
                   READINGSET *readingSet);
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);

	extern void Handler(void *handle, READINGSET *readings);
};


TEST(DELTA, AbsoluteChangeAnyDatapoint1)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "100");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1", "dp2"};

    vector<long> dpValues = {1000, 1000};
    Reading *rdng1 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {1010, 1051};
    Reading *rdng2 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {1105, 1050};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);


	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 2);
	vector<Datapoint *> points = out->getReadingData();
	ASSERT_EQ(points.size(), 2);
	Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1000);
    outdp = points[1];
	ASSERT_STREQ(outdp->getName().c_str(), "dp2");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1000);

	out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 2);
	points = out->getReadingData();
	ASSERT_EQ(points.size(), 2);
	outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1105);
    outdp = points[1];
	ASSERT_STREQ(outdp->getName().c_str(), "dp2");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1050);
}


TEST(DELTA, AbsoluteChangeAnyDatapoint2)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "100");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1", "dp2"};

    vector<long> dpValues = {1000, 1000};
    Reading *rdng1 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {1001, 1088};
    Reading *rdng2 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {1111, 1002};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 2);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 2);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1000);

    outdp = points[1];
	ASSERT_STREQ(outdp->getName().c_str(), "dp2");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1000);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 2);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 2);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1111);

    outdp = points[1];
	ASSERT_STREQ(outdp->getName().c_str(), "dp2");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1002);
}

TEST(DELTA, AbsoluteChangeAnyDatapointSmallValues1)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.0001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {1.9999};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {2.0000};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {2.0001};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 1.9999);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2.0001);
}

TEST(DELTA, AbsoluteChangeAnyDatapointSmallValues2)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {1.999999};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {2.000000};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {2.000001};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 1.999999);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2.000001);
}

TEST(DELTA, AbsoluteChangeAnyDatapointSmallValues3)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {0.002399};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {0.002400};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {0.002401};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 0.002399);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 0.002401);
}

TEST(DELTA, AbsoluteChangeAnyDatapointSmallValues4)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {0.000002399};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {0.000002400};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {0.000002401};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 0.000002399);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 0.000002401);
}


TEST(DELTA, AbsoluteChangeAnyDatapointLargeValues1)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {1133.999999};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {1134.000000};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {1134.000001};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 1133.999999);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 1134.000001);
}


TEST(DELTA, AbsoluteChangeAnyDatapointLargeValues2)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {2000000.788899};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {2000000.788900};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {2000000.788901};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2000000.788899);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2000000.788901);
}


TEST(DELTA, AbsoluteChangeAnyDatapointLargeValues3)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {2000000000.788899};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {2000000000.788900};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {2000000000.788901};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2000000000.788899);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2000000000.788901);
}

TEST(DELTA, AbsoluteChangeAnyDatapointLargeValues4)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "absolute value");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "0.000001");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1"};

    vector<double> dpValues = {2000000000.788899};
    Reading *rdng1 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {2000000000.788905};
    Reading *rdng2 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);

    dpValues = {2000000000.7889055};
    Reading *rdng3 = createReadingWithDoubleDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2000000000.788899);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 1);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 1);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_FLOAT);
	ASSERT_EQ(outdp->getData().toDouble(), 2000000000.788905);
}


TEST(DELTA, PercentChangeAnyDatapoint)
{
	PLUGIN_INFORMATION *info = plugin_info();
	ConfigCategory *config = new ConfigCategory("scale", info->config);
	ASSERT_NE(config, (ConfigCategory *)NULL);
	config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
	config->setValue("toleranceMeasure", "percentage");

	ASSERT_EQ(config->itemExists("tolerance"), true);
	config->setValue("tolerance", "10");

    ASSERT_EQ(config->itemExists("processingMode"), true);
	config->setValue("processingMode", "Include full reading if any Datapoint exceeds tolerance");

	config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
	ReadingSet *outReadings;
	void *handle = plugin_init(config, &outReadings, Handler);
	vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1", "dp2"};

    vector<long> dpValues = {1000, 1000};
    Reading *rdng1 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {1001, 1088};
    Reading *rdng2 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {1111, 1002};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

	ReadingSet *readingSet = new ReadingSet(readings);
	plugin_ingest(handle, (READINGSET *)readingSet);

	vector<Reading *>results = outReadings->getAllReadings();
	ASSERT_EQ(results.size(), 2);

	Reading *out = results[0];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 2);
	vector<Datapoint *> points = out->getReadingData();

	ASSERT_EQ(points.size(), 2);
	
    Datapoint *outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1000);

    outdp = points[1];
	ASSERT_STREQ(outdp->getName().c_str(), "dp2");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1000);

    out = results[1];
	ASSERT_STREQ(out->getAssetName().c_str(), "ast");
	ASSERT_EQ(out->getDatapointCount(), 2);
	points = out->getReadingData();

	ASSERT_EQ(points.size(), 2);
	
    outdp = points[0];
	ASSERT_STREQ(outdp->getName().c_str(), "dp1");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1111);

    outdp = points[1];
	ASSERT_STREQ(outdp->getName().c_str(), "dp2");
	ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
	ASSERT_EQ(outdp->getData().toInt(), 1002);
}
