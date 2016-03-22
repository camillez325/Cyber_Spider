#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include <string>
using namespace std;

#include <string>
#include <vector>

#include "DiskMultiMap.h"
class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& interactions
                       );
    bool purge(const std::string& entity);
    
private:
    DiskMultiMap m_tableOne;
    DiskMultiMap m_tableTwo;
    string name1;
    string name2;
    unsigned int calculatePrevalence(const std::string key);
};
inline bool operator<(const InteractionTuple& t1, const InteractionTuple& t2);
inline bool operator==(const InteractionTuple& t1, const InteractionTuple& t2);


#endif // INTELWEB_H_

