//
//  IntelWeb.cpp
//  Project4 (official)
//
//  Created by Chuyue Zhang on 3/7/16.
//  Copyright © 2016 Camille Zhang. All rights reserved.
//

#include "IntelWeb.h"
#include "MultiMapTuple.h"
#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <vector>

using namespace std;

inline bool operator<(const InteractionTuple& t1, const InteractionTuple& t2)
{
    if (t1.context != t2.context)
        return t1.context < t2.context;
    if (t1.from!= t2.from)
        return t1.from < t2.from;
    if (t1.to!= t2.to)
        return t1.to < t2.to;
    return false;
}

inline bool operator==(const InteractionTuple& t1, const InteractionTuple& t2)
{
    if ( t1.from == t2.from && t1.to == t2.to && t1.context == t2.context)
        return true;
    return false;
}

IntelWeb::IntelWeb()
{
    name1 = "-one.dat";
    name2 = "-two.dat";
}

IntelWeb::~IntelWeb()
{
    m_tableOne.close();
    m_tableTwo.close();
}

bool IntelWeb::createNew(const std::string &filePrefix, unsigned int maxDataItems)
{
   
    if (!m_tableOne.createNew(filePrefix + name1, maxDataItems/0.75))
    {
        m_tableOne.close();
        m_tableTwo.close();
        return false;
    }
    if (!m_tableTwo.createNew(filePrefix + name2, maxDataItems/0.75))
    {
        m_tableOne.close();
        m_tableTwo.close();
        return false;
    }
    return true;
    
}

bool IntelWeb::openExisting(const std::string &filePrefix)
{
    
    if (!m_tableOne.openExisting(filePrefix + "-one.dat"))
    {
        m_tableOne.close();
        m_tableTwo.close();
        return false;
    }
    if (!m_tableTwo.openExisting(filePrefix + "-two.dat"))
    {
        m_tableOne.close();
        m_tableTwo.close();
        return false;
    }
    return true;
}

void IntelWeb::close()
{
    m_tableOne.close();
    m_tableTwo.close();
}

bool IntelWeb::ingest(const std::string &telemetryFile)
{
    ifstream inf(telemetryFile);
    if (!inf)
        return false;
    string line;
    while (getline(inf, line))
    {
        istringstream iss(line);
        string machine;
        string left;
        string right;
        iss >> machine >> left >> right;
        m_tableOne.insert(left, right, machine);
        m_tableTwo.insert(right, left, machine);
    }
    return true;
    
}

unsigned int IntelWeb::calculatePrevalence(const std::string key)
{
    unsigned int count = 0;
    DiskMultiMap::Iterator it1 = m_tableOne.search(key);
    while (it1.isValid())
    {
        count++;
        ++it1;
    }
    DiskMultiMap::Iterator it2 = m_tableTwo.search(key);
    while (it2.isValid())
    {
        count++;
        ++it2;
    }
    return count;
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
                   unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound,
                   std::vector<InteractionTuple>& interactions
                   )
{
    unsigned int count = 0;
    if (indicators.size() == 0)
        return 0;
    bool match = false;
    set<string> alreadyExamined;
    map<string,int> p;
    queue<string> stringQueue;
    for (auto& item: indicators)
    {
        stringQueue.push(item);
    }
    while (!stringQueue.empty())
    {
        string temp = stringQueue.front();
        stringQueue.pop();
        if (alreadyExamined.count(temp)!=0 || calculatePrevalence(temp) >= minPrevalenceToBeGood)
            continue;
        alreadyExamined.insert(temp);
        
        DiskMultiMap::Iterator it1 = m_tableOne.search(temp);
        while (it1.isValid())
        {
            match = true;
            MultiMapTuple mt = *it1;
            const string key1 = mt.key;
            const string value1 = mt.value;
            const string context1 = mt.context;
            InteractionTuple itp(key1, value1, context1);
            interactions.push_back(itp);
            p[key1] += 1;
            p[value1] += 1;
            
            
            stringQueue.push(value1);
            ++it1;
        }
        
        DiskMultiMap::Iterator it2 = m_tableTwo.search(temp);
        while (it2.isValid())
        {
            MultiMapTuple mt = *it2;
            string value2 = mt.value;
            stringQueue.push(value2);
            ++it2;
        }
    }
    if (!match)
        return 0;
    map<string,int>::iterator pp = p.begin();
    while (pp!= p.end())
    {
        if (std::find(indicators.begin(), indicators.end(),pp->first) != indicators.end())
        {
            map<string,int>::iterator ppt = p.erase(pp);
            ppt = p.begin();
        }
        pp++;
    }
    pp = p.begin();
    while (pp != p.end())
    {
        badEntitiesFound.push_back(pp->first);
        pp++;
    }
    sort(interactions.begin(), interactions.end());
    interactions.erase(std::unique(interactions.begin(), interactions.end()), interactions.end());
    count = badEntitiesFound.size();
    return count;
}

bool IntelWeb::purge(const std::string &entity)
{
    bool found = false;
    DiskMultiMap::Iterator it1 = m_tableOne.search(entity);
    while (it1.isValid())
    {
        found = true;
        MultiMapTuple mt1 = *it1;
        m_tableOne.erase(mt1.key, mt1.value, mt1.context);
        ++it1;
    }
    DiskMultiMap::Iterator it2 = m_tableTwo.search(entity);
    while (it2.isValid())
    {
        MultiMapTuple mt2 = *it2;
        m_tableTwo.erase(mt2.key, mt2.value, mt2.context);
        m_tableOne.erase(mt2.value, mt2.key, mt2.context);
        ++it2;
    }
    return found;
}




















