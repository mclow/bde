// bdem_berencoder.h                  -*-C++-*-
#ifndef INCLUDED_BDEM_BERENCODER
#define INCLUDED_BDEM_BERENCODER

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a BER encoder class.
//
//@CLASSES:
//   bdem_BerEncoder: BER encoder
//
//@SEE_ALSO: bdem_berdecoder, bdem_bdemencoder, baexml_encoder
//
//@AUTHOR: Rohan Bhindwale (rbhindwa),
//         Shezan Baig (sbaig),
//         Alexander Libman (alibman1)
//
//@DESCRIPTION:  The 'bdem_BerEncoder' class provided in this component
// contains a parameterized 'encode' function that encodes a specified
// value-semantic object into a specified stream.  There are two overloaded
// versions of this function: one that writes to an 'bsl::streambuf' and
// another than writes to an 'bsl::ostream'.
//
// This component encodes objects based on the X.690 BER specification.  It can
// only be used with types supported by the 'bdeat' framework.  In particular,
// types generated by the 'bas_codegen.pl' tool can be used.  Since the types
// are generated from XSD, the X.694 mappings are applied.
//
///Usage
///-----
// Suppose we have an XML schema inside a file called 'employee.xsd':
//..
//  <?xml version='1.0' encoding='UTF-8'?>
//  <xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'
//             xmlns:test='http://bloomberg.com/schemas/test'
//             targetNamespace='http://bloomberg.com/schemas/test'
//             elementFormDefault='unqualified'>
//
//      <xs:complexType name='Address'>
//          <xs:sequence>
//              <xs:element name='street' type='xs:string'/>
//              <xs:element name='city'   type='xs:string'/>
//              <xs:element name='state'  type='xs:string'/>
//          </xs:sequence>
//      </xs:complexType>
//
//      <xs:complexType name='Employee'>
//          <xs:sequence>
//              <xs:element name='name'        type='xs:string'/>
//              <xs:element name='homeAddress' type='test:Address'/>
//              <xs:element name='age'         type='xs:int'/>
//          </xs:sequence>
//      </xs:complexType>
//
//  </xs:schema>
//..
// Using the 'bas_codegen.pl' tool, we can generate C++ classes for this
// schema:
//..
//  $ bas_codegen.pl -g h -g cpp -p test employee.xsd
//..
// This tool will generate the header and implementation files for the
// 'test_address' and 'test_employee' components in the current directory.
//
// Now suppose we want to encode information about a particular employee using
// the BER encoding.  Note that we will use 'bdesb' stream buffers for in-core
// buffer management:
//..
//  #include <bdesb_memoutstreambuf.h>
//  #include <bdesb_fixedmeminstreambuf.h>
//
//  #include <test_employee.h>
//
//  #include <bdem_berencoder.h>
//  #include <bdem_berdecoder.h>
//
//  using namespace BloombergLP;
//
//  void usageExample()
//  {
//      bdesb_MemOutStreamBuf osb;
//
//      test::Employee bob;
//
//      bob.name()                 = "Bob";
//      bob.homeAddress().street() = "Some Street";
//      bob.homeAddress().city()   = "Some City";
//      bob.homeAddress().state()  = "Some State";
//      bob.age()                  = 21;
//
//      bdem_BerEncoder encoder;
//      int retCode = encoder.encode(&osb, bob);
//
//      assert(0 == retCode);
//..
// At this point, 'osb' contains a representation of 'bob' in BER format.  Now
// we will verify the contents of 'osb' using the 'bdem_berdecoder'
// component:
//..
//      bdesb_FixedMemInStreamBuf isb(osb.data(), osb.length());     // NO COPY
//      test::Employee            obj;
//
//      bdem_BerDecoder decoder;
//      retCode = decoder.decode(&isb, &obj);
//
//      assert(0                          == retCode);
//      assert(bob.name()                 == obj.name());
//      assert(bob.homeAddress().street() == obj.homeAddress().street());
//      assert(bob.homeAddress().city()   == obj.homeAddress().city());
//      assert(bob.homeAddress().state()  == obj.homeAddress().state());
//      assert(bob.age()                  == obj.age());
//  }
//..

#ifndef INCLUDED_BDESCM_VERSION
#include <bdescm_version.h>
#endif

#ifndef INCLUDED_BDEM_BERCONSTANTS
#include <bdem_berconstants.h>
#endif

#ifndef INCLUDED_BDEM_BERENCODEROPTIONS
#include <bdem_berencoderoptions.h>
#endif

#ifndef INCLUDED_BDEM_BERUNIVERSALTAGNUMBER
#include <bdem_beruniversaltagnumber.h>
#endif

#ifndef INCLUDED_BDEM_BERUTIL
#include <bdem_berutil.h>
#endif

#ifndef INCLUDED_BDEAT_ARRAYFUNCTIONS
#include <bdeat_arrayfunctions.h>
#endif

#ifndef INCLUDED_BDEAT_ATTRIBUTEINFO
#include <bdeat_attributeinfo.h>
#endif

#ifndef INCLUDED_BDEAT_CHOICEFUNCTIONS
#include <bdeat_choicefunctions.h>
#endif

#ifndef INCLUDED_BDEAT_CUSTOMIZEDTYPEFUNCTIONS
#include <bdeat_customizedtypefunctions.h>
#endif

#ifndef INCLUDED_BDEAT_ENUMFUNCTIONS
#include <bdeat_enumfunctions.h>
#endif

#ifndef INCLUDED_BDEAT_FORMATTINGMODE
#include <bdeat_formattingmode.h>
#endif

#ifndef INCLUDED_BDEAT_NULLABLEVALUEFUNCTIONS
#include <bdeat_nullablevaluefunctions.h>
#endif

#ifndef INCLUDED_BDEAT_SEQUENCEFUNCTIONS
#include <bdeat_sequencefunctions.h>
#endif

#ifndef INCLUDED_BDEAT_TYPECATEGORY
#include <bdeat_typecategory.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BDEUT_STRINGREF
#include <bdeut_stringref.h>
#endif

#ifndef INCLUDED_BDESB_MEMOUTSTREAMBUF
#include <bdesb_memoutstreambuf.h>
#endif

#ifndef INCLUDED_BSLS_OBJECTBUFFER
#include <bsls_objectbuffer.h>
#endif

#ifndef INCLUDED_BSL_OSTREAM
#include <bsl_ostream.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

#ifndef INCLUDED_BSL_TYPEINFO
#include <bsl_typeinfo.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

namespace BloombergLP {

struct bdem_BerEncoder_encodeProxy;
class  bdem_BerEncoder_Visitor;
class  bdem_BerEncoder_UniversalElementVisitor;
class  bdem_BerEncoder_LevelGuard;

                           // =====================
                           // class bdem_BerEncoder
                           // =====================

class bdem_BerEncoder {
    // This class contains the parameterized 'encode' functions that encode
    // 'bdeat' types to an outgoing stream in BER format.

  private:
    // FRIENDS
    friend struct bdem_BerEncoder_encodeProxy;
    friend class  bdem_BerEncoder_Visitor;
    friend class  bdem_BerEncoder_UniversalElementVisitor;
    friend class  bdem_BerEncoder_LevelGuard;

    // PRIVATE TYPES
    class MemOutStream : public bsl::ostream {
        // This class provides stream for logging using 'bdesb_MemOutStreamBuf'
        // as a streambuf.  The logging stream is created on demand, i.e.,
        // during the first attempt to log message.

        // DATA
        bdesb_MemOutStreamBuf d_sb;

        // NOT IMPLEMENTED
        MemOutStream(const MemOutStream&);
        MemOutStream& operator=(const MemOutStream&);

      public:
        // CREATORS
        MemOutStream(bslma_Allocator *basicAllocator = 0);
            // Create a new stream using the specified 'basicAllocator'.

        virtual ~MemOutStream();
            // Destroy this stream and release memory back to the allocator.
            //
            // Although the compiler should generate this destructor
            // implicitly, xlC 8 breaks when the destructor is called by name
            // unless it is explicitly declared.

        // MANIPULATORS
        void reset();
            // Reset the internal streambuf to empty.

        // ACCESSORS
        const char *data() const;
            // Return a pointer to the memory containing the formatted values
            // formatted to this stream.  The data is not null-terminated
            // unless a null character was appended onto this stream.

        int length() const;
            // Return the length of of the formatted data, including null
            // characters appended to the stream, if any.
    };

  public:
    // PUBLIC TYPES
    enum ErrorSeverity {
        BDEM_BER_SUCCESS = 0x00,
        BDEM_BER_ERROR   = 0x02
    };

  private:
    // DATA
    const bdem_BerEncoderOptions    *d_options;      // held, not owned
    bslma_Allocator                 *d_allocator;    // held, not owned

    bsls_ObjectBuffer<MemOutStream>  d_logArea;
        // placeholder for MemOutStream

    MemOutStream                    *d_logStream;
        // if not zero, log stream was created at the moment of first
        // logging and must be destroyed

    ErrorSeverity                    d_severity;       // error severity

    bsl::streambuf                  *d_streamBuf;      // held, not owned
    int                              d_currentDepth;   // current depth

    // NOT IMPLEMENTED
    bdem_BerEncoder(const bdem_BerEncoder&);
    bdem_BerEncoder& operator=(const bdem_BerEncoder&);

    // PRIVATE MANIPULATORS
    ErrorSeverity logMsg(const char                    *msg,
                         bdem_BerConstants::TagClass    tagClass,
                         int                            tagNumber,
                         const char                    *name  = 0,
                         int                            index = -1);
        // Log 'msg'.  Return the 'this->ErrorSeverity()'.

    ErrorSeverity logError(bdem_BerConstants::TagClass  tagClass,
                           int                          tagNumber,
                           const char                  *name  = 0,
                           int                          index = -1);
        // Log error and upgrade the severity level.  Return the
        // 'this->ErrorSeverity()'.

    bsl::ostream& logStream();
        // Return the stream for logging.  Note the if stream has not
        // been created yet, it will be created during this call.

    int encodeImpl(const bsl::vector<char>& value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::Array);

    template <typename TYPE>
    int encodeArrayImpl(const TYPE&         value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::Choice);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::NullableValue);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::CustomizedType);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::Enumeration);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::Sequence);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::Simple);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::Array);

    template <typename TYPE>
    int encodeImpl(const TYPE&              value,
               bdem_BerConstants::TagClass  tagClass,
               int                          tagNumber,
               int                          formattingMode,
               bdeat_TypeCategory::DynamicType);

  public:
    // CREATORS
    bdem_BerEncoder(const bdem_BerEncoderOptions *options = 0,
                    bslma_Allocator              *basicAllocator = 0);
        // Construct a encoder object using the optionally specified 'options',
        // and 'basicAllocator'.

    ~bdem_BerEncoder();
        // Destroy this object.  This destruction has no effect on objects
        // pointed-to by the pointers provided at construction.

    template <typename TYPE>
    int encode(bsl::streambuf *streamBuf, const TYPE& value);
        // Encode the specified non-modifiable 'value' to the specified
        // 'streamBuf'.  Return 0 on success, and a non-zero value otherwise.

    template <typename TYPE>
    int encode(bsl::ostream& stream, const TYPE& value);
        // Encode the specified non-modifiable 'value' to the specified
        // 'stream'.   Return 0 on success, and a non-zero value otherwise.
        // Note that 'stream' will be invalidated if the encoding fails.

    //ACCESSORS
    const bdem_BerEncoderOptions *options() const;
        // Return pointer to the options.

    ErrorSeverity  errorSeverity() const;
        // Return the severity of the most severe warning or error encountered
        // during the last call to the 'decode' method.  The severity is reset
        // each time 'decode' is called.

    bdeut_StringRef loggedMessages() const;
        // Return a string containing any error, warning, or trace messages
        // that were logged during the last call to the 'decode' method.  The
        // log is reset each time 'decode' is called.
};

// ---  Anything below this line is implementation specific.  Do not use.  ----

        // ========================================
        // private class bdem_BerEncoder_LevelGuard
        // ========================================

class bdem_BerEncoder_LevelGuard {
    // This class serves the purpose to automatically increment-decrement
    // the current depth level.

    // DATA
    bdem_BerEncoder *d_encoder;

    // NOT IMPLEMENTED
    bdem_BerEncoder_LevelGuard(bdem_BerEncoder_LevelGuard&);
    bdem_BerEncoder_LevelGuard& operator=(bdem_BerEncoder_LevelGuard&);

  public:
    // CREATORS
    bdem_BerEncoder_LevelGuard(bdem_BerEncoder *encoder);

    ~bdem_BerEncoder_LevelGuard();
};

                 // =====================================
                 // private class bdem_BerEncoder_Visitor
                 // =====================================

class bdem_BerEncoder_Visitor {
    // This class is used as a visitor for visiting contained objects during
    // encoding.  Produces always BER elements with CONTEXT_SPECIFIC BER tag.

    // DATA
    bdem_BerEncoder             *d_encoder;   // encoder to write data to
    bdem_BerEncoder_LevelGuard   d_levelGuard;

    // NOT IMPLEMENTED
    bdem_BerEncoder_Visitor(const bdem_BerEncoder_Visitor&);
    bdem_BerEncoder_Visitor& operator=(const bdem_BerEncoder_Visitor&);

  public:
    // CREATORS
    bdem_BerEncoder_Visitor(bdem_BerEncoder *encoder);
    ~bdem_BerEncoder_Visitor();

    // MANIPULATORS
    template <typename TYPE, typename INFO>
    int operator()(const TYPE& value, const INFO& info);
};

         // =====================================================
         // private class bdem_BerEncoder_UniversalElementVisitor
         // =====================================================

class bdem_BerEncoder_UniversalElementVisitor {
    // This class is used as a visitor for visiting the top-level element and
    // also array elements during encoding.  This class is required so that the
    // universal tag number of the element can be determined when the element
    // is visited.

    // PRIVATE DATA MEMBERS
    bdem_BerEncoder            *d_encoder;   // streambuf to write data to
    int                         d_formattingMode;  // formatting mode to use
    bdem_BerEncoder_LevelGuard  d_levelGuard;

  private:
    // NOT IMPLEMENTED
    bdem_BerEncoder_UniversalElementVisitor(
                           const bdem_BerEncoder_UniversalElementVisitor&);
    bdem_BerEncoder_UniversalElementVisitor& operator=(
                           const bdem_BerEncoder_UniversalElementVisitor&);

  public:
    // CREATORS
    bdem_BerEncoder_UniversalElementVisitor(bdem_BerEncoder *encoder,
                                            int             formattingMode);

    ~bdem_BerEncoder_UniversalElementVisitor();

    // MANIPULATORS
    template <typename TYPE>
    int operator()(const TYPE& value);
};

// ===========================================================================
//                               PROXY CLASSES
// ===========================================================================

                 // ==================================
                 // struct bdem_BerEncoder_encodeProxy
                 // ==================================

struct bdem_BerEncoder_encodeProxy {
    // Component-private struct.  Provides accessor that
    // that keeps current context and can be used in
    // different bdeat Category Functions.

    // DATA MEMBERS
    bdem_BerEncoder             *d_encoder;
    bdem_BerConstants::TagClass  d_tagClass;
    int                          d_tagNumber;
    int                          d_formattingMode;

    // CREATORS
    // Creators have been omitted to allow simple static initialization of
    // this struct.

    // FUNCTIONS
    template <typename TYPE>
    int operator()(const TYPE& object, bslmf_Nil);

    template <typename TYPE, typename ANY_CATEGORY>
    int operator()(const TYPE& object, ANY_CATEGORY category);

    template <typename TYPE>
    int operator()(const TYPE& object);
};

// ===========================================================================
//                      INLINE FUNCTION DEFINITIONS
// ===========================================================================

                         // -----------------------------------
                         // class bdem_BerEncoder::MemOutStream
                         // -----------------------------------

inline
bdem_BerEncoder::MemOutStream::MemOutStream(bslma_Allocator *basicAllocator)
: bsl::ostream(0)
, d_sb(bslma_Default::allocator(basicAllocator))
{
    rdbuf(&d_sb);
}

// MANIPULATORS
inline
void bdem_BerEncoder::MemOutStream::reset()
{
    d_sb.reset();
}

// ACCESSORS
inline
const char *bdem_BerEncoder::MemOutStream::data() const
{
    return d_sb.data();
}

inline
int bdem_BerEncoder::MemOutStream::length() const
{
    return (int)d_sb.length();
}

              // ---------------------------------
              // class bdem_BerEncoder::LevelGuard
              // ---------------------------------

inline
bdem_BerEncoder_LevelGuard::bdem_BerEncoder_LevelGuard(
                                                      bdem_BerEncoder *encoder)
: d_encoder (encoder)
{
    ++d_encoder->d_currentDepth;
}

inline
bdem_BerEncoder_LevelGuard::~bdem_BerEncoder_LevelGuard()
{
    --d_encoder->d_currentDepth;
}

              // ----------------------------------
              // struct bdem_BerEncoder_encodeProxy
              // ----------------------------------

template <typename TYPE>
inline
int bdem_BerEncoder_encodeProxy::operator()(const TYPE&, bslmf_Nil)
{
    BSLS_ASSERT_SAFE(0);
    return -1;
}

template <typename TYPE, typename ANY_CATEGORY>
inline
int bdem_BerEncoder_encodeProxy::operator()(const TYPE&  object,
                                            ANY_CATEGORY category)
{
    return d_encoder->encodeImpl(object,
                                 d_tagClass,
                                 d_tagNumber,
                                 d_formattingMode,
                                 category);
}

template <typename TYPE>
inline
int bdem_BerEncoder_encodeProxy::operator()(const TYPE& object)
{
    typedef typename
    bdeat_TypeCategory::Select<TYPE>::Type TypeCategory;

    return this->operator()(object, TypeCategory());
}

                         // ---------------------
                         // class bdem_BerEncoder
                         // ---------------------

// ACCESSORS
inline
const bdem_BerEncoderOptions *bdem_BerEncoder::options() const
{
    return d_options;
}

inline
bdem_BerEncoder::ErrorSeverity bdem_BerEncoder::errorSeverity() const
{
    return d_severity;
}

inline
bdeut_StringRef bdem_BerEncoder::loggedMessages() const
{
    if (d_logStream) {
        return bdeut_StringRef(d_logStream->data(), d_logStream->length());
    }

    return bdeut_StringRef();
}

inline
bsl::ostream& bdem_BerEncoder::logStream()
{
    if (d_logStream == 0) {
        d_logStream = new(d_logArea.buffer()) MemOutStream(d_allocator);
    }
    return *d_logStream;
}

template <typename TYPE>
int bdem_BerEncoder::encode(bsl::streambuf *streamBuf, const TYPE& value)
{
    BSLS_ASSERT(!d_streamBuf);

    d_streamBuf = streamBuf;
    d_severity  = BDEM_BER_SUCCESS;

    if (d_logStream != 0) {
        d_logStream->reset();
    }

    d_currentDepth = 0;

    int rc;

    if (! d_options) {
        bdem_BerEncoderOptions options;  // temporary options object
        d_options = &options;
        bdem_BerEncoder_UniversalElementVisitor
                            visitor(this, bdeat_FormattingMode::BDEAT_DEFAULT);

        rc = visitor(value);
        d_options = 0;
    }
    else {
        bdem_BerEncoder_UniversalElementVisitor
                            visitor(this, bdeat_FormattingMode::BDEAT_DEFAULT);
        rc = visitor(value);
    }

    d_streamBuf = 0;
    return rc;
}

template <typename TYPE>
int bdem_BerEncoder::encode(bsl::ostream& stream, const TYPE& value)
{
    if (!stream.good()) {
        return -1;
    }

    if (0 != this->encode(stream.rdbuf(), value)) {
        stream.setstate(bsl::ios_base::failbit);
        return -1;
    }
    return 0;
}

// PRIVATE MANIPULATORS
template <typename TYPE>
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          formattingMode,
                                bdeat_TypeCategory::Choice)
{
    enum { BDEM_SUCCESS = 0, BDEM_FAILURE = -1 };

    const bdem_BerConstants::TagType tagType =
                                           bdem_BerConstants::BDEM_CONSTRUCTED;

    int rc = bdem_BerUtil::putIdentifierOctets(d_streamBuf,
                                               tagClass,
                                               tagType,
                                               tagNumber);
    if (rc | bdem_BerUtil::putIndefiniteLengthOctet(d_streamBuf)) {
        return BDEM_FAILURE;                                          // RETURN
    }

    const bool isUntagged =
                         formattingMode & bdeat_FormattingMode::BDEAT_UNTAGGED;

    if (!isUntagged) {
        // According to X.694 (clause 20.4), an XML choice (not anonymous)
        // element is encoded as a sequence with 1 element.

        rc = bdem_BerUtil::putIdentifierOctets(
                                      d_streamBuf,
                                      bdem_BerConstants::BDEM_CONTEXT_SPECIFIC,
                                      tagType,
                                      0);
        if (rc | bdem_BerUtil::putIndefiniteLengthOctet(d_streamBuf)) {
            return BDEM_FAILURE;
        }
    }

    const int selectionId = bdeat_ChoiceFunctions::selectionId(value);

    if (bdeat_ChoiceFunctions::BDEAT_UNDEFINED_SELECTION_ID != selectionId) {

        bdem_BerEncoder_Visitor visitor(this);

        if (0 != bdeat_ChoiceFunctions::accessSelection(value, visitor)) {
            return BDEM_FAILURE;                                      // RETURN
        }
    }

    if (!isUntagged) {
        // According to X.694 (clause 20.4), an XML choice (not anonymous)
        // element is encoded as a sequence with 1 element.

        // Don't waste time checking the result of this call -- the only thing
        // that can go wrong is eof, which will happen again when we call it
        // again below.
        bdem_BerUtil::putEndOfContentOctets(d_streamBuf);
    }

    return bdem_BerUtil::putEndOfContentOctets(d_streamBuf);
}

template <typename TYPE>
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          formattingMode,
                                bdeat_TypeCategory::NullableValue)
{
    enum { BDEM_SUCCESS = 0, BDEM_FAILURE = -1 };

    bool isNillable = formattingMode & bdeat_FormattingMode::BDEAT_NILLABLE;

    if (isNillable) {

        // nillable is encoded in BER as a sequence with one optional element

        int rc = bdem_BerUtil::putIdentifierOctets(
                                           d_streamBuf,
                                           tagClass,
                                           bdem_BerConstants::BDEM_CONSTRUCTED,
                                           tagNumber);
        if (rc | bdem_BerUtil::putIndefiniteLengthOctet(d_streamBuf)) {
            return BDEM_FAILURE;
        }

        if (!bdeat_NullableValueFunctions::isNull(value)) {

            bdem_BerEncoder_encodeProxy proxy1 = {
                this,
                bdem_BerConstants::BDEM_CONTEXT_SPECIFIC, // tagClass
                0,                                   // tagNumber
                formattingMode
            };

            if (0 != bdeat_NullableValueFunctions::accessValue(value, proxy1))
            {
                return BDEM_FAILURE;
            }
        } // end of bdeat_NullableValueFunctions::isNull(...)

        return bdem_BerUtil::putEndOfContentOctets(d_streamBuf);
    } // end of isNillable

    if (!bdeat_NullableValueFunctions::isNull(value)) {

        bdem_BerEncoder_encodeProxy proxy2 = { this,
                                              tagClass,
                                              tagNumber,
                                              formattingMode };

        if (0 != bdeat_NullableValueFunctions::accessValue(value, proxy2)) {
                return BDEM_FAILURE;
        }

    }

    return BDEM_SUCCESS;
}

template <typename TYPE>
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          formattingMode,
                                bdeat_TypeCategory::CustomizedType)
{
    typedef typename
    bdeat_CustomizedTypeFunctions::BaseType<TYPE>::Type BaseType;

    typedef typename
    bdeat_TypeCategory::Select<BaseType>::Type          BaseTypeCategory;

    int rc = encodeImpl(
                      bdeat_CustomizedTypeFunctions::convertToBaseType(value),
                      tagClass,
                      tagNumber,
                      formattingMode,
                      BaseTypeCategory());

    return rc;
}

template <typename TYPE>
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          ,
                                bdeat_TypeCategory::Enumeration)
{
    int rc = bdem_BerUtil::putIdentifierOctets(
                                             d_streamBuf,
                                             tagClass,
                                             bdem_BerConstants::BDEM_PRIMITIVE,
                                             tagNumber);

    int intValue;
    bdeat_EnumFunctions::toInt(&intValue, value);

    rc |= bdem_BerUtil::putValue(d_streamBuf, intValue);

    return rc;
}

template <typename TYPE>
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          ,
                                bdeat_TypeCategory::Sequence)
{
    bdem_BerEncoder_Visitor visitor(this);

    int rc = bdem_BerUtil::putIdentifierOctets(
                                           d_streamBuf,
                                           tagClass,
                                           bdem_BerConstants::BDEM_CONSTRUCTED,
                                           tagNumber);
    rc |= bdem_BerUtil::putIndefiniteLengthOctet(d_streamBuf);
    if (rc) {
        return rc;
    }

    rc = bdeat_SequenceFunctions::accessAttributes(value, visitor);
    rc |= bdem_BerUtil::putEndOfContentOctets(d_streamBuf);

    return rc;
}

template <typename TYPE>
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          ,
                                bdeat_TypeCategory::Simple)
{
    int rc = bdem_BerUtil::putIdentifierOctets(
                                             d_streamBuf,
                                             tagClass,
                                             bdem_BerConstants::BDEM_PRIMITIVE,
                                             tagNumber);
    rc |= bdem_BerUtil::putValue(d_streamBuf, value);

    return rc;
}

template <typename TYPE>
inline
int bdem_BerEncoder::encodeImpl(const TYPE& value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          formattingMode,
                                bdeat_TypeCategory::Array)
{
    enum { BDEM_SUCCESS = 0,  BDEM_FAILURE = -1 };

    if (d_currentDepth <= 1 || tagClass == bdem_BerConstants::BDEM_UNIVERSAL) {
        return BDEM_FAILURE;
    }
    // Note: bsl::vector<char> is handled as a special case in the CPP file.
    return this->encodeArrayImpl(value,
                                 tagClass,
                                 tagNumber,
                                 formattingMode);
}

template <typename TYPE>
int
bdem_BerEncoder::encodeArrayImpl(const TYPE&                  value,
                                 bdem_BerConstants::TagClass  tagClass,
                                 int                          tagNumber,
                                 int                          formattingMode)
{
    enum { BDEM_FAILURE = -1 };

    const bdem_BerConstants::TagType tagType =
                                           bdem_BerConstants::BDEM_CONSTRUCTED;

    int rc = bdem_BerUtil::putIdentifierOctets(d_streamBuf,
                                               tagClass,
                                               tagType,
                                               tagNumber);
    rc |= bdem_BerUtil::putIndefiniteLengthOctet(d_streamBuf);
    if (rc) {
        return BDEM_FAILURE;                                          // RETURN
    }

    bdem_BerEncoder_UniversalElementVisitor visitor(this,
                                                    formattingMode);

    const int size = (int)bdeat_ArrayFunctions::size(value);

    for (int i = 0; i < size; ++i) {
        if (0 != bdeat_ArrayFunctions::accessElement(value, visitor, i)) {

            this->logError(tagClass,
                           tagNumber,
                           0,  // bdeat_TypeName::name(value),
                           i);

            return BDEM_FAILURE;                                      // RETURN
        }
    }

    return bdem_BerUtil::putEndOfContentOctets(d_streamBuf);
}

template <typename TYPE>
inline
int bdem_BerEncoder::encodeImpl(const TYPE&                  value,
                                bdem_BerConstants::TagClass  tagClass,
                                int                          tagNumber,
                                int                          formattingMode,
                                bdeat_TypeCategory::DynamicType)
{
    bdem_BerEncoder_encodeProxy proxy = {
        this,
        tagClass,
        tagNumber,
        formattingMode
    };

    return bdeat_TypeCategoryUtil::accessByCategory(value, proxy);
}

                 // -------------------------------------
                 // private class bdem_BerEncoder_Visitor
                 // -------------------------------------

// CREATORS
inline
bdem_BerEncoder_Visitor::bdem_BerEncoder_Visitor(bdem_BerEncoder *encoder)
: d_encoder(encoder)
, d_levelGuard(encoder)
{
}

inline
bdem_BerEncoder_Visitor::~bdem_BerEncoder_Visitor()
{
}

// MANIPULATORS
template <typename TYPE, typename INFO>
inline
int bdem_BerEncoder_Visitor::operator()(const TYPE& value, const INFO& info)
{
    typedef typename
    bdeat_TypeCategory::Select<TYPE>::Type TypeCategory;

    int rc = d_encoder->encodeImpl(
                              value,
                              bdem_BerConstants::BDEM_CONTEXT_SPECIFIC,
                              info.id(),
                              info.formattingMode(),
                              TypeCategory());

    if (rc) {
        d_encoder->logError(bdem_BerConstants::BDEM_CONTEXT_SPECIFIC,
                            info.id(),
                            info.name());
    }

    return rc;
}

         // -----------------------------------------------------
         // private class bdem_BerEncoder_UniversalElementVisitor
         // -----------------------------------------------------

// CREATORS
inline
bdem_BerEncoder_UniversalElementVisitor::
bdem_BerEncoder_UniversalElementVisitor(bdem_BerEncoder *encoder,
                                        int              formattingMode)
: d_encoder(encoder)
, d_formattingMode(formattingMode)
, d_levelGuard(encoder)
{
}

inline
bdem_BerEncoder_UniversalElementVisitor::
~bdem_BerEncoder_UniversalElementVisitor()
{
}

// MANIPULATORS
template <typename TYPE>
int bdem_BerEncoder_UniversalElementVisitor::operator()(const TYPE& value)
{
    enum { BDEM_SUCCESS = 0, BDEM_FAILURE = -1 };

    typedef typename
    bdeat_TypeCategory::Select<TYPE>::Type TypeCategory;

    bdem_BerUniversalTagNumber::Value tagNumber
           = bdem_BerUniversalTagNumber::select(value, d_formattingMode);

    if (d_encoder->encodeImpl(value,
                              bdem_BerConstants::BDEM_UNIVERSAL,
                              static_cast<int>(tagNumber),
                              d_formattingMode,
                              TypeCategory())) {
        d_encoder->logError(bdem_BerConstants::BDEM_UNIVERSAL,
                            tagNumber,
                            0  // bdeat_TypeName::name(value)
                           );
        return BDEM_FAILURE;
    }

    return BDEM_SUCCESS;
}

}  // close namespace BloombergLP

#endif

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2007
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
