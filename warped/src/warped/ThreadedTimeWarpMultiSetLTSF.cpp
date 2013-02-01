#include "ThreadedTimeWarpMultiSetLTSF.h"

ThreadedTimeWarpMultiSetLTSF::ThreadedTimeWarpMultiSetLTSF(int objectCount, int LTSFCountVal, const string syncMech, const string scheQScheme) {

	//scheduleQ scheme
	scheduleQScheme = scheQScheme;

	// Set up scheduleQueue (LTSF queue)
	if( scheduleQScheme == "MULTILTSF" ) {
		scheduleQueue = new multiset<const Event*,
				receiveTimeLessThanEventIdLessThan> ;
	} else if( scheduleQScheme == "LADDERQ" ) {
		ladderQ = new LadderQueue;
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}

	objectStatusLock = new LockState *[objectCount];
	scheduleQueueLock = new LockState();

	//synchronization mechanism
	syncMechanism = syncMech;

	LTSFCount = LTSFCountVal;

	minReceiveTime = 0;

	//Initialize LTSF Event Queue
	for (int i = 0; i < objectCount; i++) {
		objectStatusLock[i] = new LockState();
		//Schedule queue
		if( scheduleQScheme == "MULTILTSF" ) {
			lowestObjectPosition.push_back(scheduleQueue->end());
		} else if( scheduleQScheme == "LADDERQ" ) {
			lowestLadderObjectPosition.push_back(ladderQ->end());
		} else {
			cout << "Invalid schedule queue scheme" << endl;
		}
	}
}

ThreadedTimeWarpMultiSetLTSF::~ThreadedTimeWarpMultiSetLTSF() {
	delete objectStatusLock;
	delete scheduleQueueLock;
	if( scheduleQScheme == "MULTILTSF" ) {
		delete scheduleQueue;
	} else if ( scheduleQScheme == "LADDERQ" ) {
		delete ladderQ;
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
}

void ThreadedTimeWarpMultiSetLTSF::getScheduleQueueLock(int threadId) {
	utils::debug << "( "<<threadId<<" T ) Getting the Sche Lock." << endl;
	while (!scheduleQueueLock->setLock(threadId, syncMechanism));
	ASSERT(scheduleQueueLock->hasLock(threadId, syncMechanism));
}
void ThreadedTimeWarpMultiSetLTSF::releaseScheduleQueueLock(int threadId) {
	ASSERT(scheduleQueueLock->hasLock(threadId, syncMechanism));
	scheduleQueueLock->releaseLock(threadId, syncMechanism);
	utils::debug << "( "<<threadId<<" T ) Releasing the Sche Lock." << endl;
}
const VTime* ThreadedTimeWarpMultiSetLTSF::nextEventToBeScheduledTime(int threadID) {
	const VTime* ret = NULL;
	this->getScheduleQueueLock(threadID);
	if( scheduleQScheme == "MULTILTSF" ) {
		if (scheduleQueue->size() > 0) {
			ret = &((*scheduleQueue->begin())->getReceiveTime());
		}
	} else if( scheduleQScheme == "LADDERQ" ) {
		if(!ladderQ->empty())
			//cout << "nextEvent" << endl;
			ret = &(ladderQ->begin()->getReceiveTime());
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
	this->releaseScheduleQueueLock(threadID);
	return (ret);
}

// Lock based Counting -- Don't call this function in a loop
int ThreadedTimeWarpMultiSetLTSF::getMessageCount(int threadId) {
	int count = 0;
	getScheduleQueueLock(threadId);

	if( scheduleQScheme == "MULTILTSF" ) {
		count = scheduleQueue->size();
	} else if ( scheduleQScheme == "LADDERQ" ) {
		cout << "LadderQ message count not handled for now" << endl;
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}

	releaseScheduleQueueLock(threadId);
	return count;
}
bool ThreadedTimeWarpMultiSetLTSF::isScheduleQueueEmpty() {
	if( scheduleQScheme == "MULTILTSF" ) {
		return scheduleQueue->empty();
	} else if ( scheduleQScheme == "LADDERQ" ) {
		return ladderQ->empty();
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}	
}

void ThreadedTimeWarpMultiSetLTSF::releaseAllScheduleQueueLocks()
{
	if (scheduleQueueLock->isLocked()) {
		scheduleQueueLock->releaseLock(scheduleQueueLock->whoHasLock(), syncMechanism);
		utils::debug << "Releasing Schedule Queue during recovery." << endl;
	}
}
void ThreadedTimeWarpMultiSetLTSF::clearScheduleQueue(int threadId)
{
	this->getScheduleQueueLock(threadId);
	if( scheduleQScheme == "MULTILTSF" ) {
		scheduleQueue->clear();
	} else if ( scheduleQScheme == "LADDERQ" ) {
		ladderQ->clear();
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
	this->releaseScheduleQueueLock(threadId);
}
void ThreadedTimeWarpMultiSetLTSF::setLowestObjectPosition(int threadId, int index)
{
	this->getScheduleQueueLock(threadId);
	if( scheduleQScheme == "MULTILTSF" ) {
		lowestObjectPosition[index] = scheduleQueue->end();
	} else if ( scheduleQScheme == "LADDERQ" ) {
		lowestLadderObjectPosition[index] = ladderQ->end();
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
	this->releaseScheduleQueueLock(threadId);
}
void ThreadedTimeWarpMultiSetLTSF::insertEvent(int objId, const Event* newEvent)
{
	if( scheduleQScheme == "MULTILTSF" ) {
		ASSERT(newEvent);
		lowestObjectPosition[objId] = scheduleQueue->insert(newEvent);
	} else if ( scheduleQScheme == "LADDERQ" ) {
		ASSERT(newEvent);
		lowestLadderObjectPosition[objId] = ladderQ->insert(newEvent);
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
}
void ThreadedTimeWarpMultiSetLTSF::insertEventEnd(int objId)
{
	if( scheduleQScheme == "MULTILTSF" ) {
		lowestObjectPosition[objId] = scheduleQueue->end();
	} else if ( scheduleQScheme == "LADDERQ" ) {
		lowestLadderObjectPosition[objId] = ladderQ->end();
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
}
void ThreadedTimeWarpMultiSetLTSF::eraseSkipFirst(int objId)
{
	// Do not erase the first time.
	if( scheduleQScheme == "MULTILTSF" ) {
		if (lowestObjectPosition[objId] != scheduleQueue->end()) {
			scheduleQueue->erase(lowestObjectPosition[objId]);
		} else {
			/*utils::debug << "( "
			 << mySimulationManager->getSimulationManagerID()
			 << " ) Object - " << objId
			 << " is returned for schedule thro' insert by the thread - "
			 << threadId << "\n";*/
		}
	} else if ( scheduleQScheme == "LADDERQ" ) {
		if(lowestLadderObjectPosition[objId] != ladderQ->end()) {
			ladderQ->erase(lowestLadderObjectPosition[objId]);
		} else {}
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
}
int ThreadedTimeWarpMultiSetLTSF::getScheduleQueueSize()
{
	if( scheduleQScheme == "MULTILTSF" ) {
		return scheduleQueue->size();
	} else if ( scheduleQScheme == "LADDERQ" ) {
		cout << "LadderQ message count not handled for now" << endl;
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}
}
const Event* ThreadedTimeWarpMultiSetLTSF::peekIt(int threadId, int* LTSFObjId)
{
	const Event* ret = NULL;
	this->getScheduleQueueLock(threadId);

	if( scheduleQScheme == "MULTILTSF" ) {
		if (scheduleQueue->size() > 0) {
			utils::debug<<"( "<< threadId << " T ) Peeking from Schedule Queue"<<endl;

			ret = *(scheduleQueue->begin());
			//cout << "T" << threadId << ": Getting event with timestamp " << ret->getReceiveTime().getApproximateIntTime() << endl;
			unsigned int objId = LTSFObjId[(ret->getReceiver().getSimulationObjectID())];
			utils::debug <<" ( "<< threadId << ") Locking the Object " <<objId <<endl;
	
			this ->getObjectLock(threadId, objId);
			//remove the object out of schedule
			scheduleQueue->erase(scheduleQueue->begin());
			//set the indexer/pointer to NULL
			lowestObjectPosition[objId] = scheduleQueue->end();
			/*		utils::debug << "( "
		 	<< mySimulationManager->getSimulationManagerID()
		 	<< " ) Object - " << objId
		 	<< " is removed out for schedule by the thread - "
		 	<< threadId << "\n";*/
		}
	} else if ( scheduleQScheme == "LADDERQ" ) {
		if( !ladderQ->empty() ) {
			utils::debug<<"( "<< threadId << " T ) Peeking from Schedule Queue"<<endl;
			ret = ladderQ->dequeue();
			if(ret == NULL) {
				cout << "empty() func returned NULL" << endl;
			}
			unsigned int newMinTime = ret->getReceiveTime().getApproximateIntTime();
			if ( newMinTime < minReceiveTime )
				cout << "Event received out of order" << endl;
			unsigned int objId = LTSFObjId[(ret->getReceiver().getSimulationObjectID())];

			utils::debug <<" ( "<< threadId << ") Locking the Object " <<objId <<endl;

			this ->getObjectLock(threadId, objId);

			//set the indexer/pointer to NULL
			lowestLadderObjectPosition[objId] = ladderQ->end();


		}
	} else {
		cout << "Invalid schedule queue scheme" << endl;
	}

	this->releaseScheduleQueueLock(threadId);
	return ret;
}

void ThreadedTimeWarpMultiSetLTSF::getObjectLock(int threadId, int objId) {
	while (!objectStatusLock[objId]->setLock(threadId, syncMechanism));
	/*	utils::debug << "( " << mySimulationManager->getSimulationManagerID()
	 << " ) Object - " << objId << " is Locked by the thread - "
	 << threadId << "\n";*/
	ASSERT(objectStatusLock[objId]->hasLock(threadId, syncMechanism));
}
void ThreadedTimeWarpMultiSetLTSF::releaseObjectLock(int threadId, int objId) {
	ASSERT(objectStatusLock[objId]->hasLock(threadId, syncMechanism));
	objectStatusLock[objId]->releaseLock(threadId, syncMechanism);
	/*	utils::debug << "( " << mySimulationManager->getSimulationManagerID()
	 << " ) Object - " << objId << " is Released by the thread - "
	 << threadId << "\n";*/
}

bool ThreadedTimeWarpMultiSetLTSF::isObjectScheduled(int objId) {
	return objectStatusLock[objId]->isLocked();
}

bool ThreadedTimeWarpMultiSetLTSF::isObjectScheduledBy(int threadId, int objId) {
	return (objectStatusLock[objId]->whoHasLock() == threadId) ? true : false;
}
void ThreadedTimeWarpMultiSetLTSF::releaseObjectLocksRecovery(int objNum) {
	if (objectStatusLock[objNum]->isLocked()) {
		objectStatusLock[objNum]->releaseLock(
				objectStatusLock[objNum]->whoHasLock(),
				syncMechanism);
		utils::debug << "Releasing Object " << objNum
				<< " during recovery." << endl;
	}
}
