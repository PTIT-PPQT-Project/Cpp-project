// include/utils/TimeUtils.hpp
#pragma once

#include <string>
#include <ctime>     // For time_t, tm
#include <chrono>    // For std::chrono functions

namespace TimeUtils {
    // Gets the current Unix timestamp (seconds since epoch)
    time_t getCurrentTimestamp();

    // Formats a Unix timestamp into a human-readable string.
    // Default format: YYYY-MM-DD HH:MM:SS
    std::string formatTimestamp(time_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");

    // (Optional) Parses a formatted time string back to a Unix timestamp.
    // Returns 0 on failure. This is a simplified implementation.
    time_t parseFromString(const std::string& timeString, const std::string& format = "%Y-%m-%d %H:%M:%S");
}