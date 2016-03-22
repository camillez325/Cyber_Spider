//
//  DiskMultiMap.cpp
//  Project4 (official)
//
//  Created by Chuyue Zhang on 3/6/16.
//  Copyright © 2016 Camille Zhang. All rights reserved.
//

#include "DiskMultiMap.h"
#include "MultiMapTuple.h"
#include "BinaryFile.h"
#include <cstring>
#include <string>
#include <functional>
using namespace std;


DiskMultiMap::DiskMultiMap()
{
    m_numBuckets = 0;
    m_headDelete = 0;
    m_head = 0;
}

DiskMultiMap::~DiskMultiMap()
{
    close();
}

bool DiskMultiMap::createNew(const std::string &filename, unsigned int numBuckets)
{
    m_numBuckets = numBuckets;
    fileName = filename;
    if (!bf.createNew(filename)) 
        return false;
    bf.write(m_numBuckets, 0);
    bf.write(m_headDelete, bf.fileLength());
    for (int i = 0; i < numBuckets; i++)
    {
        if(!bf.write(m_head, bf.fileLength()))
            return false;
    }
    return true;
}

bool DiskMultiMap::openExisting(const std::string &filename)
{
   
    close();
    fileName = filename;
    if (!bf.openExisting(filename))
        return false;
    
    bf.read(m_numBuckets, 0);
    return true;
}

void DiskMultiMap::close()
{
    if (bf.isOpen())
        bf.close();
    return;
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context)

{
    if (key.size() > 120 || value.size() > 120 || context.size() > 120)
        return false;
    //hashing
    std::hash<string> hash_string;
    unsigned int hashedKey = hash_string(key) % m_numBuckets;
    
    
    BinaryFile::Offset m_hashedKey;
    m_hashedKey = hashedKey;
    BinaryFile::Offset bucketLoc = m_hashedKey* sizeof(BinaryFile::Offset) + sizeof(m_headDelete) + sizeof(m_numBuckets);
    
    //fill in three cstrings
    Node n;
    strcpy(n.m_key, key.c_str());
    strcpy(n.m_value, value.c_str());
    strcpy(n.m_context, context.c_str());
    
    BinaryFile::Offset firstOffset;
    bf.read(firstOffset, sizeof(unsigned int));
    if (firstOffset != 0)    //there's available deleted node!
    {
        BinaryFile::Offset firstLoc;
        bf.read(firstLoc, bucketLoc);
        n.m_next = firstLoc;
        
        firstLoc = firstOffset;
        bf.write(firstLoc, bucketLoc);
        
        Node temp;
        bf.read(temp, firstOffset);
        BinaryFile::Offset nextAvailable;
        nextAvailable = temp. m_next;
        
        bf.write(n, firstLoc);
        bf.write(nextAvailable, sizeof(unsigned int));
        return true;
    }
    
    BinaryFile::Offset head;
    bf.read(head, bucketLoc);
    if ( head == 0)
    {
        bf.write(bf.fileLength(), bucketLoc);
        n.m_next = bucketLoc;
        bf.write(n, bf.fileLength());
        return true;
    }
    else
    {
        BinaryFile::Offset currentLoc;
        bf.read(currentLoc, bucketLoc);
        bf.write(bf.fileLength(), bucketLoc);
        n.m_next = currentLoc;
        bf.write(n, bf.fileLength());
        return true;
        
    }
}

int DiskMultiMap::erase(const std::string &key, const std::string &value, const std::string &context)
{
    //hash it first!
    std::hash<string> hash_string;
    unsigned int hashedKey = hash_string(key) % m_numBuckets;
    BinaryFile::Offset m_hashedKey;
    m_hashedKey = hashedKey;
    BinaryFile::Offset bucketLoc = m_hashedKey*sizeof(BinaryFile::Offset) + sizeof(m_headDelete) + sizeof(m_numBuckets);
    
    BinaryFile::Offset tester;
    bf.read(tester, bucketLoc);
    if (tester == 0)
        return 0;
    
    int count = 0;
    Node node;
    Node prenode;
    BinaryFile::Offset preLoc;
    BinaryFile::Offset Loc;
    preLoc = bucketLoc;
    bf.read(Loc, bucketLoc);
    bf.read(node, Loc);
    
    while (Loc != bucketLoc)
    {
        
        bf.read(node, Loc);
        bf.read(prenode, preLoc);
        if (strcmp(node.m_key, key.c_str()) == 0 && strcmp(node.m_value, value.c_str()) == 0 && strcmp(node.m_context, context.c_str()) == 0)
        {
            count++;
            addDeletedNode(Loc);
            if (preLoc == bucketLoc)
            {
                bf.write(node.m_next, bucketLoc);
            }
            else
            {
                prenode.m_next = node.m_next;
                bf.write(prenode, preLoc);
            }
        }
        preLoc = Loc;
        Loc = node.m_next;
    }
    return count;
    
}

void DiskMultiMap::addDeletedNode(BinaryFile::Offset offset)
{
    BinaryFile::Offset first;
    bf.read(first, sizeof(unsigned int));
    Node deletedNode;
    bf.read(deletedNode, offset);
    deletedNode.m_next = first;
    bf.write(deletedNode, offset);
    bf.write(offset, sizeof(unsigned int));

}
DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key)
{
    //hash it!
    std::hash<string> hash_string;
    unsigned int hashedKey = hash_string(key) % m_numBuckets;
    BinaryFile::Offset m_hashedKey;
    m_hashedKey = hashedKey;
    BinaryFile::Offset bucketLoc = m_hashedKey*sizeof(BinaryFile::Offset) + sizeof(m_headDelete) + sizeof(m_numBuckets);
    
    Iterator it;
    BinaryFile::Offset currentLoc;
    bf.read(currentLoc, bucketLoc);
    while (currentLoc != 0 && currentLoc != bucketLoc)
    {
        Node n;
        bf.read(n, currentLoc);
        if (strcmp(n.m_key, key.c_str()) == 0)
        {
            Iterator it(currentLoc, true, n.m_key, n.m_value, n.m_context, fileName, bucketLoc);
            return it;
        }
        currentLoc = n.m_next;
        
    }
    return it;
}

DiskMultiMap::Iterator::Iterator()
{
    m_loc = 0;
    isLocValid = false;
}

DiskMultiMap::Iterator::Iterator(BinaryFile::Offset location, bool isLocationValid, char storedKey[121], char storedValue[121], char storedContext[121], string fname, BinaryFile::Offset bucketLoc)
{
    m_loc = location;
    isLocValid = isLocationValid;
    strcpy(m_storedKey, storedKey);
    strcpy(m_storedValue, storedValue);
    strcpy(m_storedContext, storedContext);
    innerfileName = fname;
    m_bucketLoc = bucketLoc;
}

bool DiskMultiMap::Iterator::isValid() const
{
    return isLocValid;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()
{
    if (isLocValid)
    {
        BinaryFile bf;
        Node n;
        bf.openExisting(innerfileName);
        BinaryFile::Offset currLoc;
    
        currLoc = m_loc;
        bf.read(n, currLoc);
        currLoc = n.m_next;
        
        while (currLoc != m_bucketLoc && currLoc != 0)
        {
            bf.read(n, currLoc);
            if (strcmp(n.m_key, m_storedKey) == 0)
            {
                m_loc = currLoc;
                return *this;
            }
            
            bf.read(n, currLoc);
            currLoc = n.m_next;
            
        }
        isLocValid = false;
        return *this;
    }
    return *this;

}

MultiMapTuple DiskMultiMap :: Iterator :: operator*()
{
    MultiMapTuple m;
    Node n;
    BinaryFile bf;
    bf.openExisting(innerfileName);
    bf.read(n, m_loc);
    
    if (!isLocValid)
    {
        m.key = "";
        m.value = "";
        m.context = "";
        bf.close();
        return m;
    }
    else
    {
        m.key = string(n.m_key);
        m.value = string(n.m_value);
        m.context = string(n.m_context);
        bf.close();
        return m;
    }
 

}



















