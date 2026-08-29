#pragma once

#include <memory>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace Cube {
    class Log {
    public:
        static std::shared_ptr<spdlog::logger> coreLogger;
        static std::shared_ptr<spdlog::logger> clientLogger;
        static std::shared_ptr<spdlog::logger> editorLogger;

        static void init();
    };
}  // namespace Cube

// internal log of Cube

#ifdef CB_DEBUG
    #define CB_CORE_TRACE(...)      SPDLOG_LOGGER_TRACE(::Cube::Log::coreLogger, __VA_ARGS__)
    #define CB_CORE_INFO(...)       SPDLOG_LOGGER_INFO(::Cube::Log::coreLogger, __VA_ARGS__)
    #define CB_CORE_WARN(...)       SPDLOG_LOGGER_WARN(::Cube::Log::coreLogger, __VA_ARGS__)
    #define CB_CORE_ERROR(...)      SPDLOG_LOGGER_ERROR(::Cube::Log::coreLogger, __VA_ARGS__)
    #define CB_CORE_CRITICAL(...)   SPDLOG_LOGGER_CRITICAL(::Cube::Log::coreLogger, __VA_ARGS__)

    // client logger
    #define CB_TRACE(...)           SPDLOG_LOGGER_TRACE(::Cube::Log::clientLogger, __VA_ARGS__)
    #define CB_INFO(...)            SPDLOG_LOGGER_INFO(::Cube::Log::clientLogger, __VA_ARGS__)
    #define CB_WARN(...)            SPDLOG_LOGGER_WARN(::Cube::Log::clientLogger, __VA_ARGS__)
    #define CB_ERROR(...)           SPDLOG_LOGGER_ERROR(::Cube::Log::clientLogger, __VA_ARGS__)
    #define CB_CRITICAL(...)        SPDLOG_LOGGER_CRITICAL(::Cube::Log::clientLogger, __VA_ARGS__)

    // editor logger
    #define CB_EDITOR_TRACE(...)    SPDLOG_LOGGER_TRACE(::Cube::Log::editorLogger, __VA_ARGS__)
    #define CB_EDITOR_INFO(...)     SPDLOG_LOGGER_INFO(::Cube::Log::editorLogger, __VA_ARGS__)
    #define CB_EDITOR_WARN(...)     SPDLOG_LOGGER_WARN(::Cube::Log::editorLogger, __VA_ARGS__)
    #define CB_EDITOR_ERROR(...)    SPDLOG_LOGGER_ERROR(::Cube::Log::editorLogger, __VA_ARGS__)
    #define CB_EDITOR_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(::Cube::Log::editorLogger, __VA_ARGS__)

    // assert
    #define CB_ASSERT(...) assert(__VA_ARGS__)
#else
    #define CB_CORE_TRACE(...) 
    #define CB_CORE_INFO(...)
    #define CB_CORE_WARN(...)
    #define CB_CORE_ERROR(...)
    #define CB_CORE_CRITICAL(...)

    #define CB_TRACE(...)
    #define CB_INFO(...)
    #define CB_WARN(...)
    #define CB_ERROR(...)
    #define CB_CRITICAL(...)

    #define CB_EDITOR_TRACE(...)
    #define CB_EDITOR_INFO(...)
    #define CB_EDITOR_WARN(...)
    #define CB_EDITOR_ERROR(...)
    #define CB_EDITOR_CRITICAL(...)

    #define CB_ASSERT(...)
#endif