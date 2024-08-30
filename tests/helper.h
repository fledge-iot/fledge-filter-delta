#include <string>
#include <vector>
#include <reading.h>

using namespace std;

void addLongTypeDatapoints(Reading *rdng, vector<string> &dpNames, vector<long> &dpValues);
void addStringTypeDatapoints(Reading *rdng, vector<string> &dpNames, vector<string> &dpValues);
void addLongTypeDatapoint(Reading *rdng, const string &dpName, const long dpValue);
void addStringTypeDatapoint(Reading *rdng, const string &dpName, const string &dpValue);
Reading *createReadingWithLongDatapoints(string assetName, const vector<string> &dpNames, const vector<long> &dpValues);
Reading *createReadingWithDoubleDatapoints(string assetName, const vector<string> &dpNames, const vector<double> &dpValues);
