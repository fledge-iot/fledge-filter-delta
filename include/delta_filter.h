#ifndef _DELTA_FILTER_H
#define _DELTA_FILTER_H
/*
 * Fledge "Delta" filter plugin.
 *
 * Copyright (c) 2018 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch           
 */     
#include <filter.h>               
#include <reading_set.h>
#include <config_category.h>
#include <string>                 
#include <vector>
#include <regex>
#include <mutex>
#include <map>

/**
 * A Fledge filter that is used to filter out duplicate data in the readings stream.
 * A tolerance may be added to the detection of duplicates, this tolerance is expressed
 * as a percentage change in the reading value that can be accepted.
 * It is also possible to define a minimum rate at whch readings may be send onwards,
 * regardless of the values.
 */
class DeltaFilter : public FledgeFilter {
	public:
		DeltaFilter(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM out);
		~DeltaFilter();
		void	ingest(std::vector<Reading *> *in, std::vector<Reading *>& out);
		void	reconfigure(const std::string& newConfig);
	private:
		double		getTolerance(const std::string& asset);
		class DeltaData {
			public:
				DeltaData(Reading *);
				~DeltaData();
				bool			evaluate(Reading *, double tolerance, struct timeval rate);
				const std::string& 	getAssetName() { return m_lastSent->getAssetName(); };
			private:
				Reading		*m_lastSent;
				struct timeval	m_lastSentTime;
		};
		typedef std::map<const std::string, DeltaData *> DeltaMap;
		void 		handleConfig(const ConfigCategory& conf);
		DeltaMap	m_state;
		struct timeval	m_rate;
		std::mutex	m_configMutex;
		double		m_tolerance;
		std::map<std::string, double>
				m_tolerances;
};


#endif
