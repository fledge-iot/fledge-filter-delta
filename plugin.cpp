/*
 * FogLAMP "delta" filter plugin.
 *
 * Copyright (c) 2018 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto, Mark Riddoch
 */

#include <plugin_api.h>
#include <config_category.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <iostream>
#include <filter_plugin.h>
#include <filter.h>
#include <reading_set.h>
#include <map>
#include <rapidjson/writer.h>
#include <delta_filter.h>

#define FILTER_NAME "delta"
#define DEFAULT_CONFIG "{\"plugin\" : { \"description\" : \"Delta filter plugin\", " \
                       		"\"type\" : \"string\", " \
				"\"default\" : \"" FILTER_NAME "\", \"readonly\" : \"true\" }, " \
			 "\"enable\": {\"description\": \"A switch that can be used to enable or disable execution of " \
					 "the delta filter.\", " \
				"\"type\": \"boolean\", " \
				"\"default\": \"false\", \"order\" : \"4\" }, " \
			 "\"tolerance\": {\"description\": \"A percentage difference that will be tolerated " \
					 "when determining if values are equal.\", " \
				"\"type\": \"float\", " \
				"\"default\": \"0\", \"order\" : \"1\" }, " \
			 "\"minRate\": {\"description\": \"The minimum rate at which data must be sent\", " \
				"\"type\": \"integer\", " \
				"\"default\": \"0\", \"order\" : \"2\" }, " \
			 "\"rateUnit\": {\"description\": \"The unit used to evaluate the minimum rate\", " \
				"\"type\": \"enumeration\", " \
				"\"options\" : [ \"per second\", \"per minute\", \"per hour\", \"per day\" ], " \
				"\"default\": \"per second\", \"order\" : \"3\" } " \
			"}"

using namespace std;
using namespace rapidjson;

/**
 * The Filter plugin interface
 */
extern "C" {

/**
 * The plugin information structure
 */
static PLUGIN_INFORMATION info = {
        FILTER_NAME,              // Name
        "1.4.5",                  // Version
        0,          		  // Flags
        PLUGIN_TYPE_FILTER,       // Type
        "1.0.0",                  // Interface version
	DEFAULT_CONFIG	          // Default plugin configuration
};

/**
 * Return the information about this plugin
 */
PLUGIN_INFORMATION *plugin_info()
{
	return &info;
}

/**
 * Initialise the plugin, called to get the plugin handle and setup the
 * output handle that will be passed to the output stream. The output stream
 * is merely a function pointer that is called with the output handle and
 * the new set of readings generated by the plugin.
 *     (*output)(outHandle, readings);
 * Note that the plugin may not call the output stream if the result of
 * the filtering is that no readings are to be sent onwards in the chain.
 * This allows the plugin to discard data or to buffer it for aggregation
 * with data that follows in subsequent calls
 *
 * @param config	The configuration category for the filter
 * @param outHandle	A handle that will be passed to the output stream
 * @param output	The output stream (function pointer) to which data is passed
 * @return		An opaque handle that is used in all subsequent calls to the plugin
 */
PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output)
{
	DeltaFilter *handle;
	handle = new DeltaFilter(FILTER_NAME,
				   *config,
				   outHandle,
				   output);

	return (PLUGIN_HANDLE)handle;
}

/**
 * Ingest a set of readings into the plugin for processing
 *
 * @param handle	The plugin handle returned from plugin_init
 * @param readingSet	The readings to process
 */
void plugin_ingest(PLUGIN_HANDLE *handle,
		   READINGSET *readingSet)
{
	DeltaFilter* filter = (DeltaFilter *)handle;
	if (!filter->isEnabled())
	{
		// Current filter is not active: just pass the readings set
		filter->m_func(filter->m_data, readingSet);
		return;
	}

	vector<Reading *> newReadings;
	filter->ingest(readingSet->getAllReadingsPtr(), newReadings);

	// Remove the input readingSet data
	delete (ReadingSet *)readingSet;

	// Create a new ReadingSet from new reading data
	ReadingSet *newReadingSet = new ReadingSet(&newReadings);

	// Pass newReadings to filter->m_func
	filter->m_func(filter->m_data, newReadingSet);
}

/**
 * Call the shutdown method in the plugin
 *
 * @param handle	The plugin handle, aka instance of DeltaFilter
 * @return	A JSON string with data to persist in storage service
 */
void plugin_shutdown(PLUGIN_HANDLE *handle)
{
	DeltaFilter* filter = (DeltaFilter *)handle;

	// Free resources
	delete filter;

	// Return JSON string
	return;
}

/**
 * Plugin start with plugin data from storage service
 *
 * @param    handle		The filter handle
 * @param    storedData		The stored plugin data
 *				from last run.
 */
void plugin_start(PLUGIN_HANDLE *handle,
		  const string& storedData)
{
	FogLampFilter* filter = (FogLampFilter *)handle;
}

// End of extern "C"
};
