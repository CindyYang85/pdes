// See copyright notice in file Copyright in the root directory of this archive.

#include "OutputEvents.h"
#include "OutputManagerImplementationBase.h"
#include "TimeWarpSimulationManager.h"
#include "Event.h"

using std::cout;
using std::cerr;
using std::endl;

OutputManagerImplementationBase::OutputManagerImplementationBase( TimeWarpSimulationManager *simMgr) :
  mySimulationManager(simMgr),
  myOutputEvents(simMgr->getNumberOfSimulationObjects(), NULL){
  for(int i = 0; i < myOutputEvents.size(); i++){
    myOutputEvents[i] = new OutputEvents(mySimulationManager);
  }
}

OutputManagerImplementationBase::~OutputManagerImplementationBase(){
  for(int i = 0; i < myOutputEvents.size(); i++){
    delete myOutputEvents[i];
  }
}

OutputEvents &
OutputManagerImplementationBase::getOutputEventsFor( const ObjectID &objectId ){
   return *(myOutputEvents[objectId.getSimulationObjectID()]);
}

void
OutputManagerImplementationBase::insert( const Event *event ){
  getOutputEventsFor( event->getSender() ).insert( event );
}

void
OutputManagerImplementationBase::fossilCollect(SimulationObject *object, const VTime &fossilCollectTime){
  ASSERT(object != NULL);
  utils::debug << "OutputManager fossil collecting " << object->getName() << " for events "
		 << "prior to time " << fossilCollectTime << endl;
  getOutputEventsFor( *(object->getObjectID()) ).fossilCollect( fossilCollectTime );
}

void
OutputManagerImplementationBase::fossilCollect(SimulationObject *object, int fossilCollectTime){
  ASSERT(object != NULL);
  utils::debug << "OutputManager fossil collecting " << object->getName() << " for events "
                 << "prior to time " << fossilCollectTime << endl;
  getOutputEventsFor( *(object->getObjectID()) ).fossilCollect( fossilCollectTime );
}

void
OutputManagerImplementationBase::fossilCollectEvent(const Event *toRemove){
  getOutputEventsFor(toRemove->getSender()).fossilCollectEvent(toRemove);
}

const Event*
OutputManagerImplementationBase::getOldestEvent(unsigned int size){
  const Event *objOldest = NULL;
  const Event *retval = NULL;
  retval = myOutputEvents[0]->getOldestEvent(size);
  for(int i = 1; i < myOutputEvents.size(); i++){
    objOldest = myOutputEvents[i]->getOldestEvent(size);
    if(objOldest != NULL){
      if(retval != NULL){
        if(objOldest->getSendTime() < retval->getSendTime()){
          retval = objOldest;
        }
      }
      else{
        retval = objOldest;
      }
    }
  }
  return retval;
}

void
OutputManagerImplementationBase::saveOutputCheckpoint(ofstream* outFile,
                                                      const ObjectID &objId,
                                                      unsigned int saveTime){
  unsigned int i = objId.getSimulationObjectID();
  myOutputEvents[i]->saveOutputCheckpoint(outFile, saveTime);
}

void
OutputManagerImplementationBase::ofcPurge(){
  for(int i = 0; i < myOutputEvents.size(); i++){
    myOutputEvents[i]->ofcPurge();
  }
}
