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


/* TEST CASE : Look for absolute change of 100 in all datapoints */
TEST(DELTA, AbsoluteChangeAllDatapoints)
{
    PLUGIN_INFORMATION *info = plugin_info();
    ConfigCategory *config = new ConfigCategory("scale", info->config);
    ASSERT_NE(config, (ConfigCategory *)NULL);
    config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
    config->setValue("toleranceMeasure", "Absolute Value");

    ASSERT_EQ(config->itemExists("tolerance"), true);
    config->setValue("tolerance", "100");

    ASSERT_EQ(config->itemExists("processingMode"), true);
    config->setValue("processingMode", "Include full reading if all Datapoints exceed tolerance");

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
    
    dpValues = {1111, 1022};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

    dpValues = {1111, 1122};
    Reading *rdng4 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng4);

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
    ASSERT_EQ(outdp->getData().toInt(), 1122);
}

/* TEST CASE : Look for 10% change in all datapoints */
TEST(DELTA, PercentChangeAllDatapoints)
{
    PLUGIN_INFORMATION *info = plugin_info();
    ConfigCategory *config = new ConfigCategory("scale", info->config);
    ASSERT_NE(config, (ConfigCategory *)NULL);
    config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
    config->setValue("toleranceMeasure", "Percentage");

    ASSERT_EQ(config->itemExists("tolerance"), true);
    config->setValue("tolerance", "10");

    ASSERT_EQ(config->itemExists("processingMode"), true);
    config->setValue("processingMode", "Include full reading if all Datapoints exceed tolerance");

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
    
    dpValues = {1111, 1022};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

    dpValues = {1111, 1122};
    Reading *rdng4 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng4);

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
    ASSERT_EQ(outdp->getData().toInt(), 1122);
}



/* TEST CASE : Look for absolute change of 100 in all datapoints */
TEST(DELTA, AbsoluteChangeNegativeValuesAllDatapoints)
{
    PLUGIN_INFORMATION *info = plugin_info();
    ConfigCategory *config = new ConfigCategory("scale", info->config);
    ASSERT_NE(config, (ConfigCategory *)NULL);
    config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
    config->setValue("toleranceMeasure", "Absolute Value");

    ASSERT_EQ(config->itemExists("tolerance"), true);
    config->setValue("tolerance", "100");

    ASSERT_EQ(config->itemExists("processingMode"), true);
    config->setValue("processingMode", "Include full reading if all Datapoints exceed tolerance");

    config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
    ReadingSet *outReadings;
    void *handle = plugin_init(config, &outReadings, Handler);
    vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1", "dp2"};

    vector<long> dpValues = {-1000, -1000};
    Reading *rdng1 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {-1001, -1088};
    Reading *rdng2 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {-1111, -1022};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

    dpValues = {-1111, -1122};
    Reading *rdng4 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng4);

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
    ASSERT_EQ(outdp->getData().toInt(), -1000);

    outdp = points[1];
    ASSERT_STREQ(outdp->getName().c_str(), "dp2");
    ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
    ASSERT_EQ(outdp->getData().toInt(), -1000);

    out = results[1];
    ASSERT_STREQ(out->getAssetName().c_str(), "ast");
    ASSERT_EQ(out->getDatapointCount(), 2);
    points = out->getReadingData();

    ASSERT_EQ(points.size(), 2);
    
    outdp = points[0];
    ASSERT_STREQ(outdp->getName().c_str(), "dp1");
    ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
    ASSERT_EQ(outdp->getData().toInt(), -1111);

    outdp = points[1];
    ASSERT_STREQ(outdp->getName().c_str(), "dp2");
    ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
    ASSERT_EQ(outdp->getData().toInt(), -1122);
}

/* TEST CASE : Look for 10% change in all datapoints */
TEST(DELTA, PercentChangeNegativeValuesAllDatapoints)
{
    PLUGIN_INFORMATION *info = plugin_info();
    ConfigCategory *config = new ConfigCategory("scale", info->config);
    ASSERT_NE(config, (ConfigCategory *)NULL);
    config->setItemsValueFromDefault();

    ASSERT_EQ(config->itemExists("toleranceMeasure"), true);
    config->setValue("toleranceMeasure", "Percentage");

    ASSERT_EQ(config->itemExists("tolerance"), true);
    config->setValue("tolerance", "10");

    ASSERT_EQ(config->itemExists("processingMode"), true);
    config->setValue("processingMode", "Include full reading if all Datapoints exceed tolerance");

    config->setValue("enable", "true");

    Logger::getLogger()->setMinLevel("debug");
    
    ReadingSet *outReadings;
    void *handle = plugin_init(config, &outReadings, Handler);
    vector<Reading *> *readings = new vector<Reading *>;

    vector<string> dpNames = {"dp1", "dp2"};

    vector<long> dpValues = {-1000, -1000};
    Reading *rdng1 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng1);

    dpValues = {-1001, -1088};
    Reading *rdng2 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng2);
    
    dpValues = {-1111, -1022};
    Reading *rdng3 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng3);

    dpValues = {-1111, -1122};
    Reading *rdng4 = createReadingWithLongDatapoints("ast", dpNames, dpValues);
    readings->emplace_back(rdng4);

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
    ASSERT_EQ(outdp->getData().toInt(), -1000);

    outdp = points[1];
    ASSERT_STREQ(outdp->getName().c_str(), "dp2");
    ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
    ASSERT_EQ(outdp->getData().toInt(), -1000);

    out = results[1];
    ASSERT_STREQ(out->getAssetName().c_str(), "ast");
    ASSERT_EQ(out->getDatapointCount(), 2);
    points = out->getReadingData();

    ASSERT_EQ(points.size(), 2);
    
    outdp = points[0];
    ASSERT_STREQ(outdp->getName().c_str(), "dp1");
    ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
    ASSERT_EQ(outdp->getData().toInt(), -1111);

    outdp = points[1];
    ASSERT_STREQ(outdp->getName().c_str(), "dp2");
    ASSERT_EQ(outdp->getData().getType(), DatapointValue::T_INTEGER);
    ASSERT_EQ(outdp->getData().toInt(), -1122);
}
