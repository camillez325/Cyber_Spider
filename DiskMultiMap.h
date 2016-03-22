//
//  DiskMultiMap.h
//  Project4 (official)
//
//  Created by Chuyue Zhang on 3/6/16.
//  Copyright © 2016 Camille Zhang. All rights reserved.
//

#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"
#include <cstring>

class DiskMultiMap
{
public:
    class Iterator
    {
    public:
        Iterator();
        Iterator(BinaryFile::Offset location, bool isLocationValid, char storedKey[121], char storedValue[121], char storedContext[121], string fname, BinaryFile::Offset bucketLoc);
        bool isValid() const;
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        BinaryFile::Offset m_loc;
        BinaryFile::Offset m_bucketLoc;
        bool isLocValid;
        char m_storedKey[121];
        char m_storedValue[121];
        char m_storedContext[121];
        string innerfileName;
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    
private:
    BinaryFile bf;
    struct Node
    {
        char m_key[121];
        char m_value[121];
        char m_context[121];
        BinaryFile::Offset m_next;
    };
    BinaryFile::Offset m_head;
    BinaryFile::Offset m_headDelete;
    unsigned int m_numBuckets;
    string fileName;
    void addDeletedNode(BinaryFile::Offset offset);
};

#endif // DISKMULTIMAP_H_

