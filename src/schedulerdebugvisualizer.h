#ifndef __QSEMANTICDB_SCHEDULERDEBUGVISUALIZER_H__
#define __QSEMANTICDB_SCHEDULERDEBUGVISUALIZER_H__
//////////////////////////////////////////////////////////////////////////////
//
//    SCHEDULERDEBUGVISUALIZER.H
//
//    Copyright © 2010, Rehno Lindeque. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////
/*                               DOCUMENTATION                              */
/*
    DESCRIPTION:
      A debugging visualizer for the scheduler
*/
namespace QSemanticDB
{
  class SemanticDBImplementation;

  class SchedulerDebugVisualizer
  {
  public:
    SchedulerDebugVisualizer(SemanticDBImplementation& db, Scheduler& scheduler) : db(db), scheduler(scheduler), printRed(false), count(0), subgraphCounter(0), visitor(0) {}

    void Print(const char* title, const char* description = 0);
    void Print(const char* title, const char* description, Scheduler::Visitor &visitor);

  protected:
    SemanticDBImplementation& db;
    Scheduler& scheduler;
    bool printRed;
    int count;
    int subgraphCounter;
    Scheduler::Visitor *visitor;

    void PrintAtom(std::ofstream &fileStream, SemanticId atom);
    void PrintSymbol(std::ofstream &fileStream, SemanticId symbol);
    void InternPrintSymbol(std::ofstream &fileStream, SemanticId symbol);
    void PrintQueue(std::ofstream &fileStream, const Schedule::TreeIterator &iTree);
    void PrintVisitedNode(std::ofstream &fileStream, const Schedule::TreeIterator &iTree/*, uint cSymbol*/);
    void PrintBranches(Scheduler::QueueStackIterator iEvalStack, std::ofstream &fileStream, const Schedule::TreeIterator &iTree);
    void PrintQueryStack(std::ofstream &fileStream);
    void PrintActiveQueue(std::ofstream &fileStream);
    void PrintEvalPosition(std::ofstream &fileStream);
  };
}

/*                                   INCLUDES                               */
// Inline implementation
//#include "schedulerdebugvisualizer.inl"

#endif
