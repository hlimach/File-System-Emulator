#include "../include/sema.h"
#include "../include/file.h"
#include "../include/globals.h"
#include "../include/mem_struct.h"



/* insert thread number and 1/0 to a vector and push it in fileQue*/
void
enque(FileNode* fNode,int threadNo, string mode)
{
    vector<int> temp;
    temp.push_back(threadNo);
    
    if(mode == "write")
        temp.push_back(1);
    
    else
        temp.push_back(0);
    
   
    queMtx.lock(); 
    fNode->fileQue.push_back(temp);
    queMtx.unlock();

}


/* dequeue the fileQue and remove the top element depending on reader or writer*/
void
dque(FileNode* fNode)
{
    if(fNode->fileQue.front()[1] == 0)
    {
        if(fNode->numReaders == 0)
            sem_wait(&fNode->writer_sema);

        fNode->numReaders++;
    }
    else
    {
        sem_wait(&fNode->writer_sema);
    }
    
    fNode->fileQue.erase(fNode->fileQue.begin());
}


/* enqueue the thread, wait for to be on top then dequeue the thread */
void
enterFile(FileNode* fNode, int threadNo, string mode)
{
    cout << threadNo << ": 1" << endl; 
    enque(fNode, threadNo,mode);
    cout << threadNo << ": 2" << endl; 

    // fNode->fileQue.front().front() returns the threadNo of top element
    while(fNode->fileQue[0][0] != threadNo);
    cout << threadNo << ": 3" << endl; 

    dque(fNode);      
    cout << threadNo << ": 4" << endl;
    cout << "queue size: " << fNode->fileQue.size() << endl; 

}


/* calls sem_post while closing file when its the last reader of the set 
   or any writer */
void
leaveFile(FileNode* fNode,string mode)
{
    if (mode == "read")
	{
		if(fNode->numReaders == 1)
			sem_post(&(fNode->writer_sema));

		fNode->numReaders--;		
	}
	else
		sem_post(&(fNode->writer_sema));
}