// bdlde_utf8util.cpp                                                 -*-C++-*-

// ----------------------------------------------------------------------------
//                                   NOTICE
//
// This component is not up to date with current BDE coding standards, and
// should not be used as an example for new development.
// ----------------------------------------------------------------------------

#include <bdlde_utf8util.h>

#include <bsls_ident.h>
BSLS_IDENT_RCSID(bdlde_utf8util_cpp,"$Id$ $CSID$")

#include <bsls_assert.h>
#include <bsls_performancehint.h>

// LOCAL MACROS

#define UNLIKELY(EXPRESSION) BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(EXPRESSION)

// LOCAL CONSTANTS

namespace {

using namespace BloombergLP;

enum {
    k_MIN_2_BYTE_VALUE = 0x80,     // min value that requires 2 bytes to encode
    k_MIN_3_BYTE_VALUE = 0x800,    // min value that requires 3 bytes to encode
    k_MIN_4_BYTE_VALUE = 0x10000,  // min value that requires 4 bytes to encode

    k_MIN_SURROGATE    = 0xd800,   // min surrogate value

    k_MAX_VALID        = 0x10ffff,  // max value that can be encoded in UTF-8

    k_CONT_VALUE_MASK  = 0x3f,      // part of a continuation byte that
                                    // contains the 6 bits of value info

    k_ONEBYTEHEAD_TEST   = 0x80,  // 1 byte Utf8
    k_ONEBYTEHEAD_RES    = 0,
    k_TWOBYTEHEAD_TEST   = 0xE0,  // 2 byte Utf8
    k_TWOBYTEHEAD_RES    = 0XC0,
    k_THREEBYTEHEAD_TEST = 0xF0,  // 3 byte Utf8
    k_THREEBYTEHEAD_RES  = 0XE0,
    k_FOURBYTEHEAD_TEST  = 0xF8,  // 4 byte Utf8
    k_FOURBYTEHEAD_RES   = 0XF0,
    k_MULTIPLEBYTE_TEST  = 0xC0,  // 2nd, 3rd, 4th byte
    k_MULTIPLEBYTE_RES   = 0X80
};

#if defined(BSLS_ASSERT_SAFE_IS_ACTIVE)

bool isValidUtf8(const char *character)
    // Return 'true' if 'character' points to a valid UTF-8 codepoint.
{
    return (character[0] & k_ONEBYTEHEAD_TEST)   == k_ONEBYTEHEAD_RES ||
          ((character[1] & k_MULTIPLEBYTE_TEST)  == k_MULTIPLEBYTE_RES &&
          ((character[0] & k_TWOBYTEHEAD_TEST)   == k_TWOBYTEHEAD_RES ||
          ((character[2] & k_MULTIPLEBYTE_TEST)  == k_MULTIPLEBYTE_RES &&
          ((character[0] & k_THREEBYTEHEAD_TEST) == k_THREEBYTEHEAD_RES ||
          ((character[3] & k_MULTIPLEBYTE_TEST)  == k_MULTIPLEBYTE_RES &&
           (character[0] & k_FOURBYTEHEAD_TEST)  == k_FOURBYTEHEAD_RES )))));
}

#endif // defined(BSLS_ASSERT_SAFE_IS_ACTIVE)

int utf8Size(char character)
    // Return the length of the UTF-8 codepoint for which the specified
    // 'character' is the first 'char'.  The behavior is undefined unless
    // 'character' is the first 'char' of a UTF-8 codepoint.
{
    if ((character & k_ONEBYTEHEAD_TEST) == k_ONEBYTEHEAD_RES) {
        return 1;                                                     // RETURN
    }
    else if ((character & k_TWOBYTEHEAD_TEST) == k_TWOBYTEHEAD_RES) {
        return 2;                                                     // RETURN
    }
    else if ((character & k_THREEBYTEHEAD_TEST) == k_THREEBYTEHEAD_RES) {
        return 3;                                                     // RETURN
    }

    return 4;
}

}  // close unnamed namespace

// STATIC HELPER FUNCTIONS

static inline
bool isNotContinuation(char value)
    // Return 'true' if the specified 'value' is NOT a UTF-8 continuation byte,
    // and 'false' otherwise.
{
    return 0x80 != (value & 0xc0);
}

static inline
bool isSurrogateValue(int value)
    // Return 'true' if the specified 'value' is a surrogate value, and 'false'
    // otherwise.
{
    enum { k_SURROGATE_MASK = 0xfffff800U };

    return (k_SURROGATE_MASK & value) == k_MIN_SURROGATE;
}

static inline
int get2ByteValue(const char *pc)
    // Return the integral value of the single code point represented by the
    // 2-byte UTF-8 sequence referred to by the specified 'pc'.  The behavior
    // is undefined unless the 2 bytes starting at 'pc' contain a UTF-8
    // sequence describing a single valid code point.
{
    return ((*pc & 0x1f) << 6) | (pc[1] & k_CONT_VALUE_MASK);
}

static inline
int get3ByteValue(const char *pc)
    // Return the integral value of the single code point represented by the
    // 3-byte UTF-8 sequence referred to by the specified 'pc'.  The behavior
    // is undefined unless the 3 bytes starting at 'pc' contain a UTF-8
    // sequence describing a single valid code point.
{
    return ((*pc & 0xf) << 12) | ((pc[1] & k_CONT_VALUE_MASK) << 6)
                               |  (pc[2] & k_CONT_VALUE_MASK);
}

static inline
int get4ByteValue(const char *pc)
    // Return the integral value of the single code point represented by the
    // 4-byte UTF-8 sequence referred to by the specified 'pc'.  The behavior
    // is undefined unless the 4 bytes starting at 'pc' contain a UTF-8
    // sequence describing a single valid code point.
{
    return ((*pc & 0x7) << 18) | ((pc[1] & k_CONT_VALUE_MASK) << 12)
                               | ((pc[2] & k_CONT_VALUE_MASK) <<  6)
                               |  (pc[3] & k_CONT_VALUE_MASK);
}

static
int validateAndCountCodePoints(const char **invalidString, const char *string)
    // Return the number of Unicode code points in the specified 'string' if it
    // contains valid UTF-8, with no effect on the specified 'invalidString'.
    // Otherwise, return a negative value and load into 'invalidString' the
    // address of the first sequence in 'string' that does not constitute the
    // start of a valid UTF-8 encoding specifying a valid Unicode code point.
    // 'string' is necessarily null-terminated, so it cannot contain embedded
    // null bytes.  Note that 'string' may contain less than
    // 'bsl::strlen(string)' Unicode code points.
{
    // The following assertions are redundant with those in the CLASS METHODS.
    // Hence, 'BSLS_ASSERT_SAFE' is used.

    BSLS_ASSERT_SAFE(invalidString);
    BSLS_ASSERT_SAFE(string);

    int count = 0;

    while (true) {
        switch ((*string >> 4) & 0xf) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            if (UNLIKELY(!*string)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                return count;                                         // RETURN
            }
            ++string;
          } break;
          case 0xc:
          case 0xd: {
            if (UNLIKELY(isNotContinuation(string[1]))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = string;
                return -1;                                            // RETURN
            }
            const int value = get2ByteValue(string);
            if (UNLIKELY(value < k_MIN_2_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = string;
                return -1;                                            // RETURN
            }
            string += 2;
          } break;
          case 0xe: {
            if (UNLIKELY(isNotContinuation(string[1]))
             || UNLIKELY(isNotContinuation(string[2]))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = string;
                return -1;                                            // RETURN
            }
            const int value = get3ByteValue(string);
            if (UNLIKELY((value < k_MIN_3_BYTE_VALUE)
                       | isSurrogateValue(value))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = string;
                return -1;                                            // RETURN
            }
            string += 3;
          } break;
          case 0xf: {
            if (UNLIKELY(isNotContinuation(string[1]))
             || UNLIKELY(isNotContinuation(string[2]))
             || UNLIKELY(isNotContinuation(string[3]))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = string;
                return -1;                                            // RETURN
            }
            const int value = get4ByteValue(string);
            if (UNLIKELY((8 & *string)
                       | (value < k_MIN_4_BYTE_VALUE)
                       | (value > k_MAX_VALID))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = string;
                return -1;                                            // RETURN
            }
            string += 4;
          } break;
          default: {
            *invalidString = string;
            return -1;                                                // RETURN
          }
        }

        ++count;
    }
}

static int validateAndCountCodePoints(const char             **invalidString,
                                      const char              *string,
                                      bsls::Types::size_type   length)
    // Return the number of Unicode code points in the specified 'string'
    // having the specified 'length' (in bytes) if 'string' contains valid
    // UTF-8, with no effect on the specified 'invalidString'.  Otherwise,
    // return a negative value and load into 'invalidString' the address of the
    // first byte in 'string' that does not constitute the start of a valid
    // UTF-8 encoding.  'string' need not be null-terminated and can contain
    // embedded null bytes.  The behavior is undefined unless
    // '0 <= IntPtr(length)'.  Note that 'string' may contain less than
    // 'length' Unicode code points.
{
    // The following assertions are redundant with those in the CLASS METHODS.
    // Hence, 'BSLS_ASSERT_SAFE' is used.

    BSLS_ASSERT_SAFE(invalidString);
    BSLS_ASSERT_SAFE(string);
    BSLS_ASSERT_SAFE(0 <= bsls::Types::IntPtr(length));

    const char       *pc     = string;
    const char *const pcEnd4 = string + length - 4;

    int count = 0;

    while (pc <= pcEnd4) {
        switch ((*pc >> 4) & 0xf) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            ++pc;
          } break;
          case 0xc:
          case 0xd: {
            const int value = get2ByteValue(pc);
            if (UNLIKELY(isNotContinuation(pc[1])
                       | (value < k_MIN_2_BYTE_VALUE))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            pc += 2;
          } break;
          case 0xe: {
            const int value = get3ByteValue(pc);
            if (UNLIKELY(isNotContinuation(pc[1])
                       | isNotContinuation(pc[2])
                       | (value < k_MIN_3_BYTE_VALUE)
                       | isSurrogateValue(value))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            pc += 3;
          } break;
          case 0xf: {
            const int value = get4ByteValue(pc);
            if (UNLIKELY((0 != (0x8 & *pc))
                       | isNotContinuation(pc[1])
                       | isNotContinuation(pc[2])
                       | isNotContinuation(pc[3])
                       | (value < k_MIN_4_BYTE_VALUE)
                       | (value > k_MAX_VALID))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            pc += 4;
          } break;
          default: {
            *invalidString = pc;
            return -1;                                                // RETURN
          }
        }

        ++count;
    }

    length -= static_cast<int>(pc - string);

    // 'length' is now < 4.

    while (length > 0) {
        int delta;

        switch ((*pc >> 4) & 0xf) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            delta = 1;
            ++count;
          } break;
          case 0xc:
          case 0xd: {
            if (UNLIKELY(length < 2)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            const int value = get2ByteValue(pc);
            if (UNLIKELY(isNotContinuation(pc[1])
                       | (value < k_MIN_2_BYTE_VALUE))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            delta = 2;
            ++count;
          } break;
          case 0xe: {
            if (UNLIKELY(length < 3)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            const int value = get3ByteValue(pc);
            if (UNLIKELY(isNotContinuation(pc[1])
                       | isNotContinuation(pc[2])
                       | (value < k_MIN_3_BYTE_VALUE)
                       | isSurrogateValue(value))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                *invalidString = pc;
                return -1;                                            // RETURN
            }
            delta = 3;
            ++count;
          } break;
          default: {
            *invalidString = pc;
            return -1;                                                // RETURN
          }
        }

        pc     += delta;
        length -= delta;
    }

    BSLS_ASSERT(0 == length);
    BSLS_ASSERT(pcEnd4 + 4 == pc);

    return count;
}


namespace BloombergLP {

namespace bdlde {
                              // ---------------
                              // struct Utf8Util
                              // ---------------

// CLASS METHODS

Utf8Util::IntPtr Utf8Util::advanceIfValid(int         *status,
                                          const char **result,
                                          const char  *string,
                                          IntPtr       numCodePoints)
{
    BSLS_ASSERT(status);
    BSLS_ASSERT(result);
    BSLS_ASSERT(string);
    BSLS_ASSERT(numCodePoints >= 0);

    IntPtr      ret = 0;      // return value -- number of code points advanced
    const char *next;         // 'next' is calculated during the loop as the
                              // starting position to start parsing the next
                              // code point, and assigned to 'string' between
                              // iterations.

    // Note that we keep 'string' pointing to the beginning of the Unicode
    // code point being processed, and only advance it to the next code point
    // between iterations.

    for (; true; ++ret, string = next) {
        next = string + 1;

        if (UNLIKELY(ret >= numCodePoints)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            // This is the first of 2 ways we can exit this function
            // successfully.

            BSLS_ASSERT(ret == numCodePoints);           // impossible to fail

            *status = 0;
            break;
        }

        // Note that if we leave this 'switch' without doing a 'continue' we'll
        // exit the loop at the bottom.

        switch (*reinterpret_cast<const unsigned char *>(string) >> 4) {
          case 0: {
            // binary: 0000xxxx: low ASCII and possibly '\0'

            if (!*string) {
                // '\0', end of input with no errors.  This is the second of
                // two ways we can successfully exit this function.

                *status = 0;
                break;
            }
          } continue;

          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            // binary: 0xxxxxxx: ASCII, but definitely not '\0'

          } continue;

          case 8:
          case 9:
          case 0xa:
          case 0xb: {
            // binary: 10xxxxxx: error: unexpected continuation octet

            *status = -1;

            // no 'continue'; exit the loop
          } break;

          case 0xc:
          case 0xd: {
            // binary: 110xxxxx: 2-octet sequence

            if (UNLIKELY(isNotContinuation(*next))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // invalid continuation

                *status = -1;
                break;
            }

            ++next;

            if (UNLIKELY(get2ByteValue(string) < k_MIN_2_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // non-minimal encoding

                *status = -1;
                break;
            }

          } continue;

          case 0xe: {
            // binary: 1110xxxx: 3 octet sequence

            if   (UNLIKELY(isNotContinuation(*next))
               || UNLIKELY(isNotContinuation(*++next))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // invalid continuations

                *status = -1;
                break;
            }

            ++next;

            int value = get3ByteValue(string);

            if (UNLIKELY(value < k_MIN_3_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // non-minimal encoding

                *status = -1;
                break;
            }

            if (UNLIKELY(isSurrogateValue(value))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // surrogate value

                *status = -1;
                break;
            }
          } continue;

          case 0xf: {
            // binary: 1111xxxx: 4 octet sequence (only legal if 11110xxx).

            if (UNLIKELY(*string & 8)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // binary: 11111xxx: illegal start of 5 (or more) octet
                // sequence

                *status = -1;
                break;
            }

            // binary: 11110xxx: legal start of 4 octet sequence

            if   (UNLIKELY(isNotContinuation(*next))
               || UNLIKELY(isNotContinuation(*++next))
               || UNLIKELY(isNotContinuation(*++next))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // invalid continuations

                *status = -1;
                break;
            }

            ++next;

            if (UNLIKELY(get4ByteValue(string) < k_MIN_4_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // non-minimal encoding

                *status = -1;
                break;
            }
          } continue;
        }

        break;
    }

    *result = string;
    return ret;
}

Utf8Util::IntPtr Utf8Util::advanceIfValid(int         *status,
                                          const char **result,
                                          const char  *string,
                                          size_type    length,
                                          IntPtr       numCodePoints)
{
    BSLS_ASSERT(status);
    BSLS_ASSERT(result);
    BSLS_ASSERT(string);
    BSLS_ASSERT(numCodePoints >= 0);

    IntPtr      ret = 0;      // return value -- number of code points advanced
    const char *next;         // 'next' is advanced during the loop to the
                              // starting position to start parsing the next
                              // code point, and assigned to 'string' between
                              // iterations.

    const char * const endOfInput = string + length;

    // Note that we keep 'string' pointing to the beginning of the Unicode
    // code point being processed, and only advance it to the next code point
    // between iterations.

    for (; true; ++ret, string = next) {
        next = string + 1;

        if (UNLIKELY(string >= endOfInput)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            // Success.  We have successfully parsed all the input.

            BSLS_ASSERT(string == endOfInput);           // impossible to fail

            *status = 0;
            break;
        }

        if (UNLIKELY(ret >= numCodePoints)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            // Success.  We have successfully advanced 'numCodePoints' code
            // points.

            BSLS_ASSERT(ret == numCodePoints);           // impossible to fail

            *status = 0;
            break;
        }

        // Note that if we leave this 'switch' without doing a 'continue'
        // (which we only do if we encounter an error), we'll exit the loop at
        // the bottom.

        switch (*reinterpret_cast<const unsigned char *>(string) >> 4) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            // binary: 0xxxxxxx: ASCII and possible '\0'

          } continue;

          case 8:
          case 9:
          case 0xa:
          case 0xb: {
            // binary: 10xxxxxx: unexpected continuation octet

            *status = -1;
          } break;

          case 0xc:
          case 0xd: {
            // binary: 110xxxxx: 2-octet sequence

            if (UNLIKELY(string + 2 > endOfInput)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // truncation of code point

                *status = -1;
                break;
            }

            if (UNLIKELY(isNotContinuation(*next))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // invalid continuation

                *status = -1;
                break;
            }

            ++next;

            if (UNLIKELY(get2ByteValue(string) < k_MIN_2_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // non-minimal encoding

                *status = -1;
                break;
            }
          } continue;

          case 0xe: {
            // binary: 1110xxxx: 3 octet sequence

            if (UNLIKELY(string + 3 > endOfInput)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // truncation of code point

                *status = -1;
                break;
            }

            if   (UNLIKELY(isNotContinuation(*next))
               || UNLIKELY(isNotContinuation(*++next))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // invalid continuations

                *status = -1;
                break;
            }

            ++next;

            int value = get3ByteValue(string);

            if (UNLIKELY(value < k_MIN_3_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // non-minimal encoding

                *status = -1;
                break;
            }

            if (UNLIKELY(isSurrogateValue(value))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // surrogate value

                *status = -1;
                break;
            }
          } continue;

          case 0xf: {
            // binary: 1111xxxx: 4 octet sequence (only legal if 11110xxx).

            if (UNLIKELY(*string & 8)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // binary: 11111xxx: invalid code point in all UTF-8 contexts.

                *status = -1;
                break;
            }

            // binary: 11110xxx: legal start to 4 octet sequence

            if (UNLIKELY(string + 4 > endOfInput)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // truncation of code point

                *status = -1;
                break;
            }

            if   (UNLIKELY(isNotContinuation(*next))
               || UNLIKELY(isNotContinuation(*++next))
               || UNLIKELY(isNotContinuation(*++next))) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // invalid continuations

                *status = -1;
                break;
            }

            ++next;

            if (UNLIKELY(get4ByteValue(string) < k_MIN_4_BYTE_VALUE)) {
                BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

                // non-minimal encoding

                *status = -1;
                break;
            }
          } continue;
        }

        break;
    }

    *result = string;
    return ret;
}

Utf8Util::IntPtr Utf8Util::advanceRaw(const char **result,
                                      const char  *string,
                                      IntPtr       numCodePoints)
{
    BSLS_ASSERT(result);
    BSLS_ASSERT(string);
    BSLS_ASSERT(numCodePoints >= 0);

    IntPtr ret = 0;        // return value, # of code points advanced

    for (; ret < numCodePoints; ++ret) {
        // There's a 'break' at the end of this loop, so any case in the switch
        // that leaves without doing a 'continue' will exit the loop.

        switch (*reinterpret_cast<const unsigned char *>(string) >> 4) {
          case 0: {
            // binary: 0000xxxx: low ASCII, possibly '\0'

            if (!*string) {
                // '\0': end of input

                break;
            }

            ++string;
          } continue;

          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            // binary: 0xxxxxxx: ASCII (definitely not '\0')

            ++string;
          } continue;

          case 8:
          case 9:
          case 0xa:
          case 0xb: {
            // binary: 10xxxxxx: We hit an unexpected continuation octet.

            BSLS_ASSERT(0 && "invalid UTF-8");
          } break;

          // The following are multi-octet sequences.  Since we're assuming
          // valid input, don't bother inspecting the continuation octets.

          case 0xc:
          case 0xd: {
            // binary: 110xxxxx: Start of two-octet sequence.

            string += 2;
          } continue;
          case 0xe: {
            // binary: 1110xxxx: Start of three-octet sequence.

            string += 3;
          } continue;
          case 0xf: {
            // binary: 1111xxxx: Start of four-octet sequence.

            BSLS_ASSERT(0 == (*string & 8));    // only legal if 11110xxx

            string += 4;
          } continue;
        }

        break;
    }

    BSLS_ASSERT(ret == numCodePoints || '\0' == *string); // impossible to fail

    *result = string;
    return ret;
}

Utf8Util::IntPtr Utf8Util::advanceRaw(const char **result,
                                      const char  *string,
                                      size_type    length,
                                      IntPtr       numCodePoints)
{
    BSLS_ASSERT(result);
    BSLS_ASSERT(string);
    BSLS_ASSERT(numCodePoints >= 0);

    IntPtr ret = 0;  // return value, # of code points advanced

    const char * const endOfInput = string + length;

    for (; true; ++ret) {
        // There's a 'break' at the end of this loop, so any case in the switch
        // that leaves without doing a 'continue' will exit the loop.

        if (UNLIKELY(string >= endOfInput)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            // Success

            BSLS_ASSERT(string == endOfInput);      // impossible to fail
                                                    // unless invalid UTF-8
            break;
        }

        if (UNLIKELY(ret >= numCodePoints)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            // Success

            BSLS_ASSERT(ret == numCodePoints);      // impossible to fail
            break;
        }

        switch (*reinterpret_cast<const unsigned char *>(string) >> 4) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            // binary: 0xxxxxxx: ASCII, possibly '\0'

            ++string;
          } continue;

          case 8:
          case 9:
          case 0xa:
          case 0xb: {
            // binary: 10xxxxxx: We hit an unexpected continuation octet.

            BSLS_ASSERT(0 && "invalid UTF-8");
          } break;

          // The following are multi-octet sequences.  Since we're assuming
          // valid input, don't bother inspecting the continuation octets.

          case 0xc:
          case 0xd: {
            // binary: 110xxxxx: Beginning of two-octet sequence.

            string += 2;
          } continue;

          case 0xe: {
            // binary: 1110xxxx: Beginning of three-octet sequence.

            string += 3;
          } continue;

          case 0xf: {
            // binary: 1111xxxx: Beginning of four-octet sequence.

            BSLS_ASSERT(0 == (*string & 8));    // only legal if 11110xxx

            // binary: 11110xxx: Legal beginning of four-octet sequence

            string += 4;
          } continue;
        }

        break;
    }

    *result = string;
    return ret;
}

// BDE_VERIFY pragma: push
// BDE_VERIFY pragma: -AN01     // 'codepoint' parameter renamed for line
                                // length.
// BDE_VERIFY pragma: -SP01     // 'FFFF' is not a typo.
// BDE_VERIFY pragma: -SP03     // 'codepnt' is not a typo.

int Utf8Util::appendUtf8Character(bsl::string *output, unsigned int codepnt)
{
    BSLS_ASSERT(output);

    ///IMPLEMENTATION NOTES
    ///--------------------
    // This UTF-8 documentation was copied verbatim from RFC 3629.  The
    // original version was downloaded from:
    //..
    //     http://tools.ietf.org/html/rfc3629
    //..
    ///////////////////////// BEGIN VERBATIM RFC TEXT /////////////////////////
    // Char number range   |        UTF-8 octet sequence
    //    (hexadecimal)    |              (binary)
    // --------------------+---------------------------------------------
    // 0000 0000-0000 007F | 0xxxxxxx
    // 0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    // 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    // 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    ////////////////////////// END VERBATIM RFC TEXT //////////////////////////

    if (codepnt < 0x80U) {
        output->push_back(static_cast<char>(codepnt));
        return 0;                                                     // RETURN
    }
    else if (codepnt < 0x800U) {
        output->push_back(static_cast<char>((codepnt >>   6)          | 0xC0));
        output->push_back(static_cast<char>((codepnt         & 0x3FU) | 0x80));
        return 0;                                                     // RETURN
    }
    else if (codepnt < 0x10000U) {
        output->push_back(static_cast<char>((codepnt  >> 12)          | 0xE0));
        output->push_back(static_cast<char>(((codepnt >>  6) & 0x3FU) | 0x80));
        output->push_back(static_cast<char>((codepnt         & 0x3FU) | 0x80));
        return 0;                                                     // RETURN
    }
    else if (codepnt < 0x110000U) {
        output->push_back(static_cast<char>((codepnt  >> 18)          | 0xF0));
        output->push_back(static_cast<char>(((codepnt >> 12) & 0x3FU) | 0x80));
        output->push_back(static_cast<char>(((codepnt >>  6) & 0x3FU) | 0x80));
        output->push_back(static_cast<char>((codepnt         & 0x3FU) | 0x80));
        return 0;                                                     // RETURN
    }

    // Invalid code point.
    return -1;
}

// BDE_VERIFY pragma: pop

int Utf8Util::getByteSize(const char* codepoint)
{
    BSLS_ASSERT_SAFE(isValidUtf8(codepoint));
    return utf8Size(codepoint[0]);
}

bool Utf8Util::isValid(const char **invalidString, const char *string)
{
    BSLS_ASSERT(invalidString);
    BSLS_ASSERT(string);

    return validateAndCountCodePoints(invalidString, string) >= 0;
}

bool Utf8Util::isValid(const char **invalidString,
                       const char  *string,
                       size_type    length)
{
    BSLS_ASSERT(invalidString);
    BSLS_ASSERT(string);
    BSLS_ASSERT(0 <= bsls::Types::IntPtr(length));

    return validateAndCountCodePoints(invalidString, string, length) >= 0;
}

Utf8Util::IntPtr Utf8Util::numCodePointsIfValid(const char **invalidString,
                                                const char  *string)
{
    BSLS_ASSERT(invalidString);
    BSLS_ASSERT(string);

    return validateAndCountCodePoints(invalidString, string);
}

Utf8Util::IntPtr Utf8Util::numCodePointsIfValid(const char **invalidString,
                                                const char  *string,
                                                size_type    length)
{
    BSLS_ASSERT(invalidString);
    BSLS_ASSERT(string);
    BSLS_ASSERT(0 <= bsls::Types::IntPtr(length));

    return validateAndCountCodePoints(invalidString, string, length);
}

Utf8Util::IntPtr Utf8Util::numCodePointsRaw(const char *string)
{
    BSLS_ASSERT(string);

    IntPtr count = 0;

    // Note that since we assume the string contains valid UTF-8, our work is
    // very simple.

    while (true) {
        switch ((*string >> 4) & 0xf) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            if (!*string) {
                return count;                                         // RETURN
            }
            ++string;
          } break;
          case 0xc:
          case 0xd: {
            string += 2;
          } break;
          case 0xe: {
            string += 3;
          } break;
          default: {
            string += 4;
          } break;
        }

        ++count;
    }
}

Utf8Util::IntPtr Utf8Util::numCodePointsRaw(const char *string,
                                            size_type   length)
{
    BSLS_ASSERT(string);

    IntPtr count = 0;

    // Note that since we assume the string contains valid UTF-8, our work is
    // very simple.

    const char *const end = string + length;

    while (string < end) {
        switch ((*string >> 4) & 0xf) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7: {
            ++string;
          } break;
          case 0xc:
          case 0xd: {
            string += 2;
          } break;
          case 0xe: {
            string += 3;
          } break;
          default: {
            string += 4;
          } break;
        }

        ++count;
    }

    BSLS_ASSERT(end == string);

    return count;
}

Utf8Util::IntPtr Utf8Util::numBytesIfValid(
                                        const bslstl::StringRef& string,
                                        IntPtr                   numCodePoints)
{
    BSLS_ASSERT(string.isEmpty() || string.data());
    BSLS_ASSERT(0 <= numCodePoints);

    size_t numBytes = 0;

    // Note that since we are assuming the string already passed one of the
    // validation functions our work is very simple.

    for (int i = 0; i < numCodePoints && numBytes < string.length(); ++i) {
        BSLS_ASSERT_SAFE(isValidUtf8(&string[numBytes]));
        numBytes += utf8Size(string[numBytes]);
    }

    if (numBytes > string.length()) {
        return -1;                                                    // RETURN
    }

    return numBytes;
}

}  // close package namespace

}  // close enterprise namespace

// ----------------------------------------------------------------------------
// Copyright 2015 Bloomberg Finance L.P.
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
