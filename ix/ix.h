#ifndef _ix_h_
#define _ix_h_

#include <vector>
#include <string>

#include "../rbf/rbfm.h"
#include "../rbf/pfm.h"

#define LeafType 1
#define IndexType 0

#define INT_SIZE                4
#define REAL_SIZE               4

# define IX_EOF (-1)  // end of the index scan
#define SUCCESS 0

#define IX_CREATE_FAILED  1
#define IX_MALLOC_FAILED  2
#define IX_OPEN_FAILED    3
#define IX_APPEND_FAILED  4
#define IX_DESTORY_FAILED 5
#define IX_CLOSE_FAILED   6
#define IX_READ_FAILED    7
#define IX_INSERT_FAILED  8
#define IX_WRITE_FAILED   9
#define NOT_ENOUGH_SPACE 10


typedef char NodeType; //one byte to differeniate leaf or index

//for recursive insert
typedef struct DummyEntry {
    void *key;
    uint32_t childPage;

} DummyEntry;

typedef struct LeafHeader {
    uint32_t prevPointer;
    uint32_t nextPointer;
    uint16_t numberOfEntry;
    uint16_t pointerToFreeSpace; //i used 32, ?? 16

} LeafHeader;

typedef struct DataEntry {
    union {
      int32_t intkey;
      float realkey;
      uint32_t charKeyOffset;
    };
    RID rid;
} DataEntry;

typedef struct NoLeafIndexHeader{
    uint16_t numberOfEntries;
    uint32_t pointerToFreeSpace; //i used 32, ?? 16
    uint32_t leftMostPointer; //one more pointer in an index page, ref to lecture 4 page 5
} NoLeafIndexHeader;

//lecture 4 page 5, index entry has two parts, key and pointer
typedef struct NoLeafIndexEntry {
    union {
        int32_t intkey;
        float realkey;
        int32_t charKeyOffset;
    };
    uint32_t childPageId;
} NoLeafIndexEntry;

//metapage, pointer to rootpage
typedef struct MetapageHeader {
    uint32_t rootPageNumber;
} MetapageHeader;



class IX_ScanIterator;
class IXFileHandle;

class IndexManager {

    public:
        static IndexManager* instance();

        // Create an index file.
        RC createFile(const string &fileName);

        // Delete an index file.
        RC destroyFile(const string &fileName);

        // Open an index and return an ixfileHandle.
        RC openFile(const string &fileName, IXFileHandle &ixfileHandle);

        // Close an ixfileHandle for an index.
        RC closeFile(IXFileHandle &ixfileHandle);

        // Insert an entry into the given index that is indicated by the given ixfileHandle.
        RC insertEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid);

        // Delete an entry from the given index that is indicated by the given ixfileHandle.
        RC deleteEntry(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid);

        // Initialize and IX_ScanIterator to support a range search
        RC scan(IXFileHandle &ixfileHandle,
                const Attribute &attribute,
                const void *lowKey,
                const void *highKey,
                bool lowKeyInclusive,
                bool highKeyInclusive,
                IX_ScanIterator &ix_ScanIterator);

        // Print the B+ tree in pre-order (in a JSON record format)
        void printBtree(IXFileHandle &ixfileHandle, const Attribute &attribute) const;

    NoLeafIndexEntry getIndexEntry(const int slotNum, const void *pageData) const;

public:
        friend class IX_ScanIterator;

    protected:
        IndexManager();
        ~IndexManager();

    private:
        static IndexManager *_index_manager;
        static PagedFileManager *_pf_manager;

    //private helper methods
        //void newIndexPage(void * page)

    int32_t getChildPage(void *pageData, const Attribute attribute, const void *key);

    RC insert(IXFileHandle &ixfileHandle, const Attribute &attribute, const void *key, const RID &rid, int32_t pageID,
              DummyEntry &dummyEntry);

    int compare(const int key, const int curKey);

    int compare(const float key, const float curKey);

    int compare(const char *key, const char *curKey);

    int compareIndexKey(void *pageData, const Attribute attribute, const void *key, const int i);

    RC insertToLeafPage(void *insertPage, const Attribute attribute, const void *key, const RID rid);


    int getFreeSpaceofIndexPage(const void *pageData);

    RC insertToInnerIndexPage(void * insertPage, const Attribute attribute, DummyEntry dummyEntry);

    int compareWithOtherLeafEntry(const void *key, const Attribute attribute, const int i, const void *insertPage);

    RC splitLeaf(IXFileHandle &ixFileHandle, const Attribute &attribute, const void *key, const RID rid, void *pageData,
                 int32_t pageID, DummyEntry &dummyEntry);

    RC splitInnerIndex(IXFileHandle &ixfileHandle, const Attribute &attribute, void *pageData, int32_t pageID,
                       DummyEntry &dummyEntry);

    RC deleteIndexEntry(const Attribute attribute, const void *key, void *pageData);


    LeafHeader getLeafHeader(const void *pageData);


    int getFreeSpaceLeaf(void *pageData);

    void printBtree_rec(IXFileHandle &ixfileHandle, string prefix, const int32_t currPage, const Attribute &attr);

    NodeType getNodetype(const void *pageData);

    DataEntry getDataEntry(const int slotNum, const void *pageData);

    RC findLeafPage(IXFileHandle &handle, const Attribute attr, const void *key, const int32_t currPageNum,
                    int32_t &resultPageNum);

    RC getRootPageNum(IXFileHandle &fileHandle, int32_t &result);

    MetapageHeader getMetaData(const void *pageData);

    RC deleteEntryFromLeaf(const Attribute attr, const void *key, const RID &rid, void *pageData);

    void setLeafHeader(const LeafHeader header, void *pageData);

    void setDataEntry(const DataEntry entry, const int slotNum, void *pageData);

    NoLeafIndexHeader getInternalHeader(const void *pageData);

    void printInternalNodeChild(IXFileHandle &ixfileHandle, void *pageData, const Attribute &attr, string prefix);

    void printInternalNodeKey(IXFileHandle &ixfileHandle, void *pageData, const Attribute &attr, string prefix);

    void printInternalNode(IXFileHandle &ixfileHandle, void *pageData, const Attribute &attr, string prefix);

    void printLeafNode(void *pageData, const Attribute &attr);
};


class IX_ScanIterator {
    public:

		// Constructor
        IX_ScanIterator();

        // Destructor
        ~IX_ScanIterator();

        // Get next matching entry
        RC getNextEntry(RID &rid, void *key);

        // Terminate index scan
        RC close();

        friend class IndexManager;
    private:
        IXFileHandle *fileHandle;
        Attribute attr;
        const void *lowKey;
        const void *highKey;
        bool lowKeyInclusive;
        bool highKeyInclusive;


        void *page;
        int slotNum;


    RC initialize(IXFileHandle &fh, Attribute attribute, const void *low, const void *high, bool lowInc, bool highInc);
};



class IXFileHandle {
    public:

    // variables to keep counter for each operation
    unsigned ixReadPageCounter;
    unsigned ixWritePageCounter;
    unsigned ixAppendPageCounter;

    // Constructor
    IXFileHandle();

    // Destructor
    ~IXFileHandle();

	// Put the current counter values of associated PF FileHandles into variables
	RC collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount);
    //helper
    RC readPage(PageNum pageNum, void *data);                           // Get a specific page
    RC writePage(PageNum pageNum, const void *data);                    // Write a specific page
    RC appendPage(const void *data);                               // Append a specific page
    unsigned getNumberOfPages();

    friend class IndexManager;
private:
    FileHandle fileHandle;


};

#endif
