#ifdef __QSEMANTICDB_SCHEDULE_H__
#ifndef __QSEMANTICDB_SCHEDULE_INL__
#define __QSEMANTICDB_SCHEDULE_INL__
//////////////////////////////////////////////////////////////////////////////
//
//    SCHEDULE.INL
//
//    Copyright © 2010, Rehno Lindeque. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

namespace QSemanticDB
{
  ScheduleQueue::ScheduleQueue(SymbolQueue* queue, uint queryDepth, const TreeIterator& iSibling) : queue(queue), queryDepth(queryDepth), nInnerBranches(0), nOuterBranches(0), frontIndex(0), iSibling(iSibling)
  {}

  void ScheduleQueue::PushBack(SemanticId symbol)
  {
    queue->push_back(symbol);
  }

  void ScheduleQueue::PopFront()
  {
    OSI_ASSERT(!Empty());
    ++frontIndex;
    if(Empty())
      Clear();
  }

  void ScheduleQueue::Clear()
  {
    queue->clear();
    frontIndex = 0;
    OSI_ASSERT(Empty());
  }

  void ScheduleQueue::Commit()
  {
    OSI_ASSERT(queryDepth > 0);
    // TODO: Not sure about this, does query depth need to be incremented again for the next branch..???
    --queryDepth; 
  }

  SemanticId ScheduleQueue::Front() const
  {
    // Pre-condition: queue must not be empty
    OSI_ASSERT(!Empty());
    return (*queue)[frontIndex];
  }

  SemanticId ScheduleQueue::Back() const
  {
    // Pre-condition: queue must not be empty
    OSI_ASSERT(!Empty());
    return queue->back();
  }

  bool ScheduleQueue::Empty() const
  {
    return frontIndex == queue->size();
  }

  size_t ScheduleQueue::Size() const
  {
    return queue->size() - frontIndex;
  }

  /*int ScheduleQueue::Branches() const
  {
    return numberOfChildren;
  }*/

  int ScheduleQueue::InnerBranches() const
  {
    return nInnerBranches;
  }

  int ScheduleQueue::OuterBranches() const
  {
    return nOuterBranches;
  }
  
  int ScheduleQueue::TotalBranches() const
  {
    return InnerBranches() + OuterBranches();
  }

  int ScheduleQueue::QueryDepth() const
  {
    return queryDepth;
  }

  void ScheduleQueue::QueryDepth(int queryDepth)
  {
    ScheduleQueue::queryDepth = queryDepth;
  }

  const ScheduleQueue::TreeIterator& ScheduleQueue::Sibling()
  {
    return iSibling;
  }

  ScheduleQueue::TreeConstIterator ScheduleQueue::Sibling() const
  {
    return iSibling;
  }

  size_t ScheduleQueue::FrontIndex() const
  {
    return frontIndex;
  }

  size_t ScheduleQueue::EndIndex() const
  {
    return queue->size();
  }

  SemanticId ScheduleQueue::operator [] (size_t index) const
  {
    return (*queue)[index];
  }

  /*ScheduleQueue::SymbolIterator ScheduleQueue::Begin()
  {
    return queue.begin() + frontIndex;
  }

  ScheduleQueue::SymbolConstIterator ScheduleQueue::Begin() const
  {
    return queue.begin() + frontIndex;
  }

  ScheduleQueue::SymbolIterator ScheduleQueue::End()
  {
    return queue.end();
  }

  ScheduleQueue::SymbolConstIterator ScheduleQueue::End() const
  {
    return queue.end();
  }*/

  Schedule::Schedule() : /*root(AllocQueue())*/ nRootBranches(1)
  {
    //OLD: tree.push_back(ScheduleQueue(root, 0));
    tree.push_back(ScheduleQueue(AllocQueue(), 0, tree.end()));
  }

  Schedule::~Schedule()
  {
     Clear();
  }

  void Schedule::Clear()
  {
    while(!pool.empty())
    {
      delete pool.top();
      pool.pop();
    }
  }

  /*Schedule::TreeIterator Schedule::InsertBranch(Schedule::TreeIterator iBranch)
  {
    ++i->numberOfChildren;
    auto iNext = i; ++iNext;
    if(i->numberOfChildren > 1)
    {
      // Invariant Condition: If this node is a parent to some other node, then it must precede it in the list
      OSI_ASSERT(iNext != tree.end());
      return tree.insert(iNext, ScheduleQueue(AllocQueue(), i->queryDepth, iNext));
    }
    else
    {
      return tree.insert(iNext, ScheduleQueue(AllocQueue(), i->queryDepth, tree.end()));
    }
  }*/

  Schedule::TreeIterator Schedule::InsertInnerBranch(TreeIterator iBranch)
  {
    // Inner branches are always placed before the outer branches in the list
    ++iBranch->nInnerBranches;
    auto iNext = iBranch; ++iNext;
    if(iBranch->InnerBranches() > 1)
    {
      // Invariant Condition: If this node is a parent to some other node, then it must precede it in the list
      OSI_ASSERT(iNext != tree.end());
      return tree.insert(iNext, ScheduleQueue(AllocQueue(), iBranch->queryDepth, iNext));
    }
    else
    {
      return tree.insert(iNext, ScheduleQueue(AllocQueue(), iBranch->queryDepth, tree.end()));
    }
  }

  Schedule::TreeIterator Schedule::InsertOuterBranch(TreeIterator iBranch)
  {
    // Outer branches are always placed after the inner branches in the list
    ++iBranch->nOuterBranches;
    auto iNext = iBranch; ++iNext;
    for(int c = 0; c < iBranch->InnerBranches(); ++c)
      ++iNext;
    if(iBranch->OuterBranches() > 1)
    {
      // Invariant Condition: If this node is a parent to some other node, then it must precede it in the list
      OSI_ASSERT(iNext != tree.end());
      return tree.insert(iNext, ScheduleQueue(AllocQueue(), iBranch->queryDepth, iNext));
    }
    else
    {
      return tree.insert(iNext, ScheduleQueue(AllocQueue(), iBranch->queryDepth, tree.end()));
    }
  }

  void Schedule::RemoveLeafBranch(const TreeIterator& iParentBranch, const TreeIterator& iLeafBranch)
  {
    // Pre-condition: This must be a leaf branch, hence must have no inner or outer branches
    OSI_ASSERT(iLeafBranch->InnerBranches() == 0 && iLeafBranch->OuterBranches() == 0);
    DeallocQueue(iLeafBranch->queue);
    tree.erase(iLeafBranch);
    
    //todo: is this correct...??
    --iParentBranch->nOuterBranches;
  }

  void Schedule::PopFront()
  {
    /* OLD: If the root node is not empty, then pop it
    if(!Root().Empty())
    {
      Root().PopFront();
      return;
    }*/

    // If the number of branches is 0, then the tree is empty, so nothing can be done.
    // TODO: perhaps make it an invariant condition that the tree can't be popped when it is empty
    //if(Root().Branches() == 0)
    if(Empty())
      return;

    // Continue with the next available branch
    //auto iTree = Begin(); ++iTree;
    auto iTree = Begin();

    // Invariant Condition: Since there is always a root and the number of branches > 0, the iterator should not be at the end of the list
    //OLD: OSI_ASSERT(iTree != End());

    /* OLD: Find a queue that can be committed
    while(iTree->QueryDepth() != 0 && iTree != End())
      ++iTree;//*/

    // If the first branch cannot be commited, then we should complete it first
    // Note: We are ignoring the possibility for concurrency here. If concurrent execution is desired we might pass in the branch we wish to use for the given thread (as a sort of environment).
    if(iTree->QueryDepth() != 0)
    {
      OSI_ASSERT(false);
      return;
    }

    /* OLD: No branch found that can be committed
    // TODO: Investigate whether this could ever happend (it probably shouldn't)
    // NOTE: (Currently all branches must be committed before a symbol can be popped.
    if(iTree == End())
      return;//*/

    // Found a branch that can be popped
    // Invariant Condition: No branch may have 0 symbols since such a branch is immediately removed from the tree unless it is the only remaining branch.
    OSI_ASSERT(!iTree->Empty() && !Empty());
    iTree->PopFront();
    if(iTree->Empty() && (RootBranches() > 1 || iTree->TotalBranches() > 0))
    {
      CollapseRootBranch(iTree);    // (If the branch is empty, it should be replaced by its child branches)*/
    }    

    /*OLD: if(RootBranches() > 0 && iTree->Empty() && iTree->OuterBranches() > 0)
      CollapseFirstRootBranch();*/


    /*
    // Condition: No queue that is not the root should ever be empty, hence the first symbol should probably not be invalid either
    OSI_ASSERT(nextSymbol != OSIX::SEMANTICID_INVALID);
    return nextSymbol;

    else
      return OSIX::SEMANTICID_INVALID;*/
  }

  SemanticId Schedule::Front() const
  {
    // If the tree is empty then the invalid symbol should be returned.
    if(Empty())
      return OSIX::SEMANTICID_INVALID;

    // Continue with the next available branch
    auto iTree = Begin();

    // Invariant Condition: Since there is always a root and the number of branches > 0, the iterator should not be at the end of the list
    OSI_ASSERT(iTree != End());

    /* OLD: Find a queue that can be committed
    while(iTree->QueryDepth() != 0 && iTree != End())
      ++iTree;//*/

    // Invariant Condition: No branch should ever be empty
    OSI_ASSERT(!iTree->Empty());
    return iTree->Front();
  }

  int Schedule::RootBranches() const
  {
    return nRootBranches;
  }

  bool Schedule::Empty() const
  {
    // Pre-condition: There should always be at least one branch, even if it is empty
    OSI_ASSERT(RootBranches() != 0);
    return RootBranches() == 1 && Begin()->Size() == 0;
  }

  Schedule::SymbolQueue* Schedule::AllocQueue()
  {
    // First try to find a deque in the pool of previously allocated deques
    if(!pool.empty())
    {
      SymbolQueue *result = pool.top();
      pool.pop();
      return result;
    }

    // Allocate a new queue if the pool is empty
    return new SymbolQueue;
  }

  //void Schedule::PopRoot() { }

  /*OLD: void Schedule::CollapseFirstRootBranch()
  {
    // Pre-condition: The tree should not be empty
    OSI_ASSERT(RootBranches() > 0);

    // Pre-condition: Inner branches are not yet supported!!
    OSI_ASSERT(Begin()->InnerBranches() == 0);


    nRootBranches -= 1;
    nRootBranches += Begin()->OuterBranches();
    DeallocQueue(Begin()->queue);
    tree.pop_front();
  }*/

  void Schedule::CollapseRootBranch(Schedule::TreeIterator iBranch)
  {
    // Pre-condition: The tree should not be empty
    // But this only tests if the root is empty: OSI_ASSERT(!Empty());
    OSI_ASSERT(RootBranches() > 1 || iBranch->TotalBranches() > 0);
    
    // Get the first child of the branch
    auto iChildBranch = iBranch;
    ++iChildBranch;
    
    if(iChildBranch != End())
    {
      // If this is not the first root branch, update its previous sibling to
      // point to this branch's first child (or if it has no children, this will
      // be its sibling due to the linked list structure)
      OSI_ASSERT((iBranch->TotalBranches() > 0 && iBranch->iSibling != iChildBranch) || (iBranch->TotalBranches() == 0 && iBranch->iSibling == iChildBranch));
      if(iBranch != Begin())
      {
        // Find the previous branch
        auto iPrevBranch = Begin(); 
        while(iPrevBranch->Sibling() != iBranch)
        {
          OSI_ASSERT(iPrevBranch != End());
          iPrevBranch = iPrevBranch->Sibling();
        }
        // Re-assign the previous branch's sibling
        iPrevBranch->iSibling = iChildBranch;
      }
      
      // Reassign the last child branch's sibling
      if(iBranch->TotalBranches() > 0 && iBranch->Sibling() != End())
      {
        while(iChildBranch->Sibling() != End())
          iChildBranch = iChildBranch->Sibling();
        iChildBranch->iSibling = iBranch->Sibling();
        OSI_ASSERT(iChildBranch->Sibling() != End());
      }
    }
        
    // Remove the root branch from the tree
    nRootBranches -= 1;
    nRootBranches += iBranch->TotalBranches();
    DeallocQueue(iBranch->queue);
    tree.erase(iBranch);
    
    /* Post-condition: Number of root branches should coincide with the linked-list data
    int nRoots = 0;
    for(auto i = Begin(); i != End(); i = i->Sibling())
      ++nRoots;
    OSI_ASSERT(nRoots == nRootBranches);//*/
  }

  void Schedule::DeallocQueue(SymbolQueue *queue)
  {
    pool.push(queue);
  }
}

#endif
#endif
