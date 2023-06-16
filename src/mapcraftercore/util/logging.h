/*
 * Copyright 2012-2016 Moritz Hilscher
 *
 * This file is part of Mapcrafter.
 *
 * Mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "../compat/thread.h"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DEFAULT_LOGGER "default"
#define DEFAULT_LOGKEY "file__" __FILE__ ":" TOSTRING(__LINE__)

#define LOGN(level, logger)                                                                        \
    mapcrafter::util::Logging::getInstance().getLogger((logger)).log(                              \
        mapcrafter::util::LogLevel::level, __FILE__, __LINE__                                      \
    )
#define LOG(level) LOGN(level, DEFAULT_LOGGER)

#define LOGNK_ONCE(level, logger, key)                                                             \
    mapcrafter::util::Logging::getInstance().getLogger((logger)).logOnce(                          \
        (key), mapcrafter::util::LogLevel::level, __FILE__, __LINE__                               \
    )
#define LOGN_ONCE(level, logger) LOGNK_ONCE(level, logger, DEFAULT_LOGKEY)
#define LOGK_ONCE(level, key) LOGNK_ONCE(level, DEFAULT_LOGGER, std::string("key__") + (key))
#define LOG_ONCE(level) LOGN_ONCE(level, DEFAULT_LOGGER)

namespace mapcrafter {
namespace util {

class Logger;
class Logging;

/**
 * Log levels according to syslog.
 */
enum class LogLevel {
    // System is unusable
    EMERGENCY = 0,
    // Action must be taken immediately
    ALERT = 1,
    // Critical conditions
    FATAL = 2, // or "critical"
    // Error conditions
    ERROR = 3,
    // Warning conditions
    WARNING = 4,
    // Normal but significant condition
    NOTICE = 5,
    // Informational messages
    INFO = 6,
    // Debug-level messages
    DEBUG = 7,
    // Unknown level, only used for levelFromString method
    UNKNOWN = 8,
};

#ifndef HAVE_ENUM_CLASS_COMPARISON

// Implement enum class comparison methods manually if not supported by compiler (gcc 4.4)

bool operator==(LogLevel level1, LogLevel level2);
bool operator!=(LogLevel level1, LogLevel level2);

bool operator<(LogLevel level1, LogLevel level2);
bool operator<=(LogLevel level1, LogLevel level2);

bool operator>(LogLevel level1, LogLevel level2);
bool operator>=(LogLevel level1, LogLevel level2);

#endif

/**
 * Helper to convert the log level enum types from/to string.
 */
class LogLevelHelper {
  public:
    /**
     * std::string to LogLevel.
     */
    static LogLevel levelFromString(const std::string &str);

    /**
     * LogLevel to std::string
     */
    static std::string levelToString(LogLevel level);

#ifdef HAVE_SYSLOG_H

    /**
     * LogLevel to syslog level.
     */
    static int levelToSyslog(LogLevel level);

#endif
};

std::ostream &operator<<(std::ostream &out, LogLevel level);

/**
 * Represents a single log message.
 */
struct LogMessage {
    // log level of this message
    LogLevel level;
    // the logger that emitted the message
    std::string logger;
    // source code filename/line where this was logged
    std::string file;
    int line;

    // actual logged message
    std::string message;
};

/**
 * This is a small helper to log messages with the << operator.
 *
 * It implements the operator<< to write the message parts into an internal string stream.
 * The content of the string stream (the log message) is sent to the Logging object when
 * the LogStream's destructor is called.
 *
 * You can use the setFake-method to make the log stream not log the message, this is
 * used to log messages only once.
 */
class LogStream {
  public:
    LogStream(LogLevel level, const std::string &logger, const std::string &file, int line);
    ~LogStream();

    void setFake(bool fake);

    template <typename T> LogStream &operator<<(const T &t) {
        (*ss) << t;
        return *this;
    }

  private:
    bool fake;
    LogMessage message;

    std::shared_ptr<std::stringstream> ss;
};

/**
 * This class represents a logger.
 *
 * You can use the log method to log messages.
 *
 * The constructor is protected, the instances of the logger objects are managed by the
 * Logging class.
 */
class Logger {
  public:
    ~Logger();

    /**
     * Returns a LogStream to log a message, you have to specify a log level for the
     * message and a file and line where this was logged.
     *
     * You should not call this method directory, use the LOG and LOGN macros instead.
     */
    LogStream log(LogLevel level, const std::string &file, int line);

    /**
     * Same as log, but returns a fake log stream if there was already something logged
     * with the specified key.
     */
    LogStream logOnce(const std::string &key, LogLevel level, const std::string &file, int line);

  protected:
    Logger(const std::string &name);

    // name of this logger
    std::string name;

    friend class Logging;
};

/**
 * This abstract represents a sink for log messages.
 *
 * You should implement the sink method to handle log messages.
 */
class LogSink {
  public:
    LogSink();
    virtual ~LogSink();

    /**
     * This abstract method is called for every message that is logged.
     *
     * You MAY NOT use the LOG(level) functionality in here, otherwise the program
     * will end up in a deadlock.
     */
    virtual void sink(const LogMessage &message);
};

/**
 * This is a log sink that automatically formats log messages with a specific format.
 */
class FormattedLogSink : public LogSink {
  public:
    FormattedLogSink();
    virtual ~FormattedLogSink();

    /**
     * Sets the log message format.
     */
    void setFormat(const std::string &format);

    /**
     * Sets the date format for the message formatting.
     */
    void setDateFormat(const std::string &date_format);

    /**
     * This method formats the received log messages and calls the sinkFormatted
     * method which you should implement.
     */
    virtual void sink(const LogMessage &message);

    /**
     * This abstract method is called for every formatted log message.
     */
    virtual void sinkFormatted(const LogMessage &message, const std::string &formatted);

  protected:
    std::string format, date_format;

    /**
     * Formats a log message with the set message/date format.
     */
    std::string formatLogEntry(const LogMessage &message);
};

/**
 * This sink logs all message to stdout/stderr (depending on log level).
 */
class LogOutputSink : public FormattedLogSink {
  public:
    LogOutputSink();
    virtual ~LogOutputSink();

    virtual void sinkFormatted(const LogMessage &message, const std::string &formatted);
};

/**
 * This sink logs all messages to a log file.
 */
class LogFileSink : public FormattedLogSink {
  public:
    LogFileSink(const std::string &filename);
    virtual ~LogFileSink();

    virtual void sinkFormatted(const LogMessage &message, const std::string &formatted);

  private:
    std::ofstream out;
};

#ifdef HAVE_SYSLOG_H

/**
 * This sink logs all message to the local syslog daemon.
 */
class LogSyslogSink : public LogSink {
  public:
    LogSyslogSink();
    virtual ~LogSyslogSink();

    virtual void sink(const LogMessage &message);
};

#endif

/**
 * Global logging facility. Manages the log sinks and allows the configuration of them.
 */
class Logging {
  public:
    ~Logging();

    /**
     * Returns/sets the default verbosity which is also used as default verbosity for the
     * log sinks. The default verbosity defaults to INFO.
     */
    LogLevel getDefaultVerbosity() const;
    void setDefaultVerbosity(LogLevel level);

    /**
     * Returns/sets the verbosity of a sink, i.e. the minimum log level log messages must
     * have to be handled by this sink. Defaults to the default verbosity.
     */
    LogLevel getSinkVerbosity(const std::string &sink) const;
    void setSinkVerbosity(const std::string &sink, LogLevel level);

    /**
     * Returns/sets whether a sink handles progress log messages. Defaults to true.
     */
    bool getSinkLogProgress(const std::string &sink) const;
    void setSinkLogProgress(const std::string &sink, bool log_progress);

    /**
     * Returns/sets a sink instance. Returns a nullptr if there is no sink with the
     * specific name.
     */
    LogSink *getSink(const std::string &name);
    void setSink(const std::string &name, LogSink *sink);

    /**
     * Resets the configured logging facility.
     * Resets the default verbosity to INFO, deletes the log sinks and creates a new
     * default output log sink.
     */
    void reset();

    /**
     * Returns the instance of a specific logger (thread-safe).
     */
    Logger &getLogger(const std::string &name);

    /**
     * Returns the singleton instance of the logging facility (thread-safe).
     */
    static Logging &getInstance();

  protected:
    Logging();

    /**
     * Updates the maximum verbosity that is used as verbosity for a log sink.
     * That way we do not even need to consider to handle a message if there is no
     * log sink with such a verbosity.
     */
    void updateMaximumVerbosity();

    /**
     * Handles a log message and passes it to all log sinks with the required verbosity.
     */
    void handleLogMessage(const LogMessage &message);

    LogLevel default_verbosity, maximum_verbosity;
    std::map<std::string, std::shared_ptr<Logger>> loggers;
    std::map<std::string, std::shared_ptr<LogSink>> sinks;
    std::map<std::string, LogLevel> sinks_verbosity;
    std::map<std::string, bool> sinks_log_progress;

    thread_ns::mutex loggers_mutex, handle_message_mutex;

    static thread_ns::mutex instance_mutex;
    static std::shared_ptr<Logging> instance;

    friend class LogStream;
};

} /* namespace util */
} /* namespace mapcrafter */

#endif /* LOGGING_H_ */
