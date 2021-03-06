#ifndef DEFAULT_TIME_WARP_EVENT_SET_H
#define DEFAULT_TIME_WARP_EVENT_SET_H

// See copyright notice in file Copyright in the root directory of this archive.

#include "warped.h"
#include "TimeWarpEventSet.h"

#include <deque>
using std::deque;
#include <vector>
using std::vector;

class SimulationObject;
class TimeWarpEventSetFactory;
class SimulationConfiguration;
class TimeWarpSimulationManager;
class DefaultTimeWarpEventContainer;

/** The default implementation of TimeWarpEventSet.  This implementation
    works in the following manner.  Events are kept in two groups,
    unprocessed and processed.  Processed events are not explicitly sorted
    (although they maybe be by virtue of the order of their insertion),
    unprocessed events are sorted on demand.  That is, they are inserted in
    arbitrary order but when events are requested they are sorted if
    needed.
*/
class DefaultTimeWarpEventSet : public TimeWarpEventSet {
public:
  DefaultTimeWarpEventSet( TimeWarpSimulationManager *initSimManager,
                           bool usingOneAntiMsg );

  ~DefaultTimeWarpEventSet();

  bool insert( const Event *event );

  bool handleAntiMessage( SimulationObject *eventFor,
                          const NegativeEvent *cancelEvent );

  const Event *getEvent(SimulationObject *);

  const Event *getEvent(SimulationObject *, const VTime&);

  const Event *peekEvent(SimulationObject *);

  const Event *peekEvent(SimulationObject *, const VTime&);

  void fossilCollect( SimulationObject *, const VTime & );

  void fossilCollect( SimulationObject *, int );

  void fossilCollect( const Event * );

  void configure( SimulationConfiguration & ){}

  void rollback( SimulationObject *, const VTime & );

  bool inThePast( const Event * );

  void ofcPurge();

  void debugDump( const string &objectName, ostream &os );

private:
  DefaultTimeWarpEventContainer &getEventContainer( const OBJECT_ID *objectID );
  
  vector<DefaultTimeWarpEventContainer *> events;

  TimeWarpSimulationManager *mySimulationManager;
};

#endif
