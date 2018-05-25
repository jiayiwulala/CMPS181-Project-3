
#include "ix.h"
#include "../rbf/pfm.h"
#include "../rbf/rbfm.h"
#include <stdio.h>

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
//#include <rbfm.h>

IndexManager* IndexManager::_index_manager = 0;
PagedFileManager* IndexManager::_pf_manager = NULL;

IndexManager* IndexManager::instance()
{
    if(!_index_manager)
        _index_manager = new IndexManager();

    return _index_manager;
}

IndexManager::IndexManager()
{
    // Initialize the internal PagedFileManager instance
    _pf_manager = PagedFileManager::instance();
}

IndexManager::~IndexManager()
{
}
//createfile, create a file with a metapage(0), rootpage(1), leafpage(2) with header in it , no entry inserted
RC IndexManager::createFile(const string &fileName)
{
    // creating a new paged file
    if (_pf_manager->createFile(fileName))
        return IX_CREATE_FAILED;
    IXFileHandle ixfileHandle;
    RC rc = openFile(fileName, ixfileHandle);
    if (rc) return IX_OPEN_FAILED;


    //setting up the first page  calloc sets allocated memory to zero.
    void * MetaPageData = calloc(PAGE_SIZE, 1);
    if (MetaPageData == NULL) {
        return IX_MALLOC_FAILED;
    }
    MetapageHeader metaHeader;//metapage 0, root page 1   root-left page 2
    metaHeader.rootPageNumber = 1;
    memcpy(MetaPageData, &metaHeader, sizeof(metaHeader));

    rc = ixfileHandle.appendPage(MetaPageData);
    if (rc) {
        closeFile(ixfileHandle);
        free(MetaPageData);
        return IX_APPEND_FAILED;
    }


    memset(MetaPageData, 0, PAGE_SIZE);
    // initialize root page , header without entry
    NoLeafIndexHeader rootHeader;
    rootHeader.numberOfEntries = 0;
    rootHeader.pointerToFreeSpace = PAGE_SIZE;
    rootHeader.leftMostPointer = 2;

    //set node type 0 internal       1 leaf
    NodeType nodeType = IndexType;
    memcpy(MetaPageData, &nodeType, sizeof(NodeType));
    memcpy((char *)MetaPageData + sizeof(NodeType), &rootHeader, sizeof(NoLeafIndexHeader));
    //append rootpage
    rc = ixfileHandle.appendPage(MetaPageData);
    if (rc) {
        closeFile(ixfileHandle);
        free(MetaPageData);
        return IX_APPEND_FAILED;
    }


    memset(MetaPageData, 0, PAGE_SIZE);
    //set up a leaf node
    NodeType nodeTypeLeaf = LeafType;
    memcpy(MetaPageData, &nodeTypeLeaf, sizeof(NodeType));

    LeafHeader leafHeader;
    leafHeader.prevPointer = NULL;
    leafHeader.nextPointer = NULL;
    leafHeader.numberOfEntry = 0;
    leafHeader.pointerToFreeSpace= PAGE_SIZE;
    memcpy((char *)MetaPageData + sizeof(NodeType), &leafHeader, sizeof(LeafHeader));
    //append leaf page
    rc = ixfileHandle.appendPage(MetaPageData);
    if (rc) {
        closeFile(ixfileHandle);
        free(MetaPageData);
        return IX_APPEND_FAILED;
    }

    closeFile(ixfileHandle);
    free(MetaPageData);
    return SUCCESS;
}

RC IndexManager::destroyFile(const string &fileName)
{
    RC rc = _pf_manager->destroyFile(fileName);
    if (rc) return IX_DESTORY_FAILED;
    return SUCCESS;
}

RC IndexManager::openFile(const string &fileName, IXFileHandle &ixfileHandle)
{
    RC rc = _pf_manager->openFile(fileName, ixfileHandle.fileHandle);
    if (rc) return IX_OPEN_FAILED;
    return SUCCESS;
}

RC IndexManager::closeFile(IXFileHandle &ixfileHandle)
{
    RC rc = _pf_manager->closeFile(ixfileHandle.fileHandle);
    if (rc) return IX_CLOSE_FAILED;
    return SUCCESS;
}

 //(1) For INT and REAL: use 4 bytes; (2) For VARCHAR: use 4 bytes for the length
 // followed by the characters. Note that the given key value doesn't contain Null flags
//Attribute is the attribution descriptor of the key, three fields, name type length
RC IndexManager::insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid) {
    // search from root, search first, then insert . the root page number can be found in metapage
    //dataEntry key can recovered from attribute and key

    //first, through attribute type and length, get key data
    DummyEntry dummyEntry;
    dummyEntry.key = NULL;
    dummyEntry.childPage = 0;

    //get root page
    void *metaPage = malloc(PAGE_SIZE);
    if (metaPage == NULL) {
        return IX_MALLOC_FAILED;
    }
    RC rc = ixfileHandle.readPage(0, metaPage);
    if (rc) {
        free(metaPage);
        return IX_READ_FAILED;
    }

    MetapageHeader metaPageHeader;
    memcpy(&metaPageHeader, metaPage, sizeof(metaPageHeader));
    int32_t rootPageNumber = metaPageHeader.rootPageNumber;
    free(metaPage);

    /*

    void *rootpageData = malloc(PAGE_SIZE);
    if (rootpageData == NULL) {
        return IX_MALLOC_FAILED;
    }
    rc = ixfileHandle.readPage(rootPageNumber, rootpageData);
    if (rc) {
        free(rootpageData);
        return IX_READ_FAILED;
    }
     */
    return insert(ixfileHandle, attribute, key, rid, rootPageNumber, dummyEntry);

}

RC IndexManager::insert(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid, int32_t pageID, DummyEntry &dummyEntry){
     void *pageData = malloc(PAGE_SIZE);
     if (pageData == NULL) {
         return IX_MALLOC_FAILED;
     }
     RC rc = ixfileHandle.readPage(pageID, pageData);
     if (rc) {
         free(pageData);
         return IX_READ_FAILED;
     }

     NodeType nodeType;
     memcpy(&nodeType, pageData, sizeof(nodeType));

     if (nodeType == IndexType) {
         int32_t childPage = getChildPage(pageData, attribute, key);
         //cout << childPage << endl;
         free(pageData);
         rc = insert(ixfileHandle, attribute, key, rid, childPage, dummyEntry);
         //cout <<"rc"<<rc<<endl;
         if (rc) return rc;
            // no split happened if dummyentry.key == NULL
         if (dummyEntry.key == NULL) {
             return SUCCESS;
         }

         pageData = malloc(PAGE_SIZE);
         if (ixfileHandle.readPage(pageID, pageData)) {
             free(pageData);

             return IX_READ_FAILED;
         }

         rc = insertToInnerIndexPage(pageData, attribute, dummyEntry);
         if (rc == SUCCESS) {
             rc = ixfileHandle.writePage(pageID, pageData);
             free(dummyEntry.key);
             dummyEntry.key = NULL;
             dummyEntry.childPage = 0;
             free(pageData);
            // pageData = NULL;
             return rc;

         }
         else if (NOT_ENOUGH_SPACE) {
             rc = splitInnerIndex(ixfileHandle, attribute, pageData, pageID, dummyEntry);
             free(pageData);
            // pageData = NULL;
             return rc;

         }
         else {
             free(pageData);
             free(dummyEntry.key);
             dummyEntry.key = NULL;
             dummyEntry.childPage = 0;
             return IX_INSERT_FAILED;
         }

     }
     else {
        rc = insertToLeafPage(pageData, attribute, key, rid);
        if (rc == NOT_ENOUGH_SPACE) {
            rc = splitLeaf(ixfileHandle, attribute, key, rid, pageData, pageID, dummyEntry);
            free(pageData);
            return rc;
        }
        else if (rc == SUCCESS) {
            if (ixfileHandle.writePage(pageID, pageData)) {
               // free(pageData); //??????
                return IX_WRITE_FAILED;
            }
            free(pageData);
            free(dummyEntry.key);
            dummyEntry.key = NULL;
            dummyEntry.childPage = 0;
            return SUCCESS;
        }
        else {
            free(pageData);
            free(dummyEntry.key);
            dummyEntry.key = NULL;
            dummyEntry.childPage = 0;
            return IX_INSERT_FAILED;
        }
     }
}

RC IndexManager::splitInnerIndex(IXFileHandle &ixFileHandle, const Attribute &attribute, void *pageData,
                                 int32_t pageID, DummyEntry &dummyEntry) {

    //cout << "1" << endl;
    NoLeafIndexHeader indexHeader;
    memcpy(&indexHeader, (char *)pageData + sizeof(NodeType), sizeof(NoLeafIndexHeader));
    int middle = indexHeader.numberOfEntries / 2;

    NoLeafIndexEntry middleEntry;
    memcpy(&middleEntry, (char *)pageData + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * (middle),
           sizeof(NoLeafIndexEntry));

    void *newIndexPage = calloc(PAGE_SIZE, 1);
    NoLeafIndexHeader newIndexHeader;
    newIndexHeader.numberOfEntries = 0;
    newIndexHeader.pointerToFreeSpace = PAGE_SIZE;
    newIndexHeader.leftMostPointer = middleEntry.childPageId;
    NodeType nodeType = IndexType;
    memcpy(newIndexPage, &nodeType, sizeof(NodeType));
    memcpy((char *)newIndexPage + sizeof(NodeType), &newIndexHeader, sizeof(NoLeafIndexHeader));

    int32_t newIndexPageId = ixFileHandle.getNumberOfPages();
    //get entry, check key type, if int/real, move directly, varchar, move entry and varchar+length
    //move middle entry and forwarding to new page

    // mov index entries
    void *moveKey = malloc (attribute.length + 4);
    // Repeatedly insert an entry from one page into the other, then delete the entry from the original page
    for (int i = middle + 1; i < indexHeader.numberOfEntries; i++)
    {
        NoLeafIndexEntry entry;
        memcpy(&entry, (char*)pageData + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * (i),
               sizeof(NoLeafIndexEntry));

        int32_t movePageId = entry.childPageId;
        if (attribute.type == TypeVarChar)
        {
            int32_t len;
            memcpy(&len, (char*)pageData + entry.charKeyOffset, INT_SIZE);
            memcpy(moveKey, &len, INT_SIZE);
            memcpy((char*)moveKey + INT_SIZE,(char*)pageData + entry.charKeyOffset + INT_SIZE, len);
        }
        else if (attribute.type == TypeInt)
        {
            memcpy(moveKey, &(entry.intkey), INT_SIZE);
        }
        else
        {
            memcpy(moveKey, &(entry.realkey), REAL_SIZE);
        }
        DummyEntry moveEntry;
        moveEntry.key = moveKey;
        moveEntry.childPage = movePageId;
        insertToInnerIndexPage(newIndexPage, attribute, moveEntry);
    }
    free(moveKey);
    indexHeader.numberOfEntries -= indexHeader.numberOfEntries - middle - 1;
    if (attribute.type == TypeVarChar) {
        indexHeader.pointerToFreeSpace += middleEntry.charKeyOffset - indexHeader.pointerToFreeSpace;
    }
    //cout<<"update header"<<indexHeader.numberOfEntries<<indexHeader.pointerToFreeSpace<<endl;
    memcpy((char *)pageData + sizeof(NodeType), &indexHeader, sizeof(NoLeafIndexEntry));

    // Delete middle entry
    int32_t keySize = INT_SIZE;
    if (attribute.type == TypeVarChar) {
        int32_t varLen;
        memcpy(&varLen, (char *)pageData + middleEntry.charKeyOffset, INT_SIZE);
        keySize = INT_SIZE + varLen;
    }

    void * middleKey = malloc(keySize);

    if (attribute.type == TypeInt) {
        memcpy(middleKey, &middleEntry.intkey, INT_SIZE);
    }
    else if (attribute.type == TypeReal) {
        memcpy(middleKey, &middleEntry.realkey, REAL_SIZE);
    }
    else {
        memcpy(middleKey, (char*)pageData + middleEntry.charKeyOffset, keySize);
    }
    //cout << "beforedelete....." << endl;
    deleteIndexEntry(attribute, middleKey, pageData);
    //cout << "delete....." << endl;

    if (compareWithOtherLeafEntry(dummyEntry.key, attribute, middle, pageData) <= 0) {
        //cout << "if----delete.....insertToInnerIndexPage" << endl;
        RC rc=insertToInnerIndexPage(pageData, attribute, dummyEntry);
        //cout<<"rc"<< rc<<endl;
        if (rc){
            //cout << "delete.....insertToInnerIndexPage" << endl;
            free(newIndexPage);
            return IX_INSERT_FAILED;
        }
    }
    else {
        //cout << "e;se=-=delete.....insertToInnerIndexPage" << endl;
        if (insertToInnerIndexPage(newIndexPage, attribute, dummyEntry)){
            //cout << "delete.....insertToInnerIndexPage>0" << endl;
            free(newIndexPage);
            return IX_INSERT_FAILED;
        }
    }

    if (ixFileHandle.writePage(pageID, pageData)) {
        free(newIndexPage);
        return IX_WRITE_FAILED;
    }
    if (ixFileHandle.appendPage(newIndexPage)) {
        free(newIndexPage);
        return IX_APPEND_FAILED;
    }

    free(newIndexPage);

    free(dummyEntry.key);
    dummyEntry.key = middleKey;
    dummyEntry.childPage = newIndexPageId;


    void *metaPage = malloc(PAGE_SIZE);
    if (metaPage == NULL) {
        return IX_MALLOC_FAILED;
    }
    RC rc = ixFileHandle.readPage(0, metaPage);
    if (rc) {
        free(metaPage);
        return IX_READ_FAILED;
    }

    MetapageHeader metaPageHeader;
    memcpy(&metaPageHeader, metaPage, sizeof(metaPageHeader));
    int32_t rootPageNumber = metaPageHeader.rootPageNumber;

    if (pageID == rootPageNumber)
    {
        void *newRootPage = calloc(PAGE_SIZE, 1);
        NodeType newNodeType = IndexType;

        NoLeafIndexHeader newrootHeader;
        newrootHeader.numberOfEntries = 0;
        newrootHeader.pointerToFreeSpace = PAGE_SIZE;
        newrootHeader.leftMostPointer= pageID;
        memcpy((char *) newRootPage, &newNodeType, sizeof(NodeType));
        memcpy((char *) newRootPage + sizeof(NodeType), &newrootHeader, sizeof(NoLeafIndexHeader));

        insertToInnerIndexPage(newRootPage, attribute, dummyEntry);

        int32_t newRootPageId = ixFileHandle.getNumberOfPages();
        if(ixFileHandle.appendPage(newRootPage))
            return IX_APPEND_FAILED;

        metaPageHeader.rootPageNumber = newRootPageId;

        memcpy(metaPage, &metaPageHeader, sizeof(MetapageHeader));
        if(ixFileHandle.writePage(0, metaPage))
            return IX_WRITE_FAILED;
        // Free memory
        free(metaPage);
        free(newRootPage);
        free(dummyEntry.key);
        dummyEntry.key = NULL;
    }
    return SUCCESS;

}

RC IndexManager::splitLeaf(IXFileHandle &ixFileHandle, const Attribute &attribute, const void* key, const RID rid,
                           void* pageData, int32_t pageID, DummyEntry &dummyEntry) {
    // split to half, then add entry to the second part,
    //copy first node in second part, insert to upperlevel index page
    //write the change to file
    LeafHeader leafHeader;
    memcpy(&leafHeader, (char *) pageData + sizeof(NodeType), sizeof(leafHeader));
    int d = leafHeader.numberOfEntry;
    int middle = d / 2;

    void *newLeafPage = calloc(PAGE_SIZE, 1);
    LeafHeader newleafHeader;
    newleafHeader.numberOfEntry = 0;
    newleafHeader.pointerToFreeSpace = PAGE_SIZE;
    newleafHeader.nextPointer = leafHeader.nextPointer;
    newleafHeader.prevPointer = pageID;

    NodeType newNodeType = LeafType;
    memcpy(newLeafPage, &newNodeType, sizeof(NodeType));
    memcpy((char *)newLeafPage + sizeof(NodeType), &newleafHeader, sizeof(LeafHeader));

    int32_t newLeafPageId = ixFileHandle.getNumberOfPages();
    leafHeader.nextPointer = newLeafPageId;
    memcpy((char *)pageData + sizeof(NodeType), &leafHeader, sizeof(LeafHeader));

    DataEntry middleEntry;
    memcpy(&middleEntry, (char *)pageData + sizeof(NodeType) + sizeof(LeafHeader) + sizeof(DataEntry) * middle, sizeof(DataEntry));

    void *moveKey = malloc (attribute.length + 4);
    for (int i = middle + 1; i < leafHeader.numberOfEntry; i++)
    {
        //take data entry after the middle entry. mov over
        DataEntry moveEntry;
        memcpy(&moveEntry, (char *)pageData + sizeof(NodeType) + sizeof(LeafHeader) + sizeof(DataEntry) * (i), sizeof(DataEntry));
        RID moveEntryRid = moveEntry.rid;

        if (attribute.type == TypeVarChar)
        {
            int32_t len;
            memcpy(&len, (char*)pageData + moveEntry.charKeyOffset, INT_SIZE);
            memcpy(moveKey, &len, INT_SIZE);
            memcpy((char*)moveKey + INT_SIZE, (char*)pageData + moveEntry.charKeyOffset + INT_SIZE, len);
        }
        else if (attribute.type == TypeInt)
        {
            memcpy(moveKey, &(moveEntry.intkey), INT_SIZE);
        }
        else
        {
            memcpy(moveKey, &(moveEntry.realkey), REAL_SIZE);
        }

        // Insert into new leaf, delete from old
        insertToLeafPage(newLeafPage, attribute, moveKey, moveEntryRid);
    }
    free(moveKey);
    leafHeader.numberOfEntry -= d - middle - 1;

    if (attribute.type == TypeVarChar) {
        leafHeader.pointerToFreeSpace += middleEntry.charKeyOffset - leafHeader.pointerToFreeSpace;
    }

    memcpy((char *)pageData + sizeof(NodeType), &leafHeader, sizeof(LeafHeader));
    //cout << "compare" << endl;
    if (compareWithOtherLeafEntry(key, attribute, middle, pageData) <= 0) {
       // cout << "compareWithOtherLeafEntry(key, attribute, middle, pageData) <= 0" << endl;
        if (insertToLeafPage(pageData, attribute, key, rid)){
            free(newLeafPage);
            return -1;
        }
    }
    else {

        if (insertToLeafPage(newLeafPage, attribute, key, rid)){

            //cout << "else" <<endl;
            free(newLeafPage);
            //cout << "insert ro leaf failed" <<endl;
            return -1;
        }
    }

    if (ixFileHandle.writePage(pageID, pageData)) {
        free(newLeafPage);
        return IX_WRITE_FAILED;
    }
    if (ixFileHandle.appendPage(newLeafPage)) {
        free(newLeafPage);
        return IX_APPEND_FAILED;
    }
   // cout << "insert finish" << endl;
    dummyEntry.childPage = newLeafPageId;

    int32_t keySize = INT_SIZE;
    if (attribute.type == TypeVarChar) {
        int32_t varLen;
        memcpy(&varLen, (char * ) pageData + middleEntry.charKeyOffset , INT_SIZE);
        keySize = INT_SIZE + varLen;
    }
    dummyEntry.key = malloc(keySize);

    if (attribute.type == TypeInt) {
        memcpy(dummyEntry.key, &(middleEntry.intkey), INT_SIZE);
    }
    else if (attribute.type == TypeReal) {
        memcpy(dummyEntry.key, &(middleEntry.realkey), REAL_SIZE);
    }
    else {
        memcpy(dummyEntry.key, (char * )pageData + middleEntry.charKeyOffset, keySize);
    }

    free(newLeafPage);
    return SUCCESS;

}


RC IndexManager::insertToLeafPage(void *insertPage, const Attribute attribute, const void* key, const RID rid){
    //cout<<"inserroleaf page"<<endl;
    LeafHeader leafHeader;   //prevpointer, nextpointer, numberofentry, freespace
    memcpy(&leafHeader, (char *)insertPage + sizeof(NodeType), sizeof(LeafHeader));

    DataEntry newEntry;
    newEntry.rid = rid;
    int32_t newEntryLength;
    if (attribute.type == TypeInt) {
        newEntryLength = sizeof(DataEntry);
        int32_t int_key;
        memcpy(&int_key, key, INT_SIZE);
        newEntry.intkey = int_key;
    }
    else if (attribute.type == TypeReal) {
        newEntryLength = sizeof(DataEntry);
        float real_key;
        memcpy(&real_key, key, REAL_SIZE);
        newEntry.realkey = real_key;
    }
    else{
        newEntryLength = sizeof(DataEntry);
        int32_t varCharLeng;
        memcpy(&varCharLeng, key, INT_SIZE);
        newEntryLength += INT_SIZE + varCharLeng;

        memcpy((char *) insertPage + leafHeader.pointerToFreeSpace - INT_SIZE - varCharLeng, key, varCharLeng + INT_SIZE);
        newEntry.charKeyOffset = leafHeader.pointerToFreeSpace - INT_SIZE - varCharLeng;
        leafHeader.pointerToFreeSpace = newEntry.charKeyOffset;
    }

    if (getFreeSpaceLeaf(insertPage) < newEntryLength){
       // cout << ")))))))))" <<endl;
        return NOT_ENOUGH_SPACE;}

    int i;
    for (i = 0; i < leafHeader.numberOfEntry; i++)
    {
        if (compareWithOtherLeafEntry( key, attribute, i, insertPage) < 0)
            break;
    }

    // i is slot number to move
    int offset = sizeof(NodeType) + sizeof(LeafHeader);
    memmove((char *) insertPage + offset + sizeof(DataEntry) * (i+1), (char *) insertPage + offset + sizeof(DataEntry) * i,
            sizeof(DataEntry) * (leafHeader.numberOfEntry - i) );

    leafHeader.numberOfEntry += 1;
    memcpy((char *) insertPage + sizeof(NodeType), &leafHeader, sizeof(LeafHeader));
    memcpy((char *) insertPage + offset + sizeof(DataEntry) * i, &newEntry, sizeof(DataEntry));
    leafHeader.numberOfEntry += 1;
    return SUCCESS;

}


int IndexManager::getFreeSpaceLeaf(void *pageData)
{
    LeafHeader header = getLeafHeader(pageData);
    return header.pointerToFreeSpace - (sizeof(NodeType) + sizeof(LeafHeader) + header.numberOfEntry * sizeof(DataEntry));
}


int IndexManager::compareWithOtherLeafEntry(const void * key, const Attribute attribute, const int i, const void *insertPage) {
    DataEntry entry;
    memcpy(&entry, (char *)insertPage + sizeof(NodeType) + sizeof(LeafHeader) + sizeof(DataEntry) * i, sizeof(DataEntry));
    if (attribute.type == TypeInt) {
        int32_t int_key;
        memcpy(&int_key, key, INT_SIZE);
        return compare(int_key, entry.intkey);
    }
    else if (attribute.type == TypeReal) {
        float float_key;
        memcpy(&float_key, key, REAL_SIZE);
        return compare(float_key, entry.realkey);
    }
    else {
        int32_t leng;
        memcpy(&leng, key, INT_SIZE);
        char keyString[leng + 1];
        keyString[leng] = '\0';
        memcpy(keyString, (char *) key + INT_SIZE, leng);

        int32_t entryLeng;
        memcpy(&entryLeng, (char *) insertPage + entry.charKeyOffset, INT_SIZE);
        char entryKeyString[entryLeng + 1];
        entryKeyString[entryLeng] = '\0';
        memcpy(entryKeyString, (char *) insertPage + entry.charKeyOffset + INT_SIZE, entryLeng);
        return compare(keyString, entryKeyString);
    }
}


int IndexManager::getFreeSpaceofIndexPage(const void* pageData){
    NoLeafIndexHeader noLeafIndexHeader;
    memcpy(&noLeafIndexHeader, (char *)pageData + sizeof(NodeType), sizeof(noLeafIndexHeader));
    uint16_t freeSpace = noLeafIndexHeader.pointerToFreeSpace;
    freeSpace -= sizeof(NodeType) + sizeof(noLeafIndexHeader);
    freeSpace -= sizeof(NoLeafIndexEntry) * noLeafIndexHeader.numberOfEntries;
    return freeSpace;
}

RC IndexManager::insertToInnerIndexPage(void * insertPage, const Attribute attribute, DummyEntry dummyEntry){
    NoLeafIndexHeader indexHeader;
    memcpy(&indexHeader, (char *) insertPage + sizeof(NodeType), sizeof(NoLeafIndexHeader));
    int indexEntrySize = sizeof(NoLeafIndexEntry);

    if (attribute.type == TypeVarChar) {
        int32_t keyLength;
        memcpy(&keyLength, dummyEntry.key, INT_SIZE);
        indexEntrySize += INT_SIZE + keyLength;
    }

    if (indexEntrySize > getFreeSpaceofIndexPage(insertPage)) {
        return NOT_ENOUGH_SPACE;
    }

    int i = 0;
    for(; i< indexHeader.numberOfEntries; i++) {
        if (compareIndexKey(insertPage, attribute, dummyEntry.key, i) < 0){
            break;
        }
    }
    memmove((char*) insertPage + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * (i + 1),
            (char*) insertPage + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * i,
            sizeof(NoLeafIndexEntry) * (indexHeader.numberOfEntries - i));

    NoLeafIndexEntry toBeInsertEntry;
    toBeInsertEntry.childPageId = dummyEntry.childPage;
    if (attribute.type == TypeInt) {
        memcpy(&toBeInsertEntry.intkey, dummyEntry.key, INT_SIZE);
    }
    else if (attribute.type == TypeReal) {
        memcpy(&toBeInsertEntry.realkey, dummyEntry.key, REAL_SIZE);
    }
    else {
        int32_t leng;
        memcpy(&leng, dummyEntry.key, INT_SIZE);
        toBeInsertEntry.charKeyOffset = indexHeader.pointerToFreeSpace  - INT_SIZE - leng;
        memcpy((char *)insertPage + toBeInsertEntry.charKeyOffset, dummyEntry.key, leng);
        indexHeader.pointerToFreeSpace -= INT_SIZE + leng;

    }
    indexHeader.numberOfEntries += 1;
    memcpy((char *) insertPage + sizeof(NodeType), &indexHeader, sizeof(NoLeafIndexHeader));
    memcpy((char *) insertPage + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * i,
            &toBeInsertEntry, sizeof(NoLeafIndexEntry));

    return SUCCESS;
}


int32_t IndexManager::getChildPage(void *pageData, const Attribute attribute, const void *key) {
    NoLeafIndexHeader header;
    memcpy(&header, (char *) pageData + sizeof(NodeType), sizeof(NoLeafIndexHeader));
    if (key == NULL) {
        return header.leftMostPointer;
    }
    int i = 0;
    //cout << "number of entries" <<header.numberOfEntries << endl;
    for (; i < header.numberOfEntries; i++) {
        //if < go left most children page store in page,
        // if > search to right, until < , go previous one childpage, if no < to the end, go cur one child page
        // if ==, go down  to its own page
        if (compareIndexKey(pageData, attribute, key, i) <= 0) {
            break;
            // key < curkey
            /*if (i == 0) {
               int32_t childPageId = header.leftMostPointer;
               return childPageId;
            }
            else {
                //previous entry
                NoLeafIndexEntry indexEntry;
                int32_t offset = sizeof(NoLeafIndexHeader) + sizeof(NodeType);
                memcpy(&indexEntry, (char *)pageData + offset + sizeof(NoLeafIndexEntry) * (i-1), sizeof(NoLeafIndexEntry));
                return indexEntry.childPageId;
            }
        }
        else {
            continue;
        }*/
        }
    }
    int32_t result;
    // Special case where key is less than all entries in this node
    if (i == 0)
    {
        result = header.leftMostPointer;
    }
        // Usual case where we grab the child to the right of the largest entry less than key
        // This also works if key is larger than all entries
    else
    {
        NoLeafIndexEntry entry = getIndexEntry(i - 1, pageData);
        result = entry.childPageId;
    }
    return result;
    /*
    if (i == 0) {
        int32_t childPageId = header.leftMostPointer;
        return childPageId;
    }
    NoLeafIndexEntry lastEntry;
    int32_t offset = sizeof(NoLeafIndexHeader) + sizeof(NodeType);
    memcpy(&lastEntry, (char *)pageData + offset + sizeof(NoLeafIndexEntry) * (i), sizeof(NoLeafIndexEntry));
    return lastEntry.childPageId;*/
}


int IndexManager::compareIndexKey(void *pageData, const Attribute attribute, const void *key, const int i){
    NoLeafIndexEntry entry;
    memcpy(&entry, (char *)pageData + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * (i), sizeof(NoLeafIndexEntry));
    if (attribute.type == TypeInt) {
        int32_t int_key;
        memcpy(&int_key, key, INT_SIZE);
        return compare(int_key, entry.intkey);
    }
    else if (attribute.type == TypeReal) {
        float float_key;
        memcpy(&float_key, key, REAL_SIZE);
        return compare(float_key, entry.realkey);
    }
    else {
        int32_t leng;
        memcpy(&leng, key, INT_SIZE);
        char keyString[leng + 1];
        keyString[leng] ='\0';
        memcpy(keyString, (char*) key + INT_SIZE, leng);

        int32_t entryLeng;
        memcpy(&entryLeng, (char *) pageData + entry.charKeyOffset, INT_SIZE);
        char entryKeyString[entryLeng + 1];
        entryKeyString[entryLeng] = '\0';
        memcpy(entryKeyString, (char *) pageData + entry.charKeyOffset + INT_SIZE, entryLeng);
        return compare(keyString, entryKeyString);
    }
}

int IndexManager::compare(const int key, const int curKey){
    if (key > curKey) return 1;
    else if (key == curKey) return 0;
    else return -1;
}

int IndexManager::compare(const float key, const float curKey) {
    if (key > curKey) return 1;
    else if (key == curKey) return 0;
    else return -1;
}

int IndexManager::compare(const char *key, const char *curKey){
    return strcmp(key, curKey);

}

RC IndexManager::getRootPageNum(IXFileHandle &fileHandle, int32_t &result) const
{
    void *metaPage = malloc(PAGE_SIZE);
    if (metaPage == NULL)
        return IX_MALLOC_FAILED;
    RC rc = fileHandle.readPage(0, metaPage);
    if (rc)
    {
        free(metaPage);
        return IX_READ_FAILED;
    }

    MetapageHeader header = getMetaData(metaPage);
    free(metaPage);
    result = header.rootPageNumber;
    return SUCCESS;
}


RC IndexManager::deleteEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid)
{
    //get the rootPage, and search from rootPage.
    int32_t leafPage;
    int32_t rootPageNum;
    RC rc = getRootPageNum(ixfileHandle, rootPageNum);
    if (rc) return rc;

    //traverse the whole tree from rootPage to find the entry with same key and rid.
    rc = findLeafPage(ixfileHandle, attribute, key, rootPageNum, leafPage);

    //if we could not find the page, return an error
    if (rc) return rc;

    //read page from disk to the memory, waiting for deleting operation, if the reading is not successful, return an error
    void *pageData = malloc(PAGE_SIZE);
    if (ixfileHandle.readPage(leafPage, pageData)) {
        free(pageData);
        return IX_READ_FAILED;
    }

    // Delete the entry from pageData, if deleting is not successful, return an error
    rc = deleteEntryFromLeaf(attribute, key, rid, pageData);
    if (rc)
    {
        free(pageData);
        return rc;
    }

    //write the page that has completed deleting in memory to the disk
    rc = ixfileHandle.writePage(leafPage, pageData);
    free(pageData);
    return rc;
}


RC IndexManager::scan(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *lowKey, const void *highKey,
        bool lowKeyInclusive, bool highKeyInclusive, IX_ScanIterator &ix_ScanIterator)
{
    return ix_ScanIterator.initialize(ixfileHandle, attribute, lowKey, highKey, lowKeyInclusive, highKeyInclusive);
}

void IndexManager::printBtree(IXFileHandle &ixfileHandle, const Attribute &attribute) const
{
    //this is the given standardized function, and we have an assisting function to assist this function
    int32_t root;
    getRootPageNum(ixfileHandle, root);
    cout << "{";
    printBtree_rec(ixfileHandle, "  ", root, attribute);
    cout << endl << "}" << endl;
}

void IndexManager::printBtree_rec(IXFileHandle &ixfileHandle, string prefix, const int32_t currPage, const Attribute &attr) const
{
    //allocate a page space for the read of currentpage.
    void *pageData = malloc(PAGE_SIZE);
    ixfileHandle.readPage(currPage, pageData);

    //if currrent page is a leaf, then we call the printLeafNode function, else if currrent page is an internal node, then we call the printInternalNode function
    NodeType type = getNodetype(pageData);
    if (type == LeafType) printLeafNode(pageData, attr);
    else printInternalNode(ixfileHandle, pageData, attr, prefix);
    free(pageData);
}

NodeType IndexManager::getNodetype(const void *pageData) const
{
    NodeType result;
    memcpy(&result, pageData, sizeof(NodeType));
    return result;
}


void IndexManager::printLeafNode(void *pageData, const Attribute &attr) const
{
    //get leaf node header information including previous page, next page, number of entries and free space offset.
    LeafHeader header = getLeafHeader(pageData);

    //allocate a space for key for later use.
    void *key = NULL;
    if (attr.type != TypeVarChar) key = malloc (INT_SIZE);
    bool tag = true;
    vector<RID> key_rids;

    //use loop to traverse the entries in the leaf node
    cout << "\"keys\":[";
    for (int i = 0; i <= header.numberOfEntry; i++) {
        //get the ith entry in the pageData
        DataEntry entry = getDataEntry(i, pageData);

        //for the first time, we need to clear the key_rids.
        if (tag && i < header.numberOfEntry) {
            key_rids.clear();
            tag = false;
            if (attr.type == TypeInt) memcpy(key, &(entry.intkey), INT_SIZE);
            else if (attr.type == TypeReal) memcpy(key, &(entry.realkey), REAL_SIZE);
            else {
                int len;
                memcpy(&len, (char*)pageData + entry.charKeyOffset, VARCHAR_LENGTH_SIZE);
                free(key);
                key = malloc(len + VARCHAR_LENGTH_SIZE + 1);
                memcpy(key, &len, VARCHAR_LENGTH_SIZE);
                memcpy((char*)key + VARCHAR_LENGTH_SIZE, (char*)pageData + entry.charKeyOffset + VARCHAR_LENGTH_SIZE, len);
                memset((char*)key + VARCHAR_LENGTH_SIZE + len, 0, 1);
            }
        }


        //after the first time, we print the child page one by one with increasing i.
        if ( i < header.numberOfEntry&& compareWithOtherLeafEntry(key, attr, i, pageData) == 0)
            key_rids.push_back(entry.rid);
        else if (i != 0) {
            cout << "\"";
            if (attr.type == TypeInt) {
                cout << "" << *(int*)key;
                memcpy(key, &(entry.intkey), INT_SIZE);
            } else if (attr.type == TypeReal) {
                cout << "" << *(float*)key;
                memcpy(key, &(entry.realkey), REAL_SIZE);
            } else {
                cout << (char*)key + 4;
                int len;
                memcpy(&len, (char*)pageData + entry.charKeyOffset, VARCHAR_LENGTH_SIZE);
                free(key);
                key = malloc(len + VARCHAR_LENGTH_SIZE + 1);
                memcpy(key, &len, VARCHAR_LENGTH_SIZE);
                memcpy((char*)key + VARCHAR_LENGTH_SIZE, (char*)pageData + entry.charKeyOffset + VARCHAR_LENGTH_SIZE, len);
                memset((char*)key + VARCHAR_LENGTH_SIZE + len, 0, 1);
            }
            cout << ":[";
            for (unsigned j = 0; j < key_rids.size(); j++) {
                if (j != 0) {
                    cout << ",";
                }
                cout << "(" << key_rids[j].pageNum << "," << key_rids[j].slotNum << ")";
            }
            cout << "]\"";
            key_rids.clear();
            key_rids.push_back(entry.rid);
        }
    }
    cout << "]}";
    free (key);
}

void IndexManager::printInternalNode(IXFileHandle &ixfileHandle, void *pageData, const Attribute &attr, string prefix) const
{
    //print internal node can be separated into print internalkey and print internalrid, thus here we call these two functions sequentially.
    printInternalNodeKey(ixfileHandle, pageData, attr, prefix);
    printInternalNodeChild(ixfileHandle, pageData, attr, prefix);
}

void IndexManager::printInternalNodeKey(IXFileHandle &ixfileHandle, void *pageData, const Attribute &attr, string prefix) const
{
    //get the internal header to know the information including number of entries, free space offset and left child page number.
    NoLeafIndexHeader header = getInternalHeader(pageData);

    //use loop to print all the keys by traversing the index entry. there are three cases when printing the keys: int, float and varchar.
    cout << "\n" << prefix << "\"keys\":[";
    for (int num = 0; num < header.numberOfEntries; num++)
    {
        if (num != 0) cout << ",";
        NoLeafIndexEntry entry = getIndexEntry(num, pageData);
        if (attr.type == TypeInt) cout << "" << entry.intkey;
        else if (attr.type == TypeReal) cout << "" << entry.realkey;
        else {
            int32_t len;
            memcpy(&len, (char*)pageData + entry.charKeyOffset, VARCHAR_LENGTH_SIZE);
            char varchar[len + 1];
            varchar[len] = '\0';
            memcpy(varchar, (char*)pageData + entry.charKeyOffset + VARCHAR_LENGTH_SIZE, len);
            cout << "" << varchar;
        }
    }
    cout << "],\n" << prefix;
}

void IndexManager::printInternalNodeChild(IXFileHandle &ixfileHandle, void *pageData, const Attribute &attr, string prefix) const
{
    //get the internal header to know the information including number of entries, free space offset and left child page number.
    NoLeafIndexHeader header = getInternalHeader(pageData);

    //print the first left child page by calling the header.leftChildPage.
    cout<< "\"children\":[\n" << prefix;
    cout << "{";
    printBtree_rec(ixfileHandle, prefix + "  ", header.leftMostPointer, attr);
    cout << "}";

    //use loop and recursion to print all the other childPages by calling the printBtree_rec and traversing the index entry.
    for (int num = 1; num <= header.numberOfEntries; num++) {
        cout << ",\n" << prefix;
        NoLeafIndexEntry entry = getIndexEntry(num - 1, pageData);
        cout << "{";
        printBtree_rec(ixfileHandle, prefix + "  ", entry.childPageId, attr);
        cout << "}";
    }
    cout << "\n" << prefix << "]";
}


DataEntry IndexManager::getDataEntry(const int slotNum, const void *pageData) const
{
    const unsigned offset = sizeof(NodeType) + sizeof(LeafHeader);
    unsigned slotOffset = offset + slotNum * sizeof(DataEntry);
    DataEntry entry;
    memcpy(&entry, (char*)pageData + slotOffset, sizeof(DataEntry));
    return entry;
}

NoLeafIndexHeader IndexManager::getInternalHeader(const void *pageData) const
{
    const unsigned offset = sizeof(NodeType);
    NoLeafIndexHeader header;
    memcpy(&header, (char*)pageData + offset, sizeof(NoLeafIndexHeader));
    return header;
}





IX_ScanIterator::IX_ScanIterator()
{
}

IX_ScanIterator::~IX_ScanIterator()
{
}


RC IX_ScanIterator::initialize(IXFileHandle &fh, Attribute attribute, const void *low, const void *high, bool lowInc, bool highInc)
{
    //get the parameter and allocate some space for later use
    attr = attribute;
    fileHandle = &fh;
    lowKey = low;
    highKey = high;
    lowKeyInclusive = lowInc;
    highKeyInclusive = highInc;
    page = malloc(PAGE_SIZE);
    if (page == NULL)
        return IX_MALLOC_FAILED;

    //initialize the IndexManager and find the starting page number and the entry number within by traversing the tree from rootPage.
    slotNum = 0;
    IndexManager *im = IndexManager::instance();
    int32_t startPageNum;
    int32_t rootPageNum;
    RC res = im->getRootPageNum(fh, rootPageNum);
    if (res) return res;
    res = im->findLeafPage(*fileHandle, attr, lowKey, rootPageNum, startPageNum);
    if (res) {
        free(page);
        return res;
    }
    res = fileHandle->readPage(startPageNum, page);
    if (res) {
        free(page);
        return res;
    }
    LeafHeader header = im->getLeafHeader(page);
    int num = 0;
    for (num = 0; num < header.numberOfEntry; num++) {
        int cmp;
        if (low == NULL) cmp = -1;
        else cmp =im->compareWithOtherLeafEntry(lowKey, attr, num, page);
        if (cmp < 0 || (cmp == 0 && lowKeyInclusive)) break;
    }
    slotNum = num;
    return SUCCESS;
}

RC IX_ScanIterator::getNextEntry(RID &rid, void *key)
{
    //initialize an IndexManager and get the leaf header information, includingprevious page, next page, number of entries and free space offset.
    IndexManager *im = IndexManager::instance();
    LeafHeader header = im->getLeafHeader(page);

    //if we run to end of the page, jump to the next page.
    if (slotNum >= header.numberOfEntry) {
        if (header.nextPointer == 0) return IX_EOF;
        slotNum = 0;
        fileHandle->readPage(header.nextPointer, page);
        return getNextEntry(rid, key);
    }

    //as in the instruction, when the highkey is null, it is equal to the infinite large, thus we will go on, else we will compare the value of highkey and the traversed key.
    int cmp;
    if (highKey == NULL) cmp = 1;
    else cmp = im->compareWithOtherLeafEntry(highKey, attr, slotNum, page);
    if (cmp == 0 && !highKeyInclusive) return IX_EOF;
    if (cmp < 0) return IX_EOF;

    //get the entry key and rid and store it.
    DataEntry entry = im->getDataEntry(slotNum, page);
    if (attr.type == TypeInt) memcpy(key, &(entry.intkey), INT_SIZE);
    else if (attr.type == TypeReal) memcpy(key, &(entry.realkey), REAL_SIZE);
    else {
        int len;
        memcpy(&len, (char*)page + entry.charKeyOffset, VARCHAR_LENGTH_SIZE);
        memcpy(key, &len, VARCHAR_LENGTH_SIZE);
        memcpy((char*)key + VARCHAR_LENGTH_SIZE, (char*)page + entry.charKeyOffset + VARCHAR_LENGTH_SIZE, len);
    }
    rid.pageNum = entry.rid.pageNum;
    rid.slotNum = entry.rid.slotNum;
    slotNum++;
    return SUCCESS;
}

RC IX_ScanIterator::close()
{
    free(page);
    return SUCCESS;
}

RC IndexManager::deleteEntryFromLeaf(const Attribute attr, const void *key, const RID &rid, void *pageData)
{
    //get the leaf header from given leaf page, including information about previous page, next page, number of entries and free space offset
    LeafHeader header = getLeafHeader(pageData);

    //use loop to find the entry whose key and rid are equal to the given key and rid
    int num;
    for (num = 0; num < header.numberOfEntry; num++)
    {
        if(compareWithOtherLeafEntry(key, attr, num, pageData) == 0)
        {
            DataEntry entry = getDataEntry(num, pageData);
            if (entry.rid.pageNum == rid.pageNum && entry.rid.slotNum == rid.slotNum)
            {
                break;
            }
        }
    }

    // If this entry does not exist, then we return an error
    if (num == header.numberOfEntry)
    {
        return -1;
    }

    //get the position of where the entry starts and where it ends, and then move all the following entries forward, that is, delete the given entry and re-organize the index. If the key type is a varchar, we need move them too.
    DataEntry entry = getDataEntry(num, pageData);
    unsigned slotStartOffset = sizeof(NodeType) + sizeof(LeafHeader) + num * sizeof(DataEntry);
    unsigned slotEndOffset = sizeof(NodeType) + sizeof(LeafHeader) + header.numberOfEntry* sizeof(DataEntry);
    memmove((char*)pageData + slotStartOffset, (char*)pageData + slotStartOffset + sizeof(DataEntry), slotEndOffset - slotStartOffset - sizeof(DataEntry));
    header.numberOfEntry -= 1;
    if (attr.type == TypeVarChar)
    {
        int32_t varcharOffset = entry.charKeyOffset;
        int32_t varchar_len;
        memcpy(&varchar_len, (char*)pageData + varcharOffset, VARCHAR_LENGTH_SIZE);
        int32_t entryLen = varchar_len + VARCHAR_LENGTH_SIZE;
        memmove((char*)pageData + header.pointerToFreeSpace + entryLen, (char*)pageData + header.pointerToFreeSpace, varcharOffset - header.pointerToFreeSpace);
        header.pointerToFreeSpace += entryLen;
        for (num = 0; num < header.numberOfEntry; num++)
        {
            entry = getDataEntry(num, pageData);
            if (entry.charKeyOffset < varcharOffset)
                entry.charKeyOffset += entryLen;
            setDataEntry(entry, num, pageData);
        }
    }

    //reset the LeafHeader, put the new header information into pageData.
    setLeafHeader(header, pageData);
    return SUCCESS;
}

void IndexManager::setLeafHeader(const LeafHeader header, void *pageData)
{
    const unsigned offset = sizeof(NodeType);
    memcpy((char*)pageData + offset, &header, sizeof(LeafHeader));
}

void IndexManager::setDataEntry(const DataEntry entry, const int slotNum, void *pageData)
{
    const unsigned offset = sizeof(NodeType) + sizeof(LeafHeader);
    unsigned slotOffset = offset + slotNum * sizeof(DataEntry);
    memcpy((char*) pageData + slotOffset, &entry, sizeof(DataEntry));
}

LeafHeader IndexManager::getLeafHeader(const void *pageData) const
{
    const unsigned offset = sizeof(NodeType);
    LeafHeader header;
    memcpy(&header, (char*)pageData + offset, sizeof(LeafHeader));
    return header;
}


RC IndexManager::findLeafPage(IXFileHandle &handle, const Attribute attr, const void *key, const int32_t currPageNum, int32_t &resultPageNum)
{
    //re-allocate the page data
    void *pageData = malloc(PAGE_SIZE);
    if (handle.readPage(currPageNum, pageData)) {
        free (pageData);
        return IX_READ_FAILED;
    }

    //check if it is the leaf page
    if (getNodetype(pageData) == LeafType) {
        resultPageNum = currPageNum;
        free(pageData);
        return SUCCESS;
    }

    //get the next child page to do the recursion
    int32_t nextChildPage = getChildPage(pageData, attr, key);
    free(pageData);
    return findLeafPage(handle, attr, key, nextChildPage, resultPageNum);
}

MetapageHeader IndexManager::getMetaData(const void *pageData) const
{
    MetapageHeader header;
    memcpy(&header, pageData, sizeof(MetapageHeader));
    return header;
}

IXFileHandle::IXFileHandle()
{
    ixReadPageCounter = 0;
    ixWritePageCounter = 0;
    ixAppendPageCounter = 0;
}

IXFileHandle::~IXFileHandle()
{
}

RC IXFileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
    readPageCount = ixReadPageCounter;
    writePageCount = ixWritePageCounter;
    appendPageCount = ixAppendPageCounter;
    return SUCCESS;
}

// Get a specific page
RC IXFileHandle::readPage(PageNum pageNum, void *data){
    ixReadPageCounter++;
    return fileHandle.readPage(pageNum, data);
}

// Write a specific page
RC IXFileHandle::writePage(PageNum pageNum, const void *data){
    ixWritePageCounter++;
    return fileHandle.writePage(pageNum, data);
}

// Append a specific page
RC IXFileHandle::appendPage(const void *data){
    ixAppendPageCounter++;
    return fileHandle.appendPage(data);

}


unsigned IXFileHandle::getNumberOfPages(){
    return fileHandle.getNumberOfPages();
}

/*
//configure a new index page, and puts it in 'page'
void IndexManager::newIndexPage(void * page){
    memset(page, 0, PAGE_SIZE);
    //index page header


}*/

RC IndexManager::deleteIndexEntry(const Attribute attribute, const void *key, void *pageData)
{
    NoLeafIndexHeader header;
    memcpy(&header, (char *) pageData + sizeof(NodeType), sizeof(NoLeafIndexHeader));

    int i;
    for (i = 0; i < header.numberOfEntries; i++)
    {
        // Scan through until we find a matching key
        if(compareIndexKey(pageData, attribute, key,  i) == 0)
        {
            break;
        }
    }
    if (i == header.numberOfEntries)
    {
        // error out if no match
        return -1;
    }

    NoLeafIndexEntry entry;
    int32_t offset = sizeof(NoLeafIndexHeader) + sizeof(NodeType);
    memcpy(&entry, (char *)pageData + offset + sizeof(NoLeafIndexEntry) * (i), sizeof(NoLeafIndexEntry));

    memmove((char*)pageData + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * i,
            (char*)pageData + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * (i+1),
             sizeof(NoLeafIndexEntry) *(header.numberOfEntries - i - 1));

    header.numberOfEntries -= 1;

    if (attribute.type == TypeVarChar)
    {
        int32_t charKeyOffset = entry.charKeyOffset;
        int32_t varLen;
        memcpy(&varLen, (char*)pageData + charKeyOffset, INT_SIZE);
        int32_t entryLen = varLen + INT_SIZE;

        memmove((char*)pageData + header.pointerToFreeSpace + entryLen, (char*)pageData + header.pointerToFreeSpace, charKeyOffset - header.pointerToFreeSpace);
        header.pointerToFreeSpace += entryLen;

        for (i = 0; i < header.numberOfEntries; i++)
        {
            int32_t offset = sizeof(NoLeafIndexHeader) + sizeof(NodeType);
            memcpy(&entry, (char *)pageData + offset + sizeof(NoLeafIndexEntry) * (i), sizeof(NoLeafIndexEntry));

            if (entry.charKeyOffset < charKeyOffset)
                entry.charKeyOffset += entryLen;

            memcpy((char*)pageData + sizeof(NodeType) + sizeof(NoLeafIndexHeader) + sizeof(NoLeafIndexEntry) * i,
                   &entry,
                    sizeof(NoLeafIndexEntry));
        }
    }
    memcpy((char *) pageData + sizeof(NodeType), &header, sizeof(NoLeafIndexHeader));

    return SUCCESS;
}
    NoLeafIndexEntry IndexManager::getIndexEntry(const int slotNum, const void *pageData) const {
        return NoLeafIndexEntry();
    }
