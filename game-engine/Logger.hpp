#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>

#define LOG_INFO(comp, evt, ...)  Logger::instance().log(LogLevel::INFO,  comp, evt, ##__VA_ARGS__)
#define LOG_WARN(comp, evt, ...)  Logger::instance().log(LogLevel::WARN,  comp, evt, ##__VA_ARGS__)
#define LOG_ERROR(comp, evt, ...) Logger::instance().log(LogLevel::ERR, comp, evt, ##__VA_ARGS__)
#define LOG_DEBUG(comp, evt, ...) Logger::instance().log(LogLevel::DEBUG, comp, evt, ##__VA_ARGS__)

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERR
};

class Logger {
private:
    std::ofstream file_;
    std::mutex    mutex_;

    explicit Logger(const std::string& path) : file_(path, std::ios::app)
    {
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + path);
        }
    }

    static const char* levelStr(LogLevel l) {
        switch(l) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERR: return "ERR";
        }
        return "INFO";
    }
public:
    static Logger& instance() {
        auto now = std::chrono::system_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()).count();

        std::filesystem::create_directories("logs");
        std::string filename = "logs/game_" + std::to_string(ms) + ".log";

        static Logger inst(filename);
        return inst;
    }

    void log(LogLevel level, const std::string& component,
             const std::string& event, const std::string& details = "")
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch()).count();

        std::ostringstream oss;
        oss << "{\"ts\":" << ms
            << ",\"level\":\"" << levelStr(level) << "\""
            << ",\"component\":\"" << component << "\""
            << ",\"event\":\"" << event << "\"";

        if (!details.empty())
            oss << ",\"details\":\"" << details << "\"";

        oss << "}\n";

        const std::string line = oss.str();
        file_  << line;
        file_.flush();
        std::cout << line;
    }
};

#endif
