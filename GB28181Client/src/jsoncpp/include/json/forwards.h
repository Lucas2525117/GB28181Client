#ifndef JSON_FORWARDS_H_INCLUDED
# define JSON_FORWARDS_H_INCLUDED

# include "config.h"

#if defined(_MSC_VER)
#if !defined(INT64)
typedef signed __int64			INT64;
#endif
#if !defined(UINT64)
typedef unsigned __int64		UINT64;
#endif
#else
#if !defined(INT64)
typedef signed long long		INT64;
#endif
#if !defined(UINT64)
typedef unsigned long long		UINT64;
#endif
#endif

namespace Json {

   // writer.h
   class FastWriter;
   class StyledWriter;

   // reader.h
   class Reader;

   // features.h
   class Features;

   // value.h
   typedef int Int;
   typedef unsigned int UInt;
   typedef INT64 Int64;
   typedef UINT64 UInt64;
   class StaticString;
   class Path;
   class PathArgument;
   class Value;
   class ValueIteratorBase;
   class ValueIterator;
   class ValueConstIterator;
#ifdef JSON_VALUE_USE_INTERNAL_MAP
   class ValueAllocator;
   class ValueMapAllocator;
   class ValueInternalLink;
   class ValueInternalArray;
   class ValueInternalMap;
#endif // #ifdef JSON_VALUE_USE_INTERNAL_MAP

} // namespace Json


#endif // JSON_FORWARDS_H_INCLUDED
