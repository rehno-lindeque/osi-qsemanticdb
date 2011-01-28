#ifndef __QSEMANTICDB_TYPES_H__
#define __QSEMANTICDB_TYPES_H__
//////////////////////////////////////////////////////////////////////////////
//
//    TYPES.H
//
//    Copyright © 2009, Rehno Lindeque. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////
/*                               DOCUMENTATION                              */
/*
    DESCRIPTION:
      Helper types for internal use.
*/
namespace QSemanticDB
{
  // Container types
  typedef std::vector<SemanticId> IdVector;
  typedef std::set<SemanticId> IdSet;
  typedef std::stack<SemanticId> IdStack;

  // Qualified symbol decomposed structure
  struct QualifiedSymbol
  {
    SemanticId domain;
    SemanticId codomain;
    SemanticId qualifiedCodomain;
    inline operator SemanticId () const { return qualifiedCodomain; }
    inline QualifiedSymbol& operator = (SemanticId id) { qualifiedCodomain = id; return *this; }
  };

  // Query symbol decomposed structure
  struct QuerySymbol
  {
    struct SpeculativeDomain
    {
      SemanticId parentDomain;
      SemanticId queryDomain;
      SemanticId qualifiedCodomain;
      inline operator SemanticId () const { return qualifiedCodomain; }
      inline SpeculativeDomain& operator = (SemanticId id) { qualifiedCodomain = id; return *this; }
    } speculativeDomain;

    SemanticId argument;
    SemanticId qualifiedCodomain;
    inline operator SemanticId () const { return qualifiedCodomain; }
    inline QuerySymbol& operator = (SemanticId id) { qualifiedCodomain = id; return *this; }
  };
}

#endif
