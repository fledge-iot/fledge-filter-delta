/*
 * Fledge "delta" filter plugin.
 *
 * Copyright (c) 2018 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Massimiliano Pinto
 */

#include <delta_filter.h>
#include <config_category.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string>
#include <iostream>
#include <reading_set.h>
#include <vector>
#include <map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

using namespace std;
using namespace rapidjson;

/**
 * Constructor for the Delta Filter. Calls the base FledgeFilter constructor
 * to setup the "plumbing" for the fitlers.
 * Also handles the configuration of the plugin.
 *
 * @param fitlerName	The name of the plugin
 * @param filterConfig	The configuration of this filter
 * @param outHandle	The output handle, class of the next plugin in chain
 * @param out		The function to call
 */
DeltaFilter::DeltaFilter(const std::string& filterName,
                               ConfigCategory& filterConfig,
                               OUTPUT_HANDLE *outHandle,
                               OUTPUT_STREAM out) :
                                  FledgeFilter(filterName, filterConfig,
                                                outHandle, out)
{
        handleConfig(filterConfig);                   
}

/**
 * Destructor for the filter plugin class.
 * Cleans up all the data related to the filter
 */
DeltaFilter::~DeltaFilter()
{
}

/**
 * Filter ingest method, the core of the filter.
 *
 * The ingest method is responsible for takign a set of readings in,
 * applying the rules of the delta file and creating a set of outgoing
 * readings which are the delta's.
 *
 * The incoming readings that are not forwarded will be deleted, if a reading
 * is forwarded then it will put put in the out vector and not freed.
 *
 * @param readings	The incoming readings from the previous filter in the pipeline
 * @param out		The outgoing set of readings, these are the delta values
 */
void DeltaFilter::ingest(vector<Reading *> *readings, vector<Reading *>& out)
{

	// Iterate over the readings
	for (vector<Reading *>::const_iterator it = readings->begin();
					it != readings->end(); it++)
	{
		Reading *reading = *it;
		lock_guard<mutex> guard(m_configMutex); // Protect against reconfiguration
		// Find this asset in the map of values we hold	
		DeltaMap::iterator deltaIt = m_state.find(reading->getAssetName());
		if (deltaIt == m_state.end())
		{
			DeltaData *delta = new DeltaData(reading);
			m_state.insert(pair<string, DeltaData *>(delta->getAssetName(), delta));
			out.push_back(*it);
		}
		else if (deltaIt->second->evaluate(reading, getTolerance(reading->getAssetName()), m_rate))
		{
			// This reading needs to be sent onwards
			out.push_back(*it);
		}
		else
		{
			delete *it;
		}
	}
	readings->clear();
}

/**
 * Constructor for the DataData class. This is a private class within
 * the filter class and is used to store the data about a particular
 * asset.
 *
 * @param reading	The reading this delta data related to
 * @param tolerance	The percentage tolerance configured for the filter
 * @param rate		The required minimum rate, expressed as time between sends
 */
DeltaFilter::DeltaData::DeltaData(Reading *reading) :
	m_lastSent(new Reading(*reading))
{
	gettimeofday(&m_lastSentTime, NULL);
}

/**
 * The destructor for the delta data. Sim,le clean up the dynamically
 * allocated data.
 */
DeltaFilter::DeltaData::~DeltaData()
{
	delete m_lastSent;
}

/**
 * Evaluate a reading to determine if it needs to be sent
 * The conditions that cause it to be sent are:
 *
 *	The minimum rate, if set requires a value to be sent
 *
 *	The difference between this value and the last value send
 *	exceeds the tolerance percentage.
 *
 * If the reading is sent a copy is made and held in the DeltaData
 * class.
 *
 * When an asset has multiple data points if any one data point change
 * exceeds the tolerance then the entire reading is sent.
 *
 * Note, the time used when considering rate is not the current time
 * but the tiem in the readings as the rate referes to the reading rate
 * and not real time. The two will be different because of buffering
 * without the services that make up a Fledge instance.
 *
 * @param candidiate	The candidiate reading
 */
bool
DeltaFilter::DeltaData::evaluate(Reading *candidiate,
				  double tolerance,
				  struct timeval rate)
{
bool		sendThis = false;
struct timeval	now, res;

	if (rate.tv_sec != 0 || rate.tv_usec != 0)
	{
		candidiate->getUserTimestamp(&now);
		timeradd(&m_lastSentTime, &rate, &res);
		if (timercmp(&now, &res, >))
		{
			sendThis = true;
		}
	}

	// Get a reading DataPoint
	const vector<Datapoint *>& oDataPoints = m_lastSent->getReadingData();
	const vector<Datapoint *>& nDataPoints = candidiate->getReadingData();

	// Iterate the datapoints of NEW reading
	for (vector<Datapoint *>::const_iterator nIt = nDataPoints.begin();
						 nIt != nDataPoints.end();
						 ++nIt)
	{
	        // Get the reference to a DataPointValue
		const DatapointValue& nValue = (*nIt)->getData();

		// Iterate the datapoints of last reading sent
		for (vector<Datapoint *>::const_iterator oIt = oDataPoints.begin();
							 oIt != oDataPoints.end();
							 ++oIt)
		{
			if ((*nIt)->getName().compare((*oIt)->getName()) != 0)
			{
				// Different name, continue
				continue;
			}
			
	                // Get the reference to a DataPointValue
                        const DatapointValue& oValue = (*oIt)->getData();

			// Same datapoint name: check type
			if (oValue.getType() != nValue.getType())
			{
				// Different type
				continue;
			}

			switch(nValue.getType())
			{
				case DatapointValue::T_INTEGER:
					if (abs(nValue.toInt() - oValue.toInt())
						> (tolerance * abs(oValue.toInt())) / 100)
					{
						sendThis = true;
					}
					break;
				case DatapointValue::T_FLOAT:
					if (fabs(nValue.toDouble() - oValue.toDouble())
						> (tolerance * fabs(oValue.toDouble())) / 100)
					{
						sendThis = true;
					}
					break;
				case DatapointValue::T_STRING:
					if (nValue.toString().compare(oValue.toString()))
					{
						sendThis = true;
					}
					break;
				case DatapointValue::T_FLOAT_ARRAY:
					// T_FLOAT_ARRAY not supported right now
				default:
					break;
			}
			if (sendThis)
			{
				break;
			}
		}
		if (sendThis)
		{
			break;
		}
	}

	if (sendThis)
	{
		delete m_lastSent;
		m_lastSent = new Reading(*candidiate);
		candidiate->getUserTimestamp(&m_lastSentTime);
	}

	return sendThis;
}

/**
 * Handle the reconfiguration of this filter
 */
void
DeltaFilter::reconfigure(const string& newConfig)
{
	lock_guard<mutex> guard(m_configMutex);
	setConfig(newConfig);
        handleConfig(m_config);
}

/**
 * Return the tolarance to use for the given asset name
 *
 * @param asset		The name of the asset
 * @return The tolarance to use
 */
double
DeltaFilter::getTolerance(const std::string& asset)
{
	auto t = m_tolerances.find(asset);
	if (t != m_tolerances.end())
	{
		return t->second;
	}
	return m_tolerance;
}


/**
 * Handle the configuration of the delta filter
 *
 * Configuration items
 *	tolerance	The percentage tolerance when comparing reading data
 *	minRate		The minimum rate at which readings should be sent
 *	rateUnit	The units in which minRate is define (per second, minute, hour or day)
 *
 * @param config	The configuration category for the filter
 */
void
DeltaFilter::handleConfig(const ConfigCategory& config)
{
	m_tolerance = strtof(config.getValue("tolerance").c_str(), NULL);
	int minRate = strtol(config.getValue("minRate").c_str(), NULL, 10);
	string unit = config.getValue("rateUnit");
	if (minRate == 0)
	{
		m_rate.tv_sec = 0;
		m_rate.tv_usec = 0;
	}
	else if (unit.compare("per second") == 0)
	{
		m_rate.tv_sec = 0;
		m_rate.tv_usec = 1000000 / minRate;
	}
	else if (unit.compare("per minute") == 0)
	{
		m_rate.tv_sec = 60 / minRate;
		m_rate.tv_usec = 0;
	}
	else if (unit.compare("per hour") == 0)
	{
		m_rate.tv_sec = 3600 / minRate;
		m_rate.tv_usec = 0;
	}
	else if (unit.compare("per day") == 0)
	{
		m_rate.tv_sec = (24 * 60 * 60) / minRate;
		m_rate.tv_usec = 0;
	}
	m_tolerances.clear();
	if (config.itemExists("overrides"))
	{
		Document doc;
		ParseResult res = doc.Parse(config.getValue("overrides").c_str());
		for (auto &t : doc.GetObject())
		{
			m_tolerances.insert(pair<string, double>(t.name.GetString(), t.value.GetDouble()));
		}
	}
}
