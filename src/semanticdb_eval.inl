#ifdef  __QSEMANTICDB_SEMANTICDB_H__
#ifndef __QSEMANTICDB_SEMANTICDB_EVAL_INL__
#define __QSEMANTICDB_SEMANTICDB_EVAL_INL__
//////////////////////////////////////////////////////////////////////////////
//
//    SEMANTICDB_EVAL.INL
//
//    Copyright © 2009-2010, Rehno Lindeque. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

namespace QSemanticDB
{
  void SemanticDBImplementation::EvalInternal(SemanticId evalId)
  {
    // Ignore hidden domains
    if(GetUnqualifiedCodomain(evalId) == INTERNALID_HIDDEN)
    {
      QSEMANTICDB_DEBUG_VERBOSE_PRINT("IGNORE HIDDEN ID (" << evalId << ")" << std::endl)
      //SemanticId evalId = scheduler.Back();
      //EvalInternal(evalId);
      //scheduler.Pop();
      return;
    }

    // Check whether the item represents an atom, a query, a speculative domain or a hidden id
    IdPropertiesIterator i = symbolProperties.find(evalId);

    // Evaluate simple symbols
    QSEMANTICDB_DEBUG_VERBOSE_PRINT("EvalInternal(" << evalId << ")...")
    if(i == symbolProperties.end() || (i->second.query == QueryNone && i->second.concrete))
    {
      QSEMANTICDB_DEBUG_VERBOSE_PRINT("EvalInternal_Before_EvalSymbol(" << evalId << ")" << std::endl)
      EvalSymbol(evalId);
      QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalInternal_After_EvalSymbol", "(Note: This symbol cannot be a query)\\nIf the symbol is qualified, then its unqualified codomain was added\\nto the (back of the) schedule.")

      // Add parent codomain edges of this id to the evaluation stack
      QSEMANTICDB_DEBUG_VERBOSE_PRINT("EvalInternal_Before_EvalScheduleDefinitions(" << evalId << ")" << std::endl)
      if(EvalScheduleDefinitions(evalId, false))
      {
        if(scheduler.OuterBranches() > 0)
          scheduler.GotoFirstBranch();
        QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalInternal_After_EvalScheduleDefinitions", "If the domain corresponds (recursively) with any qualified codomains (i.e. global definitions x = ...),\\n then they were added to the (back of the) schedule.\\The scheduler also proceeds to the first branch that was added.")
        do
        {
          QSEMANTICDB_DEBUG_VERBOSE_PRINT("EvalIfQuery(" << scheduler.Get() << ')' << std::endl)
          if (!EvalIfQuery(scheduler.Get()))
          {
            // break;
            //todo: what should be done now?
            // IS THIS CORRECT???
            //scheduler.GotoNextBranch();
            break;
          }
        } while(!scheduler.Done()); // TODO: scheduler.Done() probably won't work properly with nested queries????
      }
      return;
    }
    // Evaluate a query (speculative symbols should never be evaluated here...)
    OSI_ASSERT(i->second.query != QueryNone && i->second.concrete);
    //QSEMANTICDB_DEBUG_VERBOSE_PRINT("EvalIfQuery" << std::endl)
    //EvalIfQuery(evalId);

    // Evaluate a query
    /*else if(i->second.query == QuerySelectionConjunct || i->second.query == QuerySelectionStrictConjunct)
    {
#ifdef QSEMANTICDB_DEBUG_VERBOSE
      infoStream << "SELECT CONJUNCT";
      if(i->second.query == QuerySelectionStrictConjunct)
        infoStream << " (STRICT)";
#endif

      // Get the domain and the unqualified codomain of the query (i.e. as an unqualified relation)
      evalId = EvalSelectionConjunct(evalId);
      if (evalId == OSIX::SEMANTICID_INVALID && i->second.query == QuerySelectionStrictConjunct)
      {
        //errorStream << "TODO: Assertion failed...";
        infoStream << " TODO: Assertion failed...";
      }
    }
    else if (i->second.query == QueryMutationConjunct || i->second.query == QueryMutationStrictConjunct)
    {
#ifdef QSEMANTICDB_DEBUG_VERBOSE
      infoStream << "MUTATION CONJUNCT";
      if(i->second.query == QueryMutationStrictConjunct)
        infoStream << " (STRICT)";
#endif
      evalId = EvalMutationConjunct(evalId);
      if (evalId == OSIX::SEMANTICID_INVALID && i->second.query == QueryMutationStrictConjunct)
      {
        //errorStream << "TODO: Assertion failed...";
        infoStream << " TODO: Assertion failed...";
      }
    }
    else
    {
      // TODO: Unhandled type of query....
      evalId = OSIX::SEMANTICID_INVALID;

#ifdef QSEMANTICDB_DEBUG_VERBOSE
      infoStream << "Unhandled type of query...";
#endif
    }

    // Set up for the next evaluation
    if(evalId == OSIX::SEMANTICID_INVALID)
      EvalContinue();

    return evalId;*/
  }

  /*void SemanticDBImplementation::EvalContinue()
  {
    /* OLD:
    if (evaluationQueries.empty())
    {
      evalId = OSIX::SEMANTICID_INVALID;
      return;
    }

    /* If there are any queries left on the evaluation stack, then evaluate them
    evalId = evaluationQueries.top();
    evaluationQueries.pop();//* /

    //* NEW:
    evalId = scheduler.Pop(); //* /
  }*/

  bool SemanticDBImplementation::EvalIfQuery(SemanticId symbol)
  {
    IdPropertiesIterator iProperties = symbolProperties.find(symbol);
    if(iProperties == symbolProperties.end())
      return false; // Not a query if no properties are associated with the symbol

    SymbolProperties &properties = iProperties->second;

    // Invariant Condition: Speculative symbols are never added to the schedule
    OSI_ASSERT(properties.concrete);

    // If the symbol is not a query we simply commit it and continue with the next branch (see EvalInternal)
    if(properties.query == QueryNone)
    {
      // (todo: unless we are currently in a query?)
      //QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("Commit..." << std::endl)
      //scheduler.Commit();
      //QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalIfQuery_QueryNone", "Not a query")
      return false;
    }

    // Increment the scheduler's query depth
    scheduler.BeginQuery();
    QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalIfQuery_AfterBeginQuery", "It was found that the active symbol is a query, hence we increment the query depth by 1\\n(in order to indicate that 1 more query must be resolved before the current branch can be committed)")

    // Add the query to a stack
#ifdef _DEBUG
    evalQueryStack.push_back(symbol);
#else
    evalQueryStack.push(symbol);
#endif

    if(properties.query == QuerySelectionConjunct || properties.query == QuerySelectionStrictConjunct)
    {
#ifdef QSEMANTICDB_DEBUG_VERBOSE
      QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("SELECT CONJUNCT")
      if(properties.query == QuerySelectionStrictConjunct)
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("(STRICT)")
#endif

      // Get the domain and the unqualified codomain of the query (i.e. as an unqualified relation)
      //SemanticId result = EvalSelectionConjunct(symbol);
      QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalIfQuery_Before_SelectionConjunct", "The query is a conjunct selection, so we will try to find a definition for a query\\nin the form (x.y) such that x = ... = y")
      bool result = EvalSelectionConjunct(symbol);
      QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE("EvalIfQuery_After_SelectionConjunct")
      //if(result != OSIX::SEMANTICID_INVALID)
#ifdef _DEBUG
      evalQueryStack.pop_back();
#else
      evalQueryStack.pop();
#endif
      /*if(result)
      {
        / * // TODO: FIX THIS. ActiveQueueSize should not be 0
        if (scheduler.activeQueue.size()==0)
        {
          QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("TODO: This is a bug, we should not be able to reach this point" << std::endl)
#ifdef _DEBUG
          evalQueryStack.pop_back();
#else
          evalQueryStack.pop();
#endif
          return 0;
        }

        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("ActiveQueueSize = " << scheduler.activeQueue.size() << std::endl)
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("ActiveQueryDepth = " << scheduler.activeQueue.back()->QueryDepth() << std::endl)
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("Commit..." << std::endl)
        scheduler.Commit();
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("ActiveQueueSize = " << scheduler.activeQueue.size() << std::endl)
        if(scheduler.activeQueue.size() > 0)
          QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("ActiveQueryDepth = " << scheduler.activeQueue.back()->QueryDepth() << std::endl)
        QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE("EvalIfQuery_Commit")* /
        return true;
      }//*/

      // Check whether an assertion error message should be triggered
      //if(result == OSIX::SEMANTICID_INVALID && properties.query == QuerySelectionStrictConjunct)
      if(!result && properties.query == QuerySelectionStrictConjunct)
      {
        //errorStream << "TODO: Assertion failed...";
        infoStream << " TODO: Assertion failed... (Strict conjunct selection)";
      }
    }
    else if(properties.query == QueryMutationConjunct || properties.query == QueryMutationStrictConjunct)
    {
#ifdef QSEMANTICDB_DEBUG_VERBOSE
      QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("MUTATION CONJUNCT")
      if(properties.query == QueryMutationStrictConjunct)
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT(" (STRICT)")
#endif
    }
    /*OLD: WHY?? This probably shouldn't be here since the scheduler rolls back during evaluation of the query...
    // Roll back the evaluation if the query did not succeed
    QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("Rollback..." << std::endl)    
    scheduler.Rollback();
#ifdef _DEBUG
    evalQueryStack.pop_back();
#else
    evalQueryStack.pop();
#endif//*/
    QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalIfQuery_QueryDone", "The query is finished, so it was removed from the query stack.\\nIt may have failed or it may have succeeded...")
    return true;
  }

  bool SemanticDBImplementation::EvalScheduleDefinitions(SemanticId evalId, bool onlyScheduleBranches)
  {
    const IdMultiIndexRange r = domainIndexQCodomains.equal_range(evalId);

    /* OLD: using a stack
    for(IdMultiIndexIterator i = r.first; i != r.second; ++i)
      evaluationQueries.push(i->second);//*/

    //* NEW: Using the eval schedules
    if (r.first == r.second)
      return false;

    IdMultiIndexIterator iNext = r.first; ++iNext;
    if(iNext == r.second && scheduler.InnerBranches() == 0 && !onlyScheduleBranches)
    {
      //IdPropertiesIterator iProperties = symbolProperties.find(r.first->second);
      // todo... crash. do GetProperties instead

      const auto& properties = GetProperties(r.first->second);

      // If the symbol is speculative, we don't schedule it, but instead schedule all the queries that follow on it
      if(properties.concrete)
      {
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("EvalScheduleDefinitions_Push(" << r.first->second << ')' << std::endl)
        scheduler.Push(r.first->second);
        return true;
      }
      else
      {
        return EvalScheduleDefinitions(r.first->second, /*false*/ true);
      }
    }

    bool result = false;
    for(IdMultiIndexIterator i = r.first; i != r.second; ++i)
    {
      //IdPropertiesIterator iProperties = symbolProperties.find(i->second);
      const auto& properties = GetProperties(r.first->second);

      if(properties.concrete)
      {
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("EvalScheduleDefinitions_PushOuter(" << i->second << ')' << std::endl)
        scheduler.PushOuterBranch(i->second);
        result = true;
      }
      else
      {
        result |= EvalScheduleDefinitions(i->second, true);
      }
    }//*/

    //QSEMANTICDB_DEBUG_VERBOSE_PRINT(std::endl)

    return result;
  }

  //SemanticId SemanticDBImplementation::EvalSymbol(SemanticId symbol)
  void SemanticDBImplementation::EvalSymbol(SemanticId symbol)
  {
    //* Determine whether this id has child codomains and evaluate them
    SemanticId unqualifiedCodomain = GetUnqualifiedCodomain(symbol);
#ifdef QSEMANTICDB_DEBUG_VERBOSE
    if(unqualifiedCodomain == OSIX::SEMANTICID_INVALID)
    {
      IdStringMap::iterator i = epsilonStrings.find(symbol);
      if(i == epsilonStrings.end())
        infoStream << "(Unknown Atom Id)";
      else
        infoStream << "(Atom: " << i->second << ")";
    }
    else
    {
      infoStream << "(Qualified Id: " << symbol << ")";
      scheduler.Push(unqualifiedCodomain);
    }
    infoStream << std::endl;
#endif

    //return unqualifiedCodomain;
    //*/
  }

  SemanticId SemanticDBImplementation::ResolveContext(SemanticId domain)
  {
    // Resolve the speculative domain on the left-hand side
    do
    {
      // Test whether this domain is concrete
      if(GetProperties(domain).concrete)
        break;

      // Get the  domain of this symbol
      domain = GetDomain(domain);
    } while(domain != OSIX::SEMANTICID_INVALID);
    return domain;
  }

  SemanticId SemanticDBImplementation::ResolveRelation(OrderedRelation& relation)
  {
    /*
    // Try to find a concrete relation
    RelationIndex::right_const_iterator i = relations.right.find(relation);
    if(i != relations.right.end())
      return i->second;

    // Get all speculative relations that involve the domain
    IdMultiIndexRange rSpecCodomains = domainIndexSpeculativeQCodomains.equal_range(relation.domain);
    for(IdMultiIndexIterator iSpecCodomain = rSpecCodomains.first; iSpecCodomain != rSpecCodomains.second; ++iSpecCodomain)
    {
      // If the query has the correct unqualified codomain then perform the query to test whether the codomain will be returned
      // TODO: At the moment we simply assume that the query is a selection
      //       We'll need to put a test in here somewhere to determine this later
      SemanticId speculativeCodomain = iSpecCodomain->second;
      RelationIndex::right_const_iterator iQueryCodomain = relations.right.find(OrderedRelation(speculativeCodomain, relation.codomain));
      if(iQueryCodomain != relations.right.end())
      {
        // Do the selection query
        SemanticId currentEvalId = evalId;
        evalId = iQueryCodomain->second;
        SemanticId result = EvalInternal();
        if(result != OSIX::SEMANTICID_INVALID)
          return result;
      }
    }

    // Try to resolve the relation in a parent context
    if(relation.domain != OSIX::SEMANTICID_EPSILON && relation.domain != OSIX::SEMANTICID_INVALID)
    {
      relation.domain = GetDomain(relation.domain);
      return ResolveRelation(relation);
    }

    // All symbols exist in the global domain epsilon, so just return the unqualified codomain itself if no qualified codomain could be found
    return relation.codomain;*/

    return OSIX::SEMANTICID_INVALID;
  }

 SemanticId SemanticDBImplementation::EvalInternalNext(Scheduler::Visitor& visitor)
 {
    // If the fetched symbol needs to be evaluated internally for the next iteration, then evaluate it
    SemanticId evalId;
    if(visitor.EndOfQueue())
    {
      EvalInternal(visitor.Get());

      // If the visitor is still at the end of the queue then we should advance to the first outer branch
      // otherwise, simply step to the next symbol
      if(!visitor.EndOfQueue())
      {
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("EvalInternalNext_!EndOfQueue" << std::endl)
        ++visitor.symbolIndex;
        evalId = visitor.Get();
      }
      else
      {
        ++visitor.queueStackIndex;
        if(!visitor.EndOfBranch())
        {
          QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("EvalInternalNext_!EndOfBranch" << std::endl)
          visitor.symbolIndex = visitor.GetTree()->FrontIndex();
          evalId = visitor.Get();
        }
        else
        {
          evalId = OSIX::SEMANTICID_INVALID;
        }
      }
    }
    else
    {
      // Advance the visitor position in the active queue
      ++visitor.symbolIndex;
      evalId = visitor.Get();
    }

    return evalId;
  }


  bool SemanticDBImplementation::EvalInternalUntil(SemanticId startSymbol, SemanticId endSymbol)
  {
    // Note: startSymbol is the query domain that was just pushed onto the schedule, where-as endSymbol is the query argument (codomain)
    //       that we're looking for. Hence evaluate all codomains of startSymbol until codomain has been found (or all codomains are fully evaluated)

    bool result = false;



    // todo... (is this correct?)
    //scheduler.PushOuterBranch(startSymbol);
    //scheduler.GotoFirstBranch();
    //QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULEVISITOR("EvalInternalUntil")


    //std::stack<Schedule::Visitor> visitor;
    //visitor.push_back(scheduler.GetVisitor());
    Scheduler::Visitor visitor = scheduler.GetVisitor();
    const Scheduler::Visitor startingVisitor = visitor;


    // Pre-conditions: Start symbol is the last symbol in the queue
    // NOT TRUE: the speculative domain is the first symbol in the queue
    //OSI_ASSERT(startSymbol == visitor.Get());

    // Add the start symbol to the queue
    scheduler.Push(startSymbol);
    ++visitor.symbolIndex;

    QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULEVISITOR2("EvalInternalUntil_StartSymbolAdded", "The start symbol (the speculative query domain) was added to the (back of the) schedule", visitor)

    // Evaluate each symbol following this one
    SemanticId evalId = EvalInternalNext(visitor);
    while(true)
    {
      QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULEVISITOR2("EvalInternalUntil_Loop", "(Busy evaluating every symbol that follows until the query argument is found or the queue rolled back)", visitor)
      evalId = EvalInternalNext(visitor);

      // Test whether we are done
      // I.e. there are no more symbols in the queue and no more branches to follow either.
      if(evalId == OSIX::SEMANTICID_INVALID)
      {
        QSEMANTICDB_DEBUG_EVALOUTPUT_PRINT("EvalInternalUntil_Before_Rollback" << std::endl)
        QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULEVISITOR2("EvalInternalUntil_Before_Rollback", "It was found that this branch doesn't correspond with the query argument!\\nRolling back to the next branch or until it is clear that the query can't be resolved...", visitor)

        scheduler.Rollback();
        visitor = scheduler.GetVisitor();

        QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULEVISITOR2("EvalInternalUntil_After_Rollback", "Rolled back to a different branch.\\nThe active branch was deleted...", visitor)

        // Check whether all codomains have now been evaluated
        // todo: Is this condition entirely complete?
        if(visitor.queueStackIndex <= startingVisitor.queueStackIndex)
          break;
      }

      // Test whether the end symbol could be found
      // Todo: Is this condition complete? Should the end symbol perhaps always be at the end of a queue to ensure that it is never a query domain?????????
      if (evalId == endSymbol)
      {
        // This branch was successful...
        result = true;
        scheduler.Commit();

        QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULEVISITOR2("EvalInternalUntil_AfterCommit", "The query argument was found in the current branch and has been committed", visitor)

        // TODO: Is this correct?
        if (scheduler.activeQueue.empty())
          break;
        visitor = scheduler.GetVisitor();

        // Check whether all codomains have now been evaluated
        // todo: Is this condition entirely complete?
        if(visitor.queueStackIndex <= startingVisitor.queueStackIndex)
          break;
      }
    }
    
    // TODO: IS THIS CORRECT?
    // Whether the query succeeded or failed, it is now finished. Hence decrease the query depth
    --scheduler.queryDepth;
    
    return result;
  }

  bool SemanticDBImplementation::EvalSelectionConjunct(SemanticId queryId)
  {
    // Decompose the query symbol
    QuerySymbol querySymbol;
    if(!Decompose(querySymbol, queryId))
    {
      errorStream << "FATAL ERROR: Could not decompose the query symbol. This should never happen and should be reported as bug!" << std::endl;
      return false;
    }

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
    infoStream << "(TEMPORARY NOTE: Query structure "
      << static_cast<SemanticId>(querySymbol)
      << "=( "
        << static_cast<SemanticId>(querySymbol.speculativeDomain)
        << "=( "
          << querySymbol.speculativeDomain.parentDomain << ", " << querySymbol.speculativeDomain.queryDomain
        << " ), "
        << querySymbol.argument
      << " )" << std::endl;
#endif

    /* OLD Split the query into a speculative domain and an unqualified query argument
    RelationIndex::left_const_iterator iQueryRelation = relations.left.find(query);
    if(iQueryRelation == relations.left.end())
    {
      errorStream << "FATAL ERROR: Could not find the relation for a query in the database. This should never happen and should be reported as bug!" << std::endl;
      return OSIX::SEMANTICID_INVALID;
    }
    OrderedRelation queryRelation = iQueryRelation->second;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
    infoStream << std::endl << "(TEMPORARY NOTE: Query relation = " << queryRelation.domain << "->" << queryRelation.codomain << ")" << std::endl;
#endif*/

    /* TODO: put this back...
    // If the domain has a hidden domain, then perform the query on the hidden domain instead
    //if (anonDomain.find(queryRelation.domain) != anonDomain.end() && )
    SemanticId hiddenCodomain = GetQualifiedCodomain(OrderedRelation(queryRelation.domain, INTERNALID_HIDDEN));
    if (hiddenCodomain != OSIX::SEMANTICID_INVALID)
    {
      queryRelation.domain = hiddenCodomain;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
      infoStream << "(TEMPORARY NOTE: Resolving relation in the hidden domain (" << hiddenCodomain << "))" << std::endl;
      infoStream << "(TEMPORARY NOTE: QUERY = " << queryRelation.domain << "->" << queryRelation.codomain << ")" << std::endl;
#endif

      // Try to find a relation
      SemanticId result = OSIX::SEMANTICID_INVALID;
      //result = ResolveRelation(queryRelation);
      RelationIndex::right_const_iterator i = relations.right.find(queryRelation);
      if(i != relations.right.end())
        result = i->second;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
      infoStream << "(TEMPORARY NOTE: QUERY RESULT = " << result << ")" << std::endl;
#endif

      return result;
    }*/

    /* OLD: Split the speculative domain into a domain and an unqualified codomain
    // SpeculativeDomain
    RelationIndex::left_const_iterator iDomainRelation = relations.left.find(queryRelation.domain);
    if(iDomainRelation == relations.left.end())
    {
      errorStream << "ERROR (TODO): Could not find a query domain in the database. Not yet sure what to do in this case..." << std::endl;
      return OSIX::SEMANTICID_INVALID;
    }
    OrderedRelation domainRelation = iDomainRelation->second;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
    infoStream << "(TEMPORARY NOTE: Speculative domain relation = " << domainRelation.domain << "->" << domainRelation.codomain << ")" << std::endl;
#endif*/

    // Find an appropriate parent domain and query domain and then perform a conjunct selection to try and find the argument
    // If the speculative domain is actually concrete, then we should simply evaluate whether it contains the argument
    auto properties = GetProperties(querySymbol.speculativeDomain);
    if(properties.concrete)
    {
      QSEMANTICDB_DEBUG_VISUALIZE_SCHEDULE2("EvalSelectionConjunct_Before_EvalInternalUntil", "In this case the speculative domain was found to be concrete,\\nso simply search for the query argument.")
      return EvalInternalUntil(querySymbol.speculativeDomain, querySymbol.argument);
    }

    // Test whether the query domain exists in a parent domain
    SemanticId parentDomain = querySymbol.speculativeDomain.parentDomain;
    while(parentDomain != OSIX::SEMANTICID_EPSILON)
    {
      parentDomain = GetDomain(parentDomain);
      QSEMANTICDB_DEBUG_VERBOSE_PRINT("Parent domain = " << parentDomain << "." << std::endl)
      SemanticId queryDomain = GetConcreteQualifiedCodomain(parentDomain, querySymbol.speculativeDomain.queryDomain);
      QSEMANTICDB_DEBUG_VERBOSE_PRINT("Query domain = " << queryDomain << "." << std::endl)
      if(queryDomain != OSIX::SEMANTICID_INVALID)
        return EvalInternalUntil(queryDomain, querySymbol.argument);
    }

    // The parent domain is now the epsilon domain and the speculative domain could not be found anywhere further down the hierarchy.
    // Since all atoms exist in the epsilon domain, we need not look any further: The query succeeded if the top most instance of the
    // speculative domain contained the query argument. Since it didn't, return false.
    // NOTE: Notice that during the last iteration of the above loop parentDomain is epsilon.
    OSI_ASSERT(parentDomain == OSIX::SEMANTICID_EPSILON);
    return false;

    /* TODO: Put this back (old version of the above)
    // Resolve the speculative domain of the query to get a concrete domain
    domainRelation.domain = ResolveContext(domainRelation.domain);

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
    infoStream << "(TEMPORARY NOTE: Concrete domain relation = " << domainRelation.domain << "->" << domainRelation.codomain << ")" << std::endl;
#endif

    // Resolve the selection itself
    SemanticId result = OSIX::SEMANTICID_INVALID;
    while(domainRelation.domain != OSIX::SEMANTICID_EPSILON)
    {
      // Find a suitable domain for the query
      domainRelation.domain = GetDomain(domainRelation.domain);
      queryRelation.domain = ResolveRelation(domainRelation);

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
      infoStream << "(TEMPORARY NOTE: QUERY DOMAIN = " << domainRelation.domain << "->" << domainRelation.codomain << ")" << std::endl;
      infoStream << "(TEMPORARY NOTE: QUERY = " << queryRelation.domain << "->" << queryRelation.codomain << ")" << std::endl;
#endif

      /* OLD METHOD:
      result = ResolveRelation(queryRelation);* /

      //* NEW METHOD: Evaluate the codomain internally until the correct codomain is reached or else return
      do
      {
        EvalInternal(queryRelation.domain);
        SemanticId result = scheduler.Get();
        if(GetUnqualifiedCodomain(result) == queryRelation.codomain)
          return result;
      }
      while(result != OSIX::SEMANTICID_INVALID); //* /

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
      infoStream << "(TEMPORARY NOTE: QUERY = " << queryRelation.domain << "->" << queryRelation.codomain << ")" << std::endl;
      infoStream << "(TEMPORARY NOTE: QUERY RESULT = " << result << ")" << std::endl;
#endif

      // If the query's domain could not be resolved, then the query has failed.
      if(queryRelation.domain != OSIX::SEMANTICID_EPSILON)
        return result;
    }*/

    //return OSIX::SEMANTICID_INVALID;
  }

  bool SemanticDBImplementation::EvalMutationConjunct(SemanticId query)
  {
    /*// Split the query into a speculative domain and an unqualified query argument
    RelationIndex::left_const_iterator iQueryRelation = relations.left.find(query);
    if(iQueryRelation == relations.left.end())
    {
      errorStream << "FATAL ERROR: Could not find the relation for a query in the database. This should never happen and should be reported as bug!" << std::endl;
      return OSIX::SEMANTICID_INVALID;
    }
    OrderedRelation queryRelation = iQueryRelation->second;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
    infoStream << std::endl << "(TEMPORARY NOTE: Query relation = " << queryRelation.domain << "->" << queryRelation.codomain << ")" << std::endl;
#endif

    // If the domain has a hidden domain, then perform the query on the hidden domain instead
    SemanticId hiddenCodomain = GetQualifiedCodomain(OrderedRelation(queryRelation.domain, INTERNALID_HIDDEN));
    if (hiddenCodomain != OSIX::SEMANTICID_INVALID)
    {
      queryRelation.domain = hiddenCodomain;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
      infoStream << "(TEMPORARY NOTE: Resolving relation in the hidden domain (" << hiddenCodomain << "))" << std::endl;
      infoStream << "(TEMPORARY NOTE: QUERY = " << queryRelation.domain << "->" << queryRelation.codomain << ")" << std::endl;
#endif

      // Try to find the relation
      SemanticId result = OSIX::SEMANTICID_INVALID;
      RelationIndex::right_const_iterator i = relations.right.find(queryRelation);
      if(i != relations.right.end())
        result = i->second;

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
      infoStream << "(TEMPORARY NOTE: QUERY RESULT = " << result << ")" << std::endl;
#endif

      return result;
    }*/

    // First select the query argument (RHS) from the query source (LHS) to check for its existence
    if(!EvalSelectionConjunct(query))
      return false; // Could not find the symbol

#if defined(QSEMANTICDB_DEBUG_VERBOSE) && defined(QSEMANTICDB_DEBUG_EVALOUTPUT)
    infoStream << "(TEMPORARY NOTE: BUSY HERE...........................)" << std::endl;
#endif

    return false;
  }


}

#endif
#endif
