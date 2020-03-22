/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include <memory>
#include <iostream>
#include "buffer.h"
#include "exceptions/buffer_exceeded_exception.h"
#include "exceptions/page_not_pinned_exception.h"
#include "exceptions/page_pinned_exception.h"
#include "exceptions/bad_buffer_exception.h"
#include "exceptions/hash_not_found_exception.h"

namespace badgerdb { 

BufMgr::BufMgr(std::uint32_t bufs)
	: numBufs(bufs) {
	bufDescTable = new BufDesc[bufs];

  for (FrameId i = 0; i < bufs; i++) 
  {
  	bufDescTable[i].frameNo = i;
  	bufDescTable[i].valid = false;
  }

  bufPool = new Page[bufs];

	int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
  hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

  clockHand = bufs - 1;
}

#This method has been modified.
BufMgr::~BufMgr() {
# flush out all dirty pages
    for (FrameId i = 0; i < bufs; ++i){
        if(bufDescTable[i].dirty == true){
            file* file = bufDescTable[i].file;
            PageId pageNo = bufDescTable[i].pageNo;
            Page newPage = bufPool[i];
            file -> writePage(newPage);
            bufDescTable[i].dirty = false;
            hashTable -> remove(file, pageNo);
            bufDescTable[i].clear();
            bufStats.diskwrites++;
        }
    }
# deallocates the buffer pool
	delete [] bufPool;
# deallocates the bufDescTable
    delete [] bufDescTable;
# deallocates the hashTable
    delete hashTable;
}

#This method has been modified.
void BufMgr::advanceClock()
{
    clockHand = (clockHand + 1) % numBufs;
}

void BufMgr::allocBuf(FrameId & frame) 
{
    FrameId initialClockHand = clockHand;
# check the frame pointed to by the intial value of clockHand
    if (bufDescTable[clockHand].valid == false){
        frame = clockHand;
    }
    else{
        if(bufDescTable[clockHand].refbit == true){
            bufDescTable[clockHand].refbit = false;
            advanceClock();
        }
        else{
            if(bufDescTable[clockHand].pinCnt > 0){
                advanceClock();
            }
            else{
                if(bufDescTable[clockHand].dirty == false){
                    frame = clockHand;
                }
                else{
                    file* file = bufDescTable[clockHand].file;
                    PageId pageNo = bufDescTable[clockHand].pageNo;
                    Page newPage = bufPool[clockHand];
                    file -> writePage(newPage);
                    hashTable -> remove(file, pageNo);
                    bufDescTable[clockHand].clear();
                    bufStats.diskwrites++;
                }
            }
        }
    }
            
    
}

	
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
}


void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
}

void BufMgr::flushFile(const File* file) 
{
}

void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
}

void BufMgr::disposePage(File* file, const PageId PageNo)
{
    
}

void BufMgr::printSelf(void) 
{
  BufDesc* tmpbuf;
	int validFrames = 0;
  
  for (std::uint32_t i = 0; i < numBufs; i++)
	{
  	tmpbuf = &(bufDescTable[i]);
		std::cout << "FrameNo:" << i << " ";
		tmpbuf->Print();

  	if (tmpbuf->valid == true)
    	validFrames++;
  }

	std::cout << "Total Number of Valid Frames:" << validFrames << "\n";
}

}
