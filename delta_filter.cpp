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
    bool sendOrig;
    Reading* readingToSend = nullptr;
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
		else if (deltaIt->second->evaluate(reading, m_toleranceMeasure, getTolerance(reading->getAssetName()), m_rate, 
                                            m_processingMode, sendOrig, readingToSend))
		{
            // evaluate's return value indicates whether a reading needs to be sent onwards
            if(sendOrig)
            {
                // out.push_back(move(*it)); // TODO: check if it is possible to just move the reading ptr to out vector
                // *it = nullptr;
                out.push_back(new Reading(*reading));
                delete *it;
            }
            else
            {
                if(readingToSend)
                    out.push_back(readingToSend); // readingToSend is allocated on heap
                delete *it;
            }
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
 * Check whether tolerance is exceeded given old and new DatapointValue objects
 *
 * @param oValue		old DatapointValue
 * @param nValue		new DatapointValue
 * @param toleranceMeasure	measure of tolerance: percentage or absolute value
 * @param tolerance		tolerance percentage or absolute value
 * @param change		returns absolute percentage or absolute change in Datapoint values
 * @return bool         whether tolerance was exceeded
 */
bool checkToleranceExceeded(const string &dpName, const DatapointValue& oValue, const DatapointValue& nValue, 
                                DeltaFilter::ToleranceMeasure toleranceMeasure, double tolerance, 
                                double &change)
{
    if ( (oValue.getType() == DatapointValue::T_INTEGER || oValue.getType() == DatapointValue::T_FLOAT) &&  
            (nValue.getType() == DatapointValue::T_INTEGER || nValue.getType() == DatapointValue::T_FLOAT) )
    {
        double prevValue = (oValue.getType() == DatapointValue::T_INTEGER) ? (double)oValue.toInt() : oValue.toDouble();
        double newValue = (nValue.getType() == DatapointValue::T_INTEGER) ? (double)nValue.toInt() : nValue.toDouble();

        change = fabs(newValue - prevValue);
        if(toleranceMeasure == DeltaFilter::ToleranceMeasure::PERCENTAGE)
            change = fabs((change * 100.0) / prevValue);

        Logger::getLogger()->debug("dpName=%s, prevValue=%.20lf, newValue=%.20lf, toleranceMeasure=%d, tolerance=%.20lf", 
                                        dpName.c_str(), prevValue, newValue, toleranceMeasure, tolerance);

        // std::numeric_limits<double>::epsilon() = 0.00000000000000022204
        double adjustedTolerance = tolerance + std::fmax(std::fabs(prevValue), std::fabs(newValue)) * std::numeric_limits<double>::epsilon();
        
        Logger::getLogger()->debug("adjustedTolerance = %.20lf, change = %.20lf", adjustedTolerance, change);
        return change > adjustedTolerance;
    }
    else if (oValue.getType() == DatapointValue::T_STRING && nValue.getType() == DatapointValue::T_STRING)
    {
        if(nValue.toString().compare(oValue.toString()) != 0)
        {
            Logger::getLogger()->debug("dpName=%s, STRING value change: prevValue=%s, newValue=%s", 
                                        dpName.c_str(), oValue.toString().c_str(), nValue.toString().c_str());
            return true;
        }
        else
        {
            return false;
        }
    }
    
    return false;
}

/**
 * Evaluate a reading to determine if it needs to be sent.
 * The conditions that cause it to be sent are:
 *
 *	1. The minimum rate, if set requires a value to be sent
 *
 *	2. The difference between new value and the last sent value for a 
 *  datapoint exceeds the configured tolerance and processing mode 
 *  is suitable w.r.t. the set of changed datapoints.
 *
 * If the reading is sent, a copy of sent datapoints is stored and held
 * in a reading object in the DeltaData class object.
 *
 * Note, the time used when considering rate is not the current time
 * but the time in the readings as the rate referes to the reading rate
 * and not real time. The two may be different because of buffering
 * within the services that make up a Fledge instance.
 *
 * @param candidate	        The candidate reading
 * @param toleranceMeasure	Whether tolerance is specified in absolute terms or 
 *                          as a percentage
 * @param tolerance	        Tolerance value in absolute terms or as a percentage 
 *                          (as per toleranceMeasure)
 * @param rate	            Time interval after which a reading must be sent even 
 *                          if tolerance is not exceeded
 * @param processingMode	Output reading processing mode
 * @param sendOrig	        Whether to send the original reading
 * @param readingToSend	    Reading to send after some DPs have been removed from 
 *                          the original reading
 * @return                  Whether a reading should be sent out by the filter
 */
bool
DeltaFilter::DeltaData::evaluate(Reading *candidate,
                                    ToleranceMeasure toleranceMeasure,
                                    double tolerance,
                                    struct timeval rate,
                                    DeltaFilter::ProcessingMode processingMode,
                                    bool &sendOrig,
                                    Reading* &readingToSend)
{
bool    maxPeriodElapsed = false;
struct timeval	now, res;

    Logger::getLogger()->debug("INPUT READING: '%s' ", candidate->toJSON().c_str());

	if (rate.tv_sec != 0 || rate.tv_usec != 0)
	{
		candidate->getUserTimestamp(&now);
		timeradd(&m_lastSentTime, &rate, &res);
		if (timercmp(&now, &res, >))
		{
			maxPeriodElapsed = true;
            // can skip below for loop completely in this case
		}
	}

	// Get a reading DataPoint
	const vector<Datapoint *>& oDataPoints = m_lastSent->getReadingData();
	const vector<Datapoint *>& nDataPoints = candidate->getReadingData();

	unordered_set<string> changedDPs;

	// Iterate the datapoints of NEW reading
	for (vector<Datapoint *>::const_iterator nIt = nDataPoints.begin();
						 nIt != nDataPoints.end();
						 ++nIt)
	{
	        // Get the reference to a DataPointValue
		const DatapointValue& nValue = (*nIt)->getData();

		bool dpFound = false;

		// Iterate the datapoints of last reading sent
		for (vector<Datapoint *>::const_iterator oIt = oDataPoints.begin();
							 oIt != oDataPoints.end();
							 ++oIt)
		{
			if ((*nIt)->getName() != (*oIt)->getName())
			{
				// Different name, continue
				continue;
			}

			dpFound  = true;

			// Get the reference to DataPointValue
			const DatapointValue& oValue = (*oIt)->getData();

			double change;

			// Same datapoint name: check type
			if (oValue.getType() != nValue.getType())
			{
				// Numerical type
				if ( (oValue.getType() == DatapointValue::T_INTEGER || oValue.getType() == DatapointValue::T_FLOAT) &&  
						(nValue.getType() == DatapointValue::T_INTEGER || nValue.getType() == DatapointValue::T_FLOAT) )
				{
					bool toleranceExceeded = checkToleranceExceeded((*nIt)->getName(), oValue, nValue, toleranceMeasure, tolerance, change);

					if (toleranceExceeded)
					{
                        Logger::getLogger()->debug("Datapoint %s has %lf %schange", (*nIt)->getName().c_str(), change,
                                                    (toleranceMeasure == ToleranceMeasure::PERCENTAGE)? "% " : "");
                        changedDPs.emplace((*nIt)->getName());
					}
				}
				else
				{
					Logger::getLogger()->warn("Incompatible change in type of datapoint %s",
								(*nIt)->getName().c_str());
				}
			}
			else // DPV type hasn't changed
			{
				switch(nValue.getType())
				{
                    case DatapointValue::T_INTEGER:
                    case DatapointValue::T_FLOAT:
                        {
                            bool toleranceExceeded = checkToleranceExceeded((*nIt)->getName(), oValue, nValue, toleranceMeasure, tolerance, change);
                            if (toleranceExceeded)
                            {
                                Logger::getLogger()->debug("Datapoint %s has %lf %schange", (*nIt)->getName().c_str(), change,
                                                            (toleranceMeasure == ToleranceMeasure::PERCENTAGE)? "% " : "");
                                changedDPs.emplace((*nIt)->getName());
                            }
                        }
                        break;

                    case DatapointValue::T_STRING:
                        {
                            bool toleranceExceeded = checkToleranceExceeded((*nIt)->getName(), oValue, nValue, toleranceMeasure, tolerance, change);
                            if (toleranceExceeded)
                            {
                                Logger::getLogger()->debug("Datapoint %s of STRING type has changed from '%s' to '%s'", 
                                                            (*nIt)->getName().c_str(), oValue.toString().c_str(), nValue.toString().c_str());
                                changedDPs.emplace((*nIt)->getName());
                            }
                        }
                        break;

                    case DatapointValue::T_FLOAT_ARRAY:
                        // T_FLOAT_ARRAY not supported right now
                    
                    default:
                        break;
                }
            }
        }
        if(!dpFound)
        {
            Logger::getLogger()->debug("Datapoint %s seen for the first time", (*nIt)->getName().c_str());
            changedDPs.emplace((*nIt)->getName());
        }
    }

    Logger::getLogger()->debug("processingMode=%d, changedDPs.size()=%d, nDataPoints.size()=%d", 
                                processingMode, changedDPs.size(), nDataPoints.size());

    for(const auto & k : changedDPs)
        Logger::getLogger()->debug("changedDPs[i]=%s", k.c_str());

    // Act according to processingMode config. Send current reading if:
    // 1. Long enough time has elapsed to compulsarily send a reading 
    // 2. Processing mode is ANY_DATAPOINT_MATCHES and atleast one DP has changed
    // 3. Processing mode is ALL_DATAPOINTS_MATCH and all DPs have changed
    // 4. Processing mode is ONLY_CHANGED_DATAPOINTS but all DPs have changed, so original reading can be forwarded as such
    // Can combine condition 3 & 4 with just "changedDPs.size() == nDataPoints.size()", but retaining for better clarity
	if ( maxPeriodElapsed ||
            (processingMode == ProcessingMode::ANY_DATAPOINT_MATCHES && !changedDPs.empty()) ||
            (processingMode == ProcessingMode::ALL_DATAPOINTS_MATCH && changedDPs.size() == nDataPoints.size()) ||
            (processingMode == ProcessingMode::ONLY_CHANGED_DATAPOINTS && changedDPs.size() == nDataPoints.size()))
	{
        // Send current reading out
        sendOrig = true;
        readingToSend = nullptr;

		// Update new values of DPs in m_lastSent
        for(const auto &dp : candidate->getReadingData())
        {
            string dpName = dp->getName();
            if(m_lastSent->getDatapoint(dpName))
                m_lastSent->removeDatapoint(dpName);
            m_lastSent->addDatapoint(new Datapoint(*candidate->getDatapoint(dpName)));
            Logger::getLogger()->debug("FORWARDING FULL READING: Updated m_lastSent: DP '%s' with value '%s'", 
                                            dpName.c_str(), m_lastSent->getDatapoint(dpName)->toJSONProperty().c_str());
        }
        
        Logger::getLogger()->debug("SENT READING: candidate=%s", candidate->toJSON().c_str());
        Logger::getLogger()->debug("UPDATED REFERENCE: m_lastSent=%s", m_lastSent->toJSON().c_str());

		candidate->getUserTimestamp(&m_lastSentTime);
        return true;
	}
    else if(processingMode == ProcessingMode::ONLY_CHANGED_DATAPOINTS && !changedDPs.empty())
    {
        // Need to maintain last sent values of unchanged DPs and new values of changed DPs being sent now

        sendOrig = false;
        readingToSend = new Reading(*candidate);

        // remove unchanged DPs from readingToSend and update/add changed DPs in m_lastSent
        for(const auto &dp : candidate->getReadingData())
        {
            string dpName = dp->getName();
            if(changedDPs.count(dpName) == 0)
            {
                Logger::getLogger()->debug("ONLY_CHANGED_DATAPOINTS: removing unchanged DP '%s' ", dpName.c_str());
                readingToSend->removeDatapoint(dpName);
            }
            else
            {
                // Update this changed DP's value in m_lastSent
                if(m_lastSent->getDatapoint(dpName))
                    m_lastSent->removeDatapoint(dpName);
                m_lastSent->addDatapoint(new Datapoint(*candidate->getDatapoint(dpName)));
                Logger::getLogger()->debug("ONLY_CHANGED_DATAPOINTS: Updated m_lastSent: DP '%s' with value '%s'", 
                                                dpName.c_str(), m_lastSent->getDatapoint(dpName)->toJSONProperty().c_str());
            }
        }
        Logger::getLogger()->debug("SENT READING: readingToSend=%s", readingToSend->toJSON().c_str());
        Logger::getLogger()->debug("UPDATED REFERENCE: m_lastSent=%s", m_lastSent->toJSON().c_str());

        candidate->getUserTimestamp(&m_lastSentTime);
        return true;
    }

    sendOrig = false;
    readingToSend = nullptr;
    
	return false;
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
 *  toleranceMeasure	Whether tolerance is specified as percentage/absolute value
 *	tolerance	The tolerance value/percentage when comparing reading data
 *	processingMode	Reading processing mode
 *	minRate		The minimum rate at which readings should be sent
 *	rateUnit	The units in which minRate is define (per second, minute, hour or day)
 *
 * @param config	The configuration category for the filter
 */
void
DeltaFilter::handleConfig(const ConfigCategory& config)
{
    string toleranceMeasure = config.getValue("toleranceMeasure");
    m_toleranceMeasure = (toleranceMeasure.compare("Percentage")==0) ? 
                            ToleranceMeasure::PERCENTAGE : 
                            ToleranceMeasure::ABSOLUTE_VALUE;
	
    m_tolerance = strtod(config.getValue("tolerance").c_str(), NULL);
    Logger::getLogger()->info("handleConfig(): toleranceStr='%s', m_tolerance=%.20lf", 
                                config.getValue("tolerance").c_str(), m_tolerance);
    
    string processingMode = config.getValue("processingMode");
    Logger::getLogger()->info("handleConfig(): processingMode='%s' = %d", 
                                processingMode.c_str(), parseProcessingMode(processingMode));
    m_processingMode = parseProcessingMode(processingMode);
    if(m_processingMode == DeltaFilter::INVALID_MODE)
    {
        Logger::getLogger()->warn("Delta filter: Invalid Reading processing mode '%s'; changing to default '%s",
                                        processingMode.c_str(), "Include full reading if any Datapoint exceeds tolerance");
        m_processingMode = DeltaFilter::ANY_DATAPOINT_MATCHES;
    }

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
