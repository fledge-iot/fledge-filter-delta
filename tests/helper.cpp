#include "helper.h"

using namespace std;

/**
 * Add string datapoint to reading
 *
 * @param rdng  Reading object to update
 * @param dpName	Datapoint name
 * @param dpValue	Datapoint value
 */
void addStringTypeDatapoint(Reading *rdng, const string &dpName, const string &dpValue)
{
    if(!rdng)
        return;
    
    DatapointValue dpv(dpValue);
    Datapoint *dp = new Datapoint(dpName, dpv);
    rdng->addDatapoint(dp);
}

/**
 * Create a new reading on heap with given vector of DP names and long values
 *
 * @param rdng  Reading asset name
 * @param dpName	Vector of Datapoint names
 * @param dpValue	Vector of Datapoint values aligned to dpName vector
 */
Reading *createReadingWithLongDatapoints(string assetName, const vector<string> &dpNames, const vector<long> &dpValues)
{
    if(dpNames.empty() || dpValues.empty() || dpNames.size() != dpValues.size())
        return nullptr;

    DatapointValue dpv((long) dpValues[0]);
    Reading *rdng = new Reading(assetName, new Datapoint(dpNames[0], dpv));
    for(int i=1; i<dpNames.size(); ++i)
    {
        DatapointValue dpv(dpValues[i]);
        Datapoint *dp = new Datapoint(dpNames[i], dpv);
        rdng->addDatapoint(dp);
    }
    return rdng;
}

/**
 * Create a new reading on heap with given vector of DP names and double values
 *
 * @param rdng  Reading asset name
 * @param dpName	Vector of Datapoint names
 * @param dpValue	Vector of Datapoint values aligned to dpName vector
 */
Reading *createReadingWithDoubleDatapoints(string assetName, const vector<string> &dpNames, const vector<double> &dpValues)
{
    if(dpNames.empty() || dpValues.empty() || dpNames.size() != dpValues.size())
        return nullptr;

    DatapointValue dpv((double) dpValues[0]);
    Reading *rdng = new Reading(assetName, new Datapoint(dpNames[0], dpv));
    for(int i=1; i<dpNames.size(); ++i)
    {
        DatapointValue dpv(dpValues[i]);
        Datapoint *dp = new Datapoint(dpNames[i], dpv);
        rdng->addDatapoint(dp);
    }
    return rdng;
}
