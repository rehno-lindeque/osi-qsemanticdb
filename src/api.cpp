//////////////////////////////////////////////////////////////////////////////
//
//    API.CPP
//
//    Copyright © 2009, Rehno Lindeque. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////
/*                               DOCUMENTATION                              */
/*
    IMPLEMENTATION:

    TODO:
*/

/*                              COMPILER MACROS                             */
#ifdef _MSC_VER
# pragma warning(push)
#endif
/*                                 INCLUDES                                 */
#include "api.h"

#ifndef OSI_C_STATIC_BUILD
# include "api.inl"
# ifdef _DEBUG
#   include "apidbg.inl"
# endif
#endif

/*                                  MACROS                                  */
#ifdef _MSC_VER
# pragma warning(pop)
#endif

/*                                  GLOBALS                                 */
const char* const QSemanticDB::SemanticDBImplementation::epsilonName = "Epsilon";
const OSIX::SemanticId QSemanticDB::SemanticDBImplementation::firstReservedId = QSemanticDB::SemanticDBImplementation::INTERNALID_HIDDEN;
const OSIX::SemanticId QSemanticDB::SemanticDBImplementation::INTERNALID_HIDDEN = 0xfffffff2;
