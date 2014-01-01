#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

// This logic is required to block the g++ warning triggered by including
// a "deprecated" header.  The warning is generated by g++'s internal
// backward/backward_warning.h header, so we block its effect by defining its
// include guard.

#if defined(BSLS_PLATFORM_OS_SOLARIS) || defined(BSLS_PLATFORM_OS_LINUX)
#if defined(BSLS_PLATFORM_CMP_GNU)
#define _BACKWARD_BACKWARD_WARNING_H 1
#endif
#endif

#include <bsl_strstream.h>
#include <strstream>
#ifdef std
#   error std was not expected to be a macro
#endif
namespace std { }
int main() { return 0; }

// ----------------------------------------------------------------------------
// Copyright (C) 2013 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
