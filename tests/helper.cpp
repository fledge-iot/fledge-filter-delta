#include "helper.h"

using namespace std;

void addLongTypeDatapoints(Reading *rdng, vector<string> &dpNames, vector<long> &dpValues)
{
    for(int i=0; i<dpNames.size(); ++i)
    {
        DatapointValue dpv(dpValues[i]);
	    Datapoint *dp = new Datapoint(dpNames[i], dpv);
        rdng->addDatapoint(dp);
    }
    return;
}

void addStringTypeDatapoints(Reading *rdng, vector<string> &dpNames, vector<string> &dpValues)
{
    for(int i=0; i<dpNames.size(); ++i)
    {
        DatapointValue dpv(dpValues[i]);
	    Datapoint *dp = new Datapoint(dpNames[i], dpv);
        rdng->addDatapoint(dp);
    }
    return;
}

void addLongTypeDatapoint(Reading *rdng, const string &dpName, const long dpValue)
{
    DatapointValue dpv(dpValue);
    Datapoint *dp = new Datapoint(dpName, dpv);
    rdng->addDatapoint(dp);
}

void addStringTypeDatapoint(Reading *rdng, const string &dpName, const string &dpValue)
{
    DatapointValue dpv(dpValue);
    Datapoint *dp = new Datapoint(dpName, dpv);
    rdng->addDatapoint(dp);
}

Reading *createReadingWithLongDatapoints(string assetName, const vector<string> &dpNames, const vector<long> &dpValues)
{
    if(dpNames.empty())
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

Reading *createReadingWithDoubleDatapoints(string assetName, const vector<string> &dpNames, const vector<double> &dpValues)
{
    if(dpNames.empty())
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
