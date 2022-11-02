#pragma once
#include <sstream>

namespace sge {
class Logger {
 public:
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    [[nodiscard]] std::stringstream& log() noexcept;
    void flush_error() noexcept;
    void flush_info() noexcept;
    void flush() const noexcept;
    static Logger& Instance() noexcept;

 private:
    Logger();
    ~Logger();
    std::stringstream m_ss;
};
#define LOG_ERROR(...) \
    { \
        sge::Logger::Instance().log() << "In file: " << __FILE__ << ", in line " << __LINE__ << ": " << __VA_ARGS__; \
        sge::Logger::Instance().flush_error(); \
    }
#define LOG_MSG(...) \
    { \
        sge::Logger::Instance().log() << __VA_ARGS__; \
        sge::Logger::Instance().flush_info(); \
    }
#define LOG_MSG_FLUSH \
    { sge::Logger::Instance().flush(); }
}  // namespace sge
