// bslstl_deque_test2.h                                               -*-C++-*-
#ifndef INCLUDED_BSLSTL_DEQUE_TEST2
#define INCLUDED_BSLSTL_DEQUE_TEST2

#include <bsls_ident.h>
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide support for the 'bslstl_deque.t.cpp' test driver.
//
//@CLASSES:
//
//@SEE_ALSO: bslstl_deque
//
//@DESCRIPTION: This component provides a third translation unit for 'deque'
// test cases to reduce the test-driver's build time.  Test cases 1-11, plus
// the usage example test, are in the original component test driver, test
// cases 12-23 are in this component's test driver, while the remaining tests
// (24 and higher, and the interactive tests) are in the 'bslstl_deque_test2'
// component's test driver.
//
// As the number of test concerns grows, and the number of types to instantiate
// 'deque' with in order to gain complete code coverage grows, we have exceeded
// the limits of certain compilers.  For example, gcc will not produce a valid
// '.o' file as the number of template instantiations exceeds some internal
// tables.  In order to continue providing full coverage, the standard
// value-semantic type test cases 1-11 are handled by the primary component
// test driver, and the tests for all remaining functionality are moved into
// two component, that comprise only a test driver.

#include <bslstl_deque.h>

#endif
// ----------------------------------------------------------------------------
// Copyright 2020 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
