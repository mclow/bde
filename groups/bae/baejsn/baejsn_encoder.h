// baejsn_encoder.h                                                   -*-C++-*-
#ifndef INCLUDED_BAEJSN_ENCODER
#define INCLUDED_BAEJSN_ENCODER

#ifndef INCLUDED_BDES_IDENT
#include <bdes_ident.h>
#endif
BDES_IDENT("$Id: $")

//@PURPOSE: Provide a JSON encoder for 'bdeat' compatible types.
//
//@CLASSES:
// baejsn_Encoder: JSON decoder for 'bdeat'-compliant types
//
//@SEE_ALSO: baejsn_decoder, baejsn_printutil
//
//@AUTHOR: Raymond Chiu (schiu49), Rohan Bhindwale (rbhindwa)
//
//@DESCRIPTION: This component provides a class, 'baejsn_Encoder', for encoding
// value-semantic objects in the JSON format.  In particular, the 'class'
// contains a parameterized 'encode' function that encodes an object into a
// specified stream.  There are two overloaded versions of this function:
//..
//: o one that writes to a 'bsl::streambuf'
//: o one that writes to an 'bsl::ostream'
//..
// This component can be used with types that support the 'bdeat' framework
// (see the 'bdeat' package for details), which is a compile-time interface for
// manipulating struct-like and union-like objects.  In particular, types
// generated by the 'bas_codegen.pl' tool, and dynamic types such as
// 'bcem_Aggregate' can be encoded using this 'class'.  The 'encode' function
// can be invoked on any object that satisfies the requirements of a sequence,
// choice, or array object as defined in the 'bdeat_sequencefunctions',
// 'bdeat_choicefunctions', and 'bdeat_arrayfunctions' components.
//
// Although the JSON format is easy to read and write and is very useful for
// debugging, it is relatively expensive to encode and decode and relatively
// bulky to transmit.  It is more efficient to use a binary encoding (such as
// BER) if the encoding format is under your control (see 'bdem_berencoder').
//
///Encoding Format for a Simple Type
///---------------------------------
// The following table describes how various Simple types are encoded.
//..
//  Simple Type          JSON Type  Notes
//  -----------          ---------  -----
//  char                 number     The integer value of the character
//  unsigned char        number     The integer value of the character
//  int                  number
//  unsigned int         number
//  bsls::Types::Int64   number
//  bsls::Types::Uint64  number
//  float                number
//  double               number
//  char *               string
//  bsl::string          string
//  bdet_Date            string     ISO 8601 format
//  bdet_DateTz          string     ISO 8601 format
//  bdet_Time            string     ISO 8601 format
//  bdet_TimeTz          string     ISO 8601 format
//  bdet_DatetimeTz      string     ISO 8601 format
//  bdet_DatetimeTz      string     ISO 8601 format
//..
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Encoding a 'bas_codegen.pl'-generated object into JSON
///-----------------------------------------------------------------
// Consider that we want to exchange an employee's information between two
// processes.  To allow this information exchange we will define the XML schema
// representation for that class, use 'bas_codegen.pl' to create the 'Employee'
// 'class' for storing that information, populate an 'Employee' object, and
// encode that object using the baejsn encoder.
//
// First, we will define the XML schema inside a file called 'employee.xsd':
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
//      <xs:element name='Employee' type='test:Employee'/>
//
//  </xs:schema>
//..
// Then, we will use the 'bas_codegen.pl' tool, to generate the C++ classes for
// this schema.  The following command will generate the header and
// implementation files for the all the classes in the 'test_messages'
// components in the current directory:
//..
//  $ bas_codegen.pl -m msg -p test xsdfile.xsd
//..
// Next, we will populate a 'test::Employee' object:
//..
//  test::Employee employee;
//  employee.name()                 = "Bob";
//  employee.homeAddress().street() = "Lexington Ave";
//  employee.homeAddress().city()   = "New York City";
//  employee.homeAddress().state()  = "New York";
//  employee.age()                  = 21;
//..
// Then, we will create a 'baejsn_Encoder' object:
//..
//  baejsn_Encoder encoder;
//..
// Now, we will output this object in the JSON format by invoking the 'encode'
// method of the encoder.  We will also create a 'baejsn_EncoderOptions' object
// that allows us to specify that the encoding should be done in a pretty
// format, and what the initial indent level and spaces per level should be.
// We will then pass that object to the 'encode' method:
//..
//  bsl::ostringstream os;
//
//  baejsn_EncoderOptions options;
//  options.setEncodingStyle(baejsn_EncoderOptions::BAEJSN_PRETTY);
//  options.setInitialIndentLevel(1);
//  options.setSpacesPerLevel(4);
//
//  const int rc = encoder.encode(os, employee, options);
//  assert(!rc);
//  assert(os);
//..
// Finally, we will verify that the output is as expected:
//..
//  const char EXP_OUTPUT[] = "    {\n"
//                            "        \"name\" : \"Bob\",\n"
//                            "        \"homeAddress\" : {\n"
//                            "            \"street\" : \"Lexington Ave\",\n"
//                            "            \"city\" : \"New York City\",\n"
//                            "            \"state\" : \"New York\"\n"
//                            "        },\n"
//                            "        \"age\" : 21\n"
//                            "    }";
//
//  assert(EXP_OUTPUT == os.str());
//..
//
///Example 2: Encoding a 'bcem_Aggregate' Object into JSON
///-------------------------------------------------------
// Now consider that we want to exchange an employee's information between two
// processes using a 'bcem_Aggregate'.  To allow this information exchange we
// will define the 'bdem_Schema' to represent the meta-data, construct the
// 'bcem_Aggregate' object with that schema, populate it, and encode that
// object using the baejsn encoder.
//
// First, we create a 'bdem_Schema' object:
//..
//  bcema_SharedPtr<bdem_Schema> schema(new bdem_Schema);
//
//  bdem_RecordDef *addressRecord = schema->createRecord("Address");
//  addressRecord->appendField(bdem_ElemType::BDEM_STRING, "street");
//  addressRecord->appendField(bdem_ElemType::BDEM_STRING, "city");
//  addressRecord->appendField(bdem_ElemType::BDEM_STRING, "state");
//
//  bdem_RecordDef *employeeRecord = schema->createRecord("Employee");
//  employeeRecord->appendField(bdem_ElemType::BDEM_STRING, "name");
//  employeeRecord->appendField(bdem_ElemType::BDEM_LIST,
//                              addressRecord,
//                              "homeAddress");
//  employeeRecord->appendField(bdem_ElemType::BDEM_INT, "age");
//..
// Then, we create a 'bcem_Aggregate' object using the schema and populate it
// with values:
//..
//  bcem_Aggregate employee(schema, "Employee");
//
//  employee["name"].setValue("Bob");
//  employee["homeAddress"]["street"].setValue("Some Street");
//  employee["homeAddress"]["city"].setValue("Some City");
//  employee["homeAddress"]["state"].setValue("Some State");
//  employee["age"].setValue(21);
//..
// Next, we create a 'baejsn_Encoder':
//..
//  baejsn_Encoder encoder;
//..
// Now, we will output this object in the JSON format by invoking the 'encode'
// method of the encoder.  We will pass a default 'baejsn_EncoderOptions'
// object to the 'encode' method which will result in the output being in a
// compact format:
//..
//  bsl::ostringstream os;
//
//  baejsn_EncoderOptions options;
//  const int rc = encoder.encode(os, employee, options);
//  assert(!rc);
//  assert(os);
//..
// Finally, we will verify that the output is as expected:
//..
//  const char EXP_OUTPUT[] = "{\"name\":\"Bob\",\"homeAddress\":{\"street\":"
//                            "\"Lexington Ave\",\"city\":\"New York City\","
//                            "\"state\":\"Some State\"},\"age\":21}";
//  assert(EXP_OUTPUT == os.str());
//..

#ifndef INCLUDED_BAESCM_VERSION
#include <baescm_version.h>
#endif

#ifndef INCLUDED_BAEJSN_ENCODEROPTIONS
#include <baejsn_encoderoptions.h>
#endif

#ifndef INCLUDED_BAEJSN_PRINTUTIL
#include <baejsn_printutil.h>
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

#ifndef INCLUDED_BDEAT_CUSTOMIZEDTYPEFUNCTIONS
#include <bdeat_customizedtypefunctions.h>
#endif

#ifndef INCLUDED_BDEAT_ENUMFUNCTIONS
#include <bdeat_enumfunctions.h>
#endif

#ifndef INCLUDED_BDEAT_FORMATTINGMODE
#include <bdeat_formattingmode.h>
#endif

#ifndef INCLUDED_BDEAT_SEQUENCEFUNCTIONS
#include <bdeat_sequencefunctions.h>
#endif

#ifndef INCLUDED_BDEAT_TYPECATEGORY
#include <bdeat_typecategory.h>
#endif

#ifndef INCLUDED_BDEAT_VALUETYPEFUNCTIONS
#include <bdeat_valuetypefunctions.h>
#endif

#ifndef INCLUDED_BDEU_PRINT
#include <bdeu_print.h>
#endif

#ifndef INCLUDED_BSLS_TYPES
#include <bsls_types.h>
#endif

#ifndef INCLUDED_BSL_SSTREAM
#include <bsl_sstream.h>
#endif

#ifndef INCLUDED_BSL_STREAMBUF
#include <bsl_streambuf.h>
#endif

#ifndef INCLUDED_BSL_STRING
#include <bsl_string.h>
#endif

#ifndef INCLUDED_BSL_VECTOR
#include <bsl_vector.h>
#endif

namespace BloombergLP {

                        // ====================
                        // class baejsn_Encoder
                        // ====================

class baejsn_Encoder {
    // This class provides a mechanism for encoding value-semantic objects in
    // the JSON format.  The 'encode' methods are function templates that will
    // encode any object that meets the requirements of a sequence, choice, or
    // array object as defined in the 'bdeat_sequencefunctions',
    // 'bdeat_choicefunctions', and 'bdeat_choicefunctions' components
    // respectively.  These generic frameworks provide a common compile-time
    // interface for accessing struct-like and union-like objects.  In
    // particular, 'bcem_aggregate' and the types generated by 'bas_codegen.pl'
    // provide the necessary interface and can be encoded using this component.

    // FRIENDS
    friend class baejsn_Encoder_EncodeImpl;

  private:
    // DATA
    bsl::ostringstream d_logStream;        // stream used for logging

  private:
    // PRIVATE MANIPULATORS
    bsl::ostream& logStream();
        // Return the stream for logging.

  public:
    // CREATORS
    explicit baejsn_Encoder(bslma::Allocator *basicAllocator = 0);
        // Create a encoder object.  Optionally specify a 'basicAllocator' used
        // to supply memory.  If 'basicAllocator' is 0, the currently installed
        // default allocator is used.

    //! ~baejsn_Encoder() = default;
        // Destroy this object.

    // MANIPULATORS
    template <typename TYPE>
    int encode(bsl::streambuf *streamBuf, const TYPE& value);
        // Encode the specified 'value' of (template parameter) 'TYPE' into the
        // specified 'streamBuf'.  Return 0 on success, and a non-zero value
        // otherwise.
        //
        // DEPRECATED: Use the 'encode' functions passed a reference to a
        // non-modifiable 'baejsn_EncoderOptions' object instead. 

    template <typename TYPE>
    int encode(bsl::ostream& stream, const TYPE& value);
        // Encode the specified 'value' of (template parameter) 'TYPE' into the
        // specified 'streamBuf'.  Return 0 on success, and a non-zero value
        // otherwise.  Note that 'stream' will be invalidated if the encoding
        // failed.
        //
        // DEPRECATED: Use the 'encode' functions passed a reference to a
        // non-modifiable 'baejsn_EncoderOptions' object instead. 

    template <typename TYPE>
    int encode(bsl::streambuf               *streamBuf,
               const TYPE&                   value,
               const baejsn_EncoderOptions&  options);
        // Encode the specified 'value' of (template parameter) 'TYPE' into the
        // specified 'streamBuf' using the specified 'options'.  Return 0 on
        // success, and a non-zero value otherwise.

    template <typename TYPE>
    int encode(bsl::ostream&                stream,
               const TYPE&                  value,
               const baejsn_EncoderOptions& options);
        // Encode the specified 'value' of (template parameter) 'TYPE' into the
        // specified 'streamBuf' using the specified 'options'.  Return 0 on
        // success, and a non-zero value otherwise.  Note that 'stream' will be
        // invalidated if the encoding failed.

    // ACCESSORS
    bsl::string loggedMessages() const;
        // Return a string containing any error, warning, or trace messages
        // that were logged during the last call to the 'encode' method.  The
        // log is reset each time 'encode' is called.
};

                        // ===============================
                        // class baejsn_Encoder_EncodeImpl
                        // ===============================

class baejsn_Encoder_EncodeImpl {
    // This class implements the parameterized 'encode' functions that encode
    // 'bdeat' types in the JSON format.  This is a component-private class and
    // should not be used outside of this component.

    // DATA
    baejsn_Encoder                       *d_encoder;          // encoder (held,
                                                              // not owned)

    bsl::ostream                          d_outputStream;     // stream for
                                                              // output

    baejsn_EncoderOptions::EncodingStyle  d_encodingStyle;    // encoding
                                                              // style

    int                                   d_indentLevel;      // initial indent
                                                              // level

    int                                   d_spacesPerLevel;   // spaces per
                                                              // level

    bool                                  d_isArrayElement;   // is current
                                                              // element part
                                                              // of an array

    bool                                  d_isUntaggedElement;// is current
                                                              // element
                                                              // untagged

    // FRIENDS
    friend struct baejsn_Encoder_DynamicTypeDispatcher;
    friend struct baejsn_Encoder_ElementVisitor;
    friend class baejsn_Encoder_SequenceVisitor;

  private:
    // PRIVATE MANIPULATORS
    bsl::ostream& logStream();
        // Return the stream used for logging.

    void indentTopElement();
        // Indent the top-level element writing the appropriate number of
        // spaces onto the output stream.

    int encodeImp(const bsl::vector<char>& value, bdeat_TypeCategory::Array);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::Array);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::Choice);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::CustomizedType);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::DynamicType);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::Enumeration);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::NullableValue);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::Sequence);
    template <typename TYPE>
    int encodeImp(const TYPE& value, bdeat_TypeCategory::Simple);
        // Encode the specified 'value' invoking the appropriate function based
        // on the corresponding 'bdeat' type of 'value'.  Return 0 on success,
        // and a non-zero value otherwise.

  public:
    // CREATORS
    baejsn_Encoder_EncodeImpl(baejsn_Encoder               *encoder,
                              bsl::streambuf               *streambuf,
                              const baejsn_EncoderOptions&  options);
        // Create a 'baejsn_Encoder_EncodeImpl' object using the specified
        // 'encoder' and 'options' and writing the encoded output to the
        // specified 'streamBuf'.

    // MANIPULATORS
    template <typename TYPE>
    int encode(const TYPE& value);
        // Encode the specified 'value' in the JSON format.  Return 0 on
        // success and a non-zero value otherwise.
};

                 // ====================================
                 // struct baejsn_Encoder_ElementVisitor
                 // ====================================

struct baejsn_Encoder_ElementVisitor {
    // This class encodes elements in an array or a choice in the JSON format.
    // This is a component-private class and should not be used outside of this
    // component.  Note that this 'class' provides the following operators:
    //..
    //  template <typename TYPE> int operator()(TYPE *value);
    //  template <typename TYPE, typename INFO>
    //  int operator()(TYPE *value, const INFO& info);
    //..


    // DATA
    baejsn_Encoder_EncodeImpl *d_encoder;  // encoder (held, not owned)

    // CREATORS
    // Creators have been omitted to allow simple static initialization of
    // this struct.

    // MANIPULATORS
    template <typename TYPE>
    int operator()(const TYPE& value);
        // Encode the specified 'value' in the JSON format.  Return 0 on
        // success and a non-zero value otherwise.

    template <typename TYPE, typename INFO>
    int operator()(const TYPE& value, const INFO &info);
        // Encode the specified 'value' using the specified 'info' in the JSON
        // format.  Return 0 on success and a non-zero value otherwise.
};

                 // ====================================
                 // class baejsn_Encoder_SequenceVisitor
                 // ====================================

class baejsn_Encoder_SequenceVisitor {
    // This functor class encodes element in a sequence.  It should be passed
    // as an argument to the 'bdeat_SequenceFunctions::accessAttributes'
    // function.  This is a component-private class and should not be used
    // outside of this component.

    // DATA
    baejsn_Encoder_EncodeImpl *d_encoder;          // encoder (held, not owned)
    bool                       d_firstElementFlag; // flag indicating if an
                                                   // current element is the
                                                   // first

  private:
    // PRIVATE CLASS METHODS
    template <class TYPE>
    static bool isAttributeNull(const TYPE&, bslmf::MetaInt<0>);
    template <class TYPE>
    static bool isAttributeNull(const TYPE& value, bslmf::MetaInt<1>);
    template <class TYPE>
    static bool isAttributeNull(const TYPE& value);
        // Return 'true' if the specified 'value' represents a
        // 'bdeat_NullableValue' type and is null, and 'false' otherwise.

  public:
    // CREATORS
    explicit baejsn_Encoder_SequenceVisitor(
                                           baejsn_Encoder_EncodeImpl *encoder);
        // Create a 'baejsn_Encoder_SequenceVisitor' object using the
        // specified 'encoder'.

    // MANIPULATORS
    template <typename TYPE, typename INFO>
    int operator()(const TYPE& value, const INFO &info);
        // Encode the specified 'value' using the specified 'info' in the JSON
        // format.  Return 0 on success and a non-zero value otherwise.
};

                 // ===========================================
                 // struct baejsn_Encoder_DynamicTypeDispatcher
                 // ===========================================

struct baejsn_Encoder_DynamicTypeDispatcher {
    // This class is used to dispatch the appropriate 'encodeImp' method for a
    // 'bdeat' Dynamic type.  This is a component-private class and should not
    // be used outside of this component.

    // DATA
    baejsn_Encoder_EncodeImpl *d_encoder;  // encoder (held, not owned)

    // CREATORS
    // Creators have been omitted to allow simple static initialization of
    // this struct.

    // MANIPULATORS
    template <typename TYPE>
    int operator()(const TYPE&, bslmf_Nil);
    template <typename TYPE, typename ANY_CATEGORY>
    int operator()(const TYPE& value, ANY_CATEGORY category);
        // Encode the specified 'value' of the specified 'bdeat' 'category' in
        // the JSON format.  Return 0 on success and a non-zero value
        // otherwise.
};

// ============================================================================
//                        INLINE FUNCTION DEFINITIONS
// ============================================================================

                            // --------------------
                            // class baejsn_Encoder
                            // --------------------

// PRIVATE MANIPULATORS
inline
bsl::ostream& baejsn_Encoder::logStream()
{
    return d_logStream;
}

// CREATORS
inline
baejsn_Encoder::baejsn_Encoder(bslma::Allocator *basicAllocator)
: d_logStream(basicAllocator)
{
}

// MANIPULATORS
template <typename TYPE>
int baejsn_Encoder::encode(bsl::streambuf *streamBuf, const TYPE& value)
{
    const baejsn_EncoderOptions options;
    return encode(streamBuf, value, options);
}

template <typename TYPE>
int baejsn_Encoder::encode(bsl::ostream& stream, const TYPE& value)
{
    const baejsn_EncoderOptions options;
    return encode(stream, value, options);
}

template <typename TYPE>
int baejsn_Encoder::encode(bsl::streambuf               *streamBuf,
                           const TYPE&                   value,
                           const baejsn_EncoderOptions&  options)
{
    BSLS_ASSERT(streamBuf);

    bdeat_TypeCategory::Value category =
                                    bdeat_TypeCategoryFunctions::select(value);
    if (bdeat_TypeCategory::BDEAT_SEQUENCE_CATEGORY != category
     && bdeat_TypeCategory::BDEAT_CHOICE_CATEGORY != category
     && bdeat_TypeCategory::BDEAT_ARRAY_CATEGORY != category) {
        logStream()
                  << "Encoded object must be a Sequence, Choice or Array type."
                  << bsl::endl;
        return -1;                                                    // RETURN
    }

    d_logStream.clear();
    d_logStream.str("");

    baejsn_Encoder_EncodeImpl encoderImpl(this, streamBuf, options);

    const int rc = encoderImpl.encode(value);

    if (!rc && baejsn_EncoderOptions::BAEJSN_PRETTY ==
                                                     options.encodingStyle()) {
        streamBuf->sputc('\n');
    }
    return rc;
}

template <typename TYPE>
int baejsn_Encoder::encode(bsl::ostream&                stream,
                           const TYPE&                  value,
                           const baejsn_EncoderOptions& options)
{
    if (!stream.good()) {
        logStream() << "Invalid stream." << bsl::endl;
        return -1;                                                    // RETURN
    }

    int rc = this->encode(stream.rdbuf(), value, options);
    if (0 != rc) {
        stream.setstate(bsl::ios_base::failbit);
        return rc;                                                    // RETURN
    }

    return 0;
}

// ACCESSORS
inline
bsl::string baejsn_Encoder::loggedMessages() const
{
    return d_logStream.str();
}

                        // -------------------------------
                        // class baejsn_Encoder_EncodeImpl
                        // -------------------------------

// PRIVATE MANIPULATORS
inline
bsl::ostream& baejsn_Encoder_EncodeImpl::logStream()
{
    return d_encoder->logStream();
}

template <typename TYPE>
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::Sequence)
{
    if (!d_isUntaggedElement) {
        if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
            if (!d_isArrayElement) {
                d_outputStream << ' ';
            }
            else {
                bdeu_Print::indent(d_outputStream,
                                   d_indentLevel,
                                   d_spacesPerLevel);
            }
        }

        d_outputStream << '{';

        if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
            d_outputStream << '\n';
        }

        ++d_indentLevel;
    }

    baejsn_Encoder_SequenceVisitor visitor(this);

    bool isArrayElement    = d_isArrayElement;
    bool isUntaggedElement = d_isUntaggedElement;

    d_isArrayElement    = false;
    d_isUntaggedElement = false;

    int rc = bdeat_SequenceFunctions::accessAttributes(value, visitor);
    if (0 != rc) {
        return rc;                                                    // RETURN
    }

    d_isArrayElement    = isArrayElement;
    d_isUntaggedElement = isUntaggedElement;

    if (!d_isUntaggedElement) {
        --d_indentLevel;

        if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
            d_outputStream << '\n';
            bdeu_Print::indent(d_outputStream,
                               d_indentLevel,
                               d_spacesPerLevel);
        }

        d_outputStream << '}';
    }

    return 0;
}

template <typename TYPE>
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::Choice)
{
    if (bdeat_ChoiceFunctions::BDEAT_UNDEFINED_SELECTION_ID !=
                                   bdeat_ChoiceFunctions::selectionId(value)) {
        if (!d_isUntaggedElement) {
            if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
                if (!d_isArrayElement) {
                    d_outputStream << ' ';
                }
                else {
                    bdeu_Print::indent(d_outputStream,
                                       d_indentLevel,
                                       d_spacesPerLevel);
                }
            }

            d_outputStream << '{';

            if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
                d_outputStream << '\n';
            }

            ++d_indentLevel;
        }

        baejsn_Encoder_ElementVisitor visitor = { this };

        bool isArrayElement    = d_isArrayElement;
        bool isUntaggedElement = d_isUntaggedElement;

        d_isArrayElement = false;
        d_isUntaggedElement = false;

        if (0 != bdeat_ChoiceFunctions::accessSelection(value, visitor)) {
            return -1;                                                // RETURN
        }

        d_isArrayElement    = isArrayElement;
        d_isUntaggedElement = isUntaggedElement;

        if (!d_isUntaggedElement) {
            --d_indentLevel;

            if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
                d_outputStream << '\n';
                bdeu_Print::indent(d_outputStream,
                                   d_indentLevel,
                                   d_spacesPerLevel);
            }

            d_outputStream << '}';
        }
    }
    else {
        logStream() << "Undefined selection for Choice object" << bsl::endl;
        return -1;                                                    // RETURN
    }
    return 0;
}

template <typename TYPE>
inline
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::Enumeration)
{
    bsl::string valueString;
    bdeat_EnumFunctions::toString(&valueString, value);
    return encode(valueString);
}

template <typename TYPE>
inline
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::CustomizedType)
{
    return encode(bdeat_CustomizedTypeFunctions::convertToBaseType(value));
}

template <typename TYPE>
inline
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::DynamicType)
{
    baejsn_Encoder_DynamicTypeDispatcher proxy = { this };
    return bdeat_TypeCategoryUtil::accessByCategory(value, proxy);
}


template <typename TYPE>
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::Simple)
{
    if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
        if (d_isArrayElement) {
            bdeu_Print::indent(d_outputStream,
                               d_indentLevel,
                               d_spacesPerLevel);
        } else {
            d_outputStream << ' ';
        }
    }

    return baejsn_PrintUtil::printValue(d_outputStream, value);
}

template <typename TYPE>
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::Array)
{
    if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
        d_outputStream << ' ';
    }

    d_outputStream << '[';

    int size = static_cast<int>(bdeat_ArrayFunctions::size(value));

    if (0 < size) {
        if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
            d_outputStream << '\n';
        }

        ++d_indentLevel;

        baejsn_Encoder_ElementVisitor visitor = { this };

        d_isArrayElement = true;

        int rc = bdeat_ArrayFunctions::accessElement(value, visitor, 0);
        if (0 != rc) {
            return rc;                                                // RETURN
        }

        for (int i = 1; i < size; ++i) {
            d_outputStream << ',';
            if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
                d_outputStream << '\n';
            }
            rc = bdeat_ArrayFunctions::accessElement(value, visitor, i);
            if (0 != rc) {
                return rc;                                            // RETURN
            }
        }

        d_isArrayElement = false;

        --d_indentLevel;
    }

    if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
        d_outputStream << '\n';
        bdeu_Print::indent(d_outputStream, d_indentLevel, d_spacesPerLevel);
    }

    d_outputStream << ']';

    return 0;
}

template <typename TYPE>
inline
int baejsn_Encoder_EncodeImpl::encodeImp(const TYPE& value,
                                         bdeat_TypeCategory::NullableValue)
{
    if (bdeat_NullableValueFunctions::isNull(value)) {
        if (baejsn_EncoderOptions::BAEJSN_PRETTY == d_encodingStyle) {
            if (d_isArrayElement) {
                bdeu_Print::indent(d_outputStream,
                                   d_indentLevel,
                                   d_spacesPerLevel);
            } else {
                d_outputStream << ' ';
            }
        }

        d_outputStream << "null";
        return 0;                                                     // RETURN
    }

    baejsn_Encoder_ElementVisitor visitor = { this };
    return bdeat_NullableValueFunctions::accessValue(value, visitor);
}

template <typename TYPE>
inline
int baejsn_Encoder_EncodeImpl::encode(const TYPE& value)
{
    typedef typename bdeat_TypeCategory::Select<TYPE>::Type TypeCategory;
    return encodeImp(value, TypeCategory());
}

// CREATORS
inline
baejsn_Encoder_EncodeImpl::baejsn_Encoder_EncodeImpl(
                                       baejsn_Encoder               *encoder,
                                       bsl::streambuf               *streambuf,
                                       const baejsn_EncoderOptions&  options)
: d_encoder(encoder)
, d_outputStream(streambuf)
, d_isArrayElement(false)
, d_isUntaggedElement(false)
{
    if (baejsn_EncoderOptions::BAEJSN_PRETTY == options.encodingStyle()) {
        d_encodingStyle  = options.encodingStyle();
        d_indentLevel    = options.initialIndentLevel();
        d_spacesPerLevel = options.spacesPerLevel();
    }
    else {
        d_encodingStyle  = baejsn_EncoderOptions::BAEJSN_COMPACT;
        d_indentLevel    = 0;
        d_spacesPerLevel = 0;
    }

    indentTopElement();
}

                    // -------------------------------------
                    // struct baejsn_Encoder_SequenceVisitor
                    // -------------------------------------

// PRIVATE CLASS METHODS
template <class TYPE>
inline
bool baejsn_Encoder_SequenceVisitor::isAttributeNull(const TYPE&,
                                                     bslmf::MetaInt<0>)
{
    return false;
}

template <class TYPE>
inline
bool baejsn_Encoder_SequenceVisitor::isAttributeNull(const TYPE& value,
                                                     bslmf::MetaInt<1>)
{
    if (bdeat_TypeCategory::BDEAT_NULLABLE_VALUE_CATEGORY ==
                              bdeat_TypeCategoryFunctions::select(value)) {
        return bdeat_NullableValueFunctions::isNull(value);           // RETURN
    }
    return false;
}

template <class TYPE>
inline
bool baejsn_Encoder_SequenceVisitor::isAttributeNull(const TYPE& value)
{
    return isAttributeNull(value,
                           bslmf::MetaInt<bdeat_NullableValueFunctions
                                            ::IsNullableValue<TYPE>::VALUE>());
}

// CREATORS
inline
baejsn_Encoder_SequenceVisitor::baejsn_Encoder_SequenceVisitor(
                                            baejsn_Encoder_EncodeImpl *encoder)
: d_encoder(encoder)
, d_firstElementFlag(true)
{
}

// MANIPULATORS
template <typename TYPE, typename INFO>
inline
int baejsn_Encoder_SequenceVisitor::operator()(const TYPE& value,
                                               const INFO& info)
{
    // Determine if 'value' is null and do not encode 'value' if it is.

    if (isAttributeNull(value)) {
        return 0;                                                     // RETURN
    }

    if (!d_firstElementFlag) {
        d_encoder->d_outputStream << ',';
        if (baejsn_EncoderOptions::BAEJSN_PRETTY ==
                                                  d_encoder->d_encodingStyle) {
            d_encoder->d_outputStream << '\n';
        }
    }

    d_firstElementFlag = false;

    baejsn_Encoder_ElementVisitor visitor = { d_encoder };
    const int rc = visitor(value, info);
    return rc;
}

                    // ------------------------------------
                    // struct baejsn_Encoder_ElementVisitor
                    // ------------------------------------

template <typename TYPE>
inline
int baejsn_Encoder_ElementVisitor::operator()(const TYPE &value)
{
    return d_encoder->encode(value);
}

template <typename TYPE, typename INFO>
inline
int baejsn_Encoder_ElementVisitor::operator()(const TYPE& value,
                                              const INFO& info)
{
    // Skip encoding of anonymous elements

    if (!(info.formattingMode() & bdeat_FormattingMode::BDEAT_UNTAGGED)) {

        if (baejsn_EncoderOptions::BAEJSN_PRETTY ==
                                                  d_encoder->d_encodingStyle) {
            bdeu_Print::indent(d_encoder->d_outputStream,
                               d_encoder->d_indentLevel,
                               d_encoder->d_spacesPerLevel);
        }

        int rc = baejsn_PrintUtil::printValue(d_encoder->d_outputStream,
                                              info.name());
        if (0 != rc) {
            d_encoder->logStream()
                << "Unable to encode the name of the element, '"
                << info.name()
                << "'."
                << bsl::endl;
            return rc;                                                // RETURN
        }

        if (baejsn_EncoderOptions::BAEJSN_PRETTY ==
                                                  d_encoder->d_encodingStyle) {
            d_encoder->d_outputStream << ' ';
        }

        d_encoder->d_outputStream << ':';
        d_encoder->d_isUntaggedElement = false;
    }
    else {
        d_encoder->d_isUntaggedElement = true;
    }

    int rc = d_encoder->encode(value);

    if (0 != rc) {
        d_encoder->logStream()
            << "Unable to encode the value of the element, '"
            << info.name()
            << "'."
            << bsl::endl;
        return rc;                                                    // RETURN
    }
    return 0;
}

                    // -------------------------------------------
                    // struct baejsn_Encoder_DynamicTypeDispatcher
                    // -------------------------------------------

// MANIPULATORS
template <typename TYPE>
inline
int baejsn_Encoder_DynamicTypeDispatcher::operator()(const TYPE&, bslmf_Nil)
{
    BSLS_ASSERT_OPT(!"Should be unreachable!");

    return -1;
}

template <typename TYPE, typename ANY_CATEGORY>
inline
int baejsn_Encoder_DynamicTypeDispatcher::operator()(const TYPE&  value,
                                                     ANY_CATEGORY category)
{
    d_encoder->encodeImp(value, category);
    return 0;
}


}  // close namespace BloombergLP

#endif

// ----------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2012
//      All Rights Reserved.
//      Property of Bloomberg L.P.  (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ----------------------------------
