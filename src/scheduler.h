#ifndef __QSEMANTICDB_SCHEDULER_H__
#define __QSEMANTICDB_SCHEDULER_H__
//////////////////////////////////////////////////////////////////////////////
//
//    SCHEDULER.H
//
//    Copyright © 2010, Rehno Lindeque. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////
/*                               DOCUMENTATION                              */
/*
    DESCRIPTION:
      A scheduler maintaining the current state of the evaluator with the schedule.
*/
namespace QSemanticDB
{
  class Scheduler
  {
  public:
    //// Construction / Destruction
    Scheduler(Schedule& schedule);
    Scheduler() = delete;
    Scheduler(const Scheduler&) = delete;
    Scheduler & operator=(const Scheduler&) = delete;

    //// Query Operations
    void Push(SemanticId symbol);
    void PushInnerBranch(SemanticId symbol);
    void PushOuterBranch(SemanticId symbol);

    // Increment the query depth;
    void BeginQuery()
    {
      ++queryDepth;
      activeQueue.back()->QueryDepth(queryDepth);
    }

    // Commit a branch for external evaluation
    void Commit();

    // Roll back an internally evaluated query (recursively for parent queues)
    void Rollback();

    // Go to the first branch of the current queue
    // (Currently only outer branches supported)
    void GotoFirstBranch()
    {
      Schedule::TreeIterator iChild = activeQueue.back(); ++iChild;
      activeQueue.push_back(iChild);
    }

    //todo: SemanticId Back();

    // Eval Operations
    //SemanticId Front(); // This operation should not be supported (see Pop)

    // Get the current symbol
    SemanticId Get();

    //void Pop(); // This operation should not be supported in the scheduler, the scheduler is a "writer" object.

    void Reset() { activeQueue.clear(); activeQueue.push_back(schedule.Begin()); }
    bool Done() const { return activeQueue.empty(); }
    int InnerBranches() const { return activeQueue.back()->InnerBranches(); }
    int OuterBranches() const { return activeQueue.back()->OuterBranches(); }
    int QueryDepth() const { return queryDepth; }

    //// Accessors
    //Schedule::TreeIterator FirstBranch() { return schedule.Begin(); }
    //Schedule::TreeConstIterator FirstBranch() const { return schedule.Begin(); }

  protected:
    Schedule& schedule;                               // The schedule tree structure
    SemanticId currentSymbol;
    std::vector<Schedule::TreeIterator> activeQueue;  // The stack of active queues (query strings)
    int queryDepth;                                   // The number of nested, but unresolved queries in the activeQueue.
    //SemanticId frontSymbol;                           // The current external eval symbol. This value is always equal to schedule.Front() but is cached here for efficiency.
    //SemanticId currentSymbol;                            // The current internal eval symbol (used with queries that require internal evaluation)
  };
}

/*                                   INCLUDES                               */
#include "scheduler.inl"

#endif
