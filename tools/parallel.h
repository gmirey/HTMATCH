/* -----------------------------------
 * HTMATCH
 * parallel.h
 * -----------------------------------
 * Defines tools to deal with parallel loops.
 *
 * Copyright 2019, Guillaume Mirey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _HTMATCH_PARALLEL_H
#define _HTMATCH_PARALLEL_H

#include "tools/system.h"
#include <algorithm>
#include <execution>

// using the HTMATCH_SEQ macro, you explicitely ask for sequential resolution of those std algorithms
//   taking a possibly parallel execution policy
#define HTMATCH_SEQ         std::execution::seq

// using the HTMATCH_PAR macro, you specify that you want a release-mode compilation to solve those std algorithms
//   in parallel indeed... all the while allowing debug-mode runtimes to use exact same syntax but, to the contrary,
//   execute sequentially (to ease with debugging these, quite simply...)
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#  define HTMATCH_PAR       std::execution::seq
#else
#  define HTMATCH_PAR       std::execution::par
#endif

namespace HTMATCH {

    // - - - - - - - - - - - - - - - - -
    // NumIter
    // a handy helper class for viewing a plain number as a std-compatible iterator
    // - - - - - - - - - - - - - - - - -
    struct NumIter {
	    typedef std::random_access_iterator_tag     iterator_category;
	    typedef i32fast                             value_type;
	    typedef i32fast                             difference_type;
	    typedef const i32fast*                      pointer;
        typedef const i32fast&                      reference;
        NumIter():_value(0) {}
        NumIter(i32fast iValue):_value(iValue) {}
        NumIter(u32fast uValue):_value(i32fast(uValue)) {}
        i32fast operator*() const { return _value; }
        const i32fast* operator->() const { return &_value; }
        NumIter& operator++() { ++_value; return *this; }
        NumIter operator++(int) { NumIter stateBefore=*this; ++(*this); return stateBefore; }
        NumIter& operator--() { --_value; return *this; }
        NumIter operator--(int) { NumIter stateBefore=*this; --(*this); return stateBefore; }
        NumIter& operator+=(i32fast iIncrement) { _value += iIncrement; return *this; }
        NumIter& operator-=(i32fast iDecrement) { return operator+=(-iDecrement); }
        NumIter operator+(i32fast iIncrement) const { return NumIter(_value+iIncrement); }
        NumIter operator-(i32fast iDecrement) const { return NumIter(_value-iDecrement); }
        i32fast operator-(const NumIter& other) const { return _value - other._value; }
        i32fast operator[](i32fast iOffset) const { return _value + iOffset; }
        bool operator==(const NumIter& other) const { return _value == other._value; }
        bool operator!=(const NumIter& other) const { return !operator==(other); }
        bool operator<(const NumIter& other) const { return _value < other._value; }
        bool operator>(const NumIter& other) const { return _value > other._value; }
        bool operator<=(const NumIter& other) const { return !operator>(other); }
        bool operator>=(const NumIter& other) const { return !operator<(other); }
    private:
        i32fast _value;
    };

    // and now we can wrap 'std::for_each' and its possibly parallel execution policies using NumIter...
    template<class _ExecPolicy, class _Func>
    inline void for_range(_ExecPolicy&& policy, u32fast uStart, u32fast uAfterLast, _Func func) {
        std::for_each<_ExecPolicy, NumIter, _Func>(policy, NumIter(uStart), NumIter(uAfterLast), func);
    }
    ; // template termination

    // ...also some version preferring a startIndex + count... just syntactic sugar, really
    template<class _ExecPolicy, class _Func>
    inline void for_count(_ExecPolicy&& policy, u32fast uStart, u32fast uCount, _Func func) {
        std::for_each<_ExecPolicy, NumIter, _Func>(policy, NumIter(uStart), NumIter(uStart+uCount), func);
    }
    ; // template termination

} // namespace HTMATCH


namespace std {


}

#endif // _HTMATCH_PARALLEL_H

