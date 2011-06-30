// bael_fileobserver2.cpp                                             -*-C++-*-
#include <bael_fileobserver2.h>

#include <bdes_ident.h>
BDES_IDENT_RCSID(bael_fileobserver2_cpp,"$Id$ $CSID$")

#include <bael_context.h>
#include <bael_record.h>
#include <bael_recordattributes.h>

#ifdef BDE_FOR_TESTING_ONLY
#include <bael_defaultobserver.h>             // for testing only
#include <bael_log.h>                         // for testing only
#include <bael_loggermanager.h>               // for testing only
#include <bael_loggermanagerconfiguration.h>  // for testing only
#include <bael_multiplexobserver.h>           // for testing only
#include <bael_recordstringformatter.h>       // for testing only
#endif

#include <bcemt_lockguard.h>

#include <bdem_list.h>

#include <bdetu_datetime.h>
#include <bdetu_systemtime.h>

#include <bdef_memfn.h>

#include <bdesu_fileutil.h>

#include <bsls_assert.h>
#include <bsls_platform.h>

#include <bsl_cmath.h>
#include <bsl_cstdio.h>
#include <bsl_cstring.h>
#include <bsl_iomanip.h>
#include <bsl_iosfwd.h>    // for 'bsl::streamoff' type
#include <bsl_iostream.h>
#include <bsl_sstream.h>

#include <bsl_c_errno.h>
#include <bsl_c_time.h>

#include <bsl_c_stdio.h>   // for 'snprintf'

#ifdef BSLS_PLATFORM__OS_UNIX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef BSLS_PLATFORM__OS_WINDOWS
#include <windows.h>
#endif

namespace {

using namespace BloombergLP;

int getErrorCode(void)
    // Return the system-specific error code.
{
#ifdef BSLS_PLATFORM__OS_WINDOWS
    int rc = GetLastError();
    return rc ? rc : errno;
#else
    return errno;
#endif
}

bsl::string getTimestampSuffix(const bdet_Datetime& timestamp)
    // Return the specified 'timestamp' in the '.YYYYMMDD_hhmmss' format.
{
    char buffer[20];

#if defined(BSLS_PLATFORM__CMP_MSVC)
#define snprintf _snprintf
#endif

    snprintf(buffer,
             sizeof buffer,
             ".%4d%02d%02d_%02d%02d%02d",
             timestamp.year(),
             timestamp.month(),
             timestamp.day(),
             timestamp.hour(),
             timestamp.minute(),
             timestamp.second());

#if defined(BSLS_PLATFORM__CMP_MSVC)
#undef snprintf
#endif

    return bsl::string(buffer);
}

bool compareLogFileTimes(const bsl::pair<bsl::string, time_t>& lhs,
                         const bsl::pair<bsl::string, time_t>& rhs)
    // Return 'true' if the datetime field of the specified 'lhs' is later
    // than that of the specified 'rhs', and 'false' otherwise.
{
    return lhs.second > rhs.second;
}

void getLogFileName(bsl::string                  *logFileName,
                    bdet_Datetime                *timestamp,
                    bool                         *datetimeInfoInFileName,
                    const char                   *logFilePattern,
                    const bdet_DatetimeInterval&  localTimeOffset,
                    bool                          appendTimestampFlag)
    // Load, into the specified 'logFileName', the filename that is obtained by
    // replacing every '%'-escape sequence in the specified 'logFilePattern'
    // with the corresponding field of the local time calculated from the
    // current time in GMT and the specified 'localTimeOffset', and, if
    // 'logFilePattern' does not contain any '%'-escape sequence and the
    // specified 'appendTimestampFlag' is true, append a timestamp in the
    // format ".%Y%M%D_%h%m%s" to the filename.  Load the current time in GMT
    // into the specified 'timestamp'.  Load 'true' into the specified
    // 'datetimeInfoInFileName' if 'logFilePattern' contains at least one
    // '%'-escape sequence, and 'false' otherwise.
{
    BSLS_ASSERT(logFileName);
    BSLS_ASSERT(timestamp);
    BSLS_ASSERT(datetimeInfoInFileName);
    BSLS_ASSERT(logFilePattern);

    *timestamp = bdetu_SystemTime::nowAsDatetimeGMT();

    const bdet_Datetime logFileTimestamp = *timestamp + localTimeOffset;

    bsl::ostringstream os;
    *datetimeInfoInFileName = false;

    for (; *logFilePattern; ++logFilePattern) {
        if ('%' == *logFilePattern) {
            if (*++logFilePattern) {
                switch (*logFilePattern) {
                  case 'Y': {
                    os << bsl::setw(4) << bsl::setfill('0')
                       << logFileTimestamp.year();
                    *datetimeInfoInFileName = true;
                  } break;
                  case 'M': {
                    os << bsl::setw(2) << bsl::setfill('0')
                       << logFileTimestamp.month();
                    *datetimeInfoInFileName = true;
                  } break;
                  case 'D': {
                    os << bsl::setw(2) << bsl::setfill('0')
                       << logFileTimestamp.day();
                    *datetimeInfoInFileName = true;
                  } break;
                  case 'h': {
                    os << bsl::setw(2) << bsl::setfill('0')
                       << logFileTimestamp.hour();
                    *datetimeInfoInFileName = true;
                  } break;
                  case 'm': {
                    os << bsl::setw(2) << bsl::setfill('0')
                       << logFileTimestamp.minute();
                    *datetimeInfoInFileName = true;
                  } break;
                  case 's': {
                    os << bsl::setw(2) << bsl::setfill('0')
                       << logFileTimestamp.second();
                    *datetimeInfoInFileName = true;
                  } break;
                  case '%': {
                    *datetimeInfoInFileName = true;
                  } break;
                  default: {
                    os << '%' << *logFilePattern;
                  } break;
                }
            }
            else {
                os << '%';  // trailing '%' in pattern
                break;
            }
        } else {
            os << *logFilePattern;
        }
    }
    *logFileName = os.str();

    if (appendTimestampFlag && !*datetimeInfoInFileName) {
        *logFileName += getTimestampSuffix(logFileTimestamp);
    }
}

int openLogFile(bsl::ofstream *stream, const char *filename)
    // Open a file stream referred to by the specified 'stream' for the file
    // with the specified 'filename' in append mode.  Return 0 on success, and
    // a non-zero value otherwise.
{
    stream->open(filename, bsl::ios::out|bsl::ios::app);
    if (!stream->is_open()) {
        fprintf(
              stderr,
              "Cannot open log file %s: %s.  File logging will be disabled!\n",
              filename, bsl::strerror(getErrorCode()));
        return -1;                                                    // RETURN
    }
    stream->clear();
    return 0;
}

}  // close unnamed namespace

namespace BloombergLP {

                          // ------------------------
                          // class bael_FileObserver2
                          // ------------------------

// PRIVATE MANIPULATORS
void bael_FileObserver2::logRecordDefault(bsl::ostream&      stream,
                                          const bael_Record& record)

{
    const bael_RecordAttributes& fixedFields = record.fixedFields();

    bdet_Datetime timestamp = fixedFields.timestamp();
    if (d_publishInLocalTime) {
        timestamp += d_localTimeOffset;
    }

    char buffer[256];
    char *ptr = buffer;

    *ptr = '\n';
    ++ptr;

    int length = timestamp.printToBuffer(ptr, sizeof(buffer) - 1);
    ptr += length;

#if defined(BSLS_PLATFORM__CMP_MSVC)
#define snprintf _snprintf
#endif

    snprintf(ptr,
             sizeof(buffer) - length - 1,
             " %d:%llu %s %s:%d ",
             fixedFields.processID(),
             fixedFields.threadID(),
             bael_Severity::toAscii(
                                 (bael_Severity::Level)fixedFields.severity()),
             fixedFields.fileName(),
             fixedFields.lineNumber());

#if defined(BSLS_PLATFORM__CMP_MSVC)
#undef snprintf
#endif

    stream << buffer;
    stream << fixedFields.category();
    stream << ' ';
    stream << fixedFields.message();
    stream << ' ';

    const bdem_List& userFields = record.userFields();
    const int numUserFields = userFields.length();
    for (int i = 0; i < numUserFields; ++i) {
        stream << userFields[i] << ' ';
    }

    stream << '\n';

    stream.flush();
}

void bael_FileObserver2::rotateFile()
{
    if (!d_logFileStream.rdbuf()->is_open()) {
        return;                                                       // RETURN
    }

    BSLS_ASSERT(d_logFilePattern.size() > 0);

    d_logFileStream.close();

    if (!d_isOpenWithTimestampFlag && !d_datetimeInfoInFileName) {

        bsl::string newFileName(d_logFileName);
        newFileName +=
                    getTimestampSuffix(d_logFileTimestamp + d_localTimeOffset);

        if (0 != bsl::rename(d_logFileName.c_str(), newFileName.c_str())) {
            fprintf(stderr, "Cannot rename %s to %s: %s\n",
                    d_logFileName.c_str(), newFileName.c_str(),
                    bsl::strerror(getErrorCode()));
        }
    }

    getLogFileName(&d_logFileName,
                   &d_logFileTimestamp,
                   &d_datetimeInfoInFileName,
                   d_logFilePattern.c_str(),
                   d_localTimeOffset,
                   d_isOpenWithTimestampFlag);

    openLogFile(&d_logFileStream, d_logFileName.c_str());
}

void bael_FileObserver2::rotateIfNecessary(const bdet_Datetime& currentLogTime)
{
    BSLS_ASSERT(d_rotationSize >= 0);
    BSLS_ASSERT(d_rotationLifetime.totalSeconds() >= 0);

    if (!d_logFileStream.rdbuf()->is_open()) {
        return;                                                       // RETURN
    }

    if (d_rotationSize) {
        // 'tellp' returns -1 on failure.  Rotate the log file if either
        // 'tellp' fails, or the rotation size is exceeded.

        if (static_cast<bsls_Types::Uint64>(d_logFileStream.tellp()) >
            static_cast<bsls_Types::Uint64>(d_rotationSize) * 1024) {

            rotateFile();
            return;                                                   // RETURN
        }
    }

    if (d_rotationLifetime.totalSeconds()
              && (currentLogTime - d_logFileTimestamp) >= d_rotationLifetime) {
        rotateFile();
        return;                                                       // RETURN
    }
}

// CREATORS
bael_FileObserver2::bael_FileObserver2(bslma_Allocator *basicAllocator)
: d_logFilePattern(basicAllocator)
, d_logFileName(basicAllocator)
, d_datetimeInfoInFileName(false)
, d_isOpenWithTimestampFlag(false)
, d_logFileFunctor(
            bdef_MemFnUtil::memFn(&bael_FileObserver2::logRecordDefault, this),
            basicAllocator)
, d_localTimeOffset(bdetu_SystemTime::localTimeOffset())
, d_publishInLocalTime(false)
, d_rotationSize(0)
, d_rotationLifetime(0)
{
}

bael_FileObserver2::~bael_FileObserver2()
{
    if (d_logFileStream.is_open()) {
        d_logFileStream.close();
    }
}

// MANIPULATORS
void bael_FileObserver2::disableFileLogging()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    if (d_logFileStream.is_open()) {
        d_logFileStream.close();
    }
}

void bael_FileObserver2::disableLifetimeRotation()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_rotationLifetime.setTotalSeconds(0);
}

void bael_FileObserver2::disableSizeRotation()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_rotationSize = 0;
}

void bael_FileObserver2::disablePublishInLocalTime()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_publishInLocalTime = false;
}

int bael_FileObserver2::enableFileLogging(const char *fileNamePattern,
                                          bool        apppendTimestampFlag)
{
    BSLS_ASSERT(fileNamePattern);

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    if (d_logFileStream.is_open()) {
        return 1;                                                     // RETURN
    }
    d_logFilePattern = fileNamePattern;
    d_isOpenWithTimestampFlag = apppendTimestampFlag;

    getLogFileName(&d_logFileName,
                   &d_logFileTimestamp,
                   &d_datetimeInfoInFileName,
                   d_logFilePattern.c_str(),
                   d_localTimeOffset,
                   d_isOpenWithTimestampFlag);

    return openLogFile(&d_logFileStream, d_logFileName.c_str());
}

void bael_FileObserver2::forceRotation()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    rotateFile();
}

void bael_FileObserver2::enablePublishInLocalTime()
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_publishInLocalTime = true;
}

void bael_FileObserver2::publish(const bael_Record&  record,
                                 const bael_Context& )
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    rotateIfNecessary(record.fixedFields().timestamp());

    if (d_logFileStream.is_open()) {
        d_logFileFunctor(d_logFileStream, record);

        if (!d_logFileStream) {
            fprintf(stderr, "Error on file stream for %s: %s\n",
                    d_logFileName.c_str(), bsl::strerror(getErrorCode()));

            d_logFileStream.close();
        }
    }
}

void bael_FileObserver2::rotateOnLifetime(
                                     const bdet_DatetimeInterval& timeInterval)
{
    const int totalSeconds = timeInterval.totalSeconds();
    BSLS_ASSERT(totalSeconds > 0);

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_rotationLifetime.setTotalSeconds(totalSeconds);
}

void bael_FileObserver2::rotateOnSize(int size)
{
    BSLS_ASSERT(size > 0);

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_rotationSize = size;
}

void bael_FileObserver2::setLogFileFunctor(
                                        const LogRecordFunctor& logFileFunctor)
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    d_logFileFunctor = logFileFunctor;
}

// ACCESSORS
bool bael_FileObserver2::isFileLoggingEnabled() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    // The standard says that 'fstream::is_open()' is a non-'const' method.
    // However, 'streambuf::is_open()' is 'const'.

    return d_logFileStream.rdbuf()->is_open();
}

bool bael_FileObserver2::isFileLoggingEnabled(bsl::string *result) const
{
    BSLS_ASSERT(result);

    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);

    // The standard says that 'fstream::is_open()' is a non-'const' method.
    // However, 'streambuf::is_open()' is 'const'.

    bool rc = d_logFileStream.rdbuf()->is_open();
    if (rc) {
        result->assign(d_logFileName);
    }

    return rc;
}

bool bael_FileObserver2::isPublishInLocalTimeEnabled() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    return d_publishInLocalTime;
}

bdet_DatetimeInterval bael_FileObserver2::rotationLifetime() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    return d_rotationLifetime;
}

int bael_FileObserver2::rotationSize() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    return d_rotationSize;
}

bdet_DatetimeInterval bael_FileObserver2::localTimeOffset() const
{
    bcemt_LockGuard<bcemt_Mutex> guard(&d_mutex);
    return d_localTimeOffset;
}

}  // close namespace BloombergLP

// ---------------------------------------------------------------------------
// NOTICE:
//      Copyright (C) Bloomberg L.P., 2007
//      All Rights Reserved.
//      Property of Bloomberg L.P. (BLP)
//      This software is made available solely pursuant to the
//      terms of a BLP license agreement which governs its use.
// ----------------------------- END-OF-FILE ---------------------------------
