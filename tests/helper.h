#ifndef _HELPER_H_
#define _HELPER_H_

#include <string>
#include <vector>
#include <reading.h>

using namespace std;

void addStringTypeDatapoint(Reading *rdng, const string &dpName, const string &dpValue);
Reading *createReadingWithLongDatapoints(string assetName, const vector<string> &dpNames, const vector<long> &dpValues);
Reading *createReadingWithDoubleDatapoints(string assetName, const vector<string> &dpNames, const vector<double> &dpValues);

#endif
