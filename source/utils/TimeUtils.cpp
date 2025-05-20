// src/utils/TimeUtils.cpp
#include "utils/TimeUtils.hpp" // Adjusted include path
#include <iomanip>   // For std::put_time, std::get_time
#include <sstream>   // For std::stringstream
#include <iostream>  // For potential error logging in parseFromString

namespace TimeUtils {

time_t getCurrentTimestamp() {
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

std::string formatTimestamp(time_t timestamp, const std::string& format) {
    if (timestamp == 0) { // Handle uninitialized or epoch timestamps gracefully
        return "N/A"; 
    }
    std::tm tm_snapshot;
    // Use platform-specific thread-safe versions of localtime
    #if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm_snapshot, &timestamp);
    #else // POSIX
        localtime_r(&timestamp, &tm_snapshot);
    #endif
    
    std::stringstream ss;
    ss << std::put_time(&tm_snapshot, format.c_str());
    return ss.str();
}

time_t parseFromString(const std::string& timeString, const std::string& format) {
    std::tm t{};
    std::istringstream ss(timeString);

    ss >> std::get_time(&t, format.c_str());
    if (ss.fail()) {
        std::cerr << "TimeUtils::parseFromString - Failed to parse time string: " << timeString << " with format: " << format << std::endl;
        return 0; 
    }
    t.tm_isdst = -1; // Let mktime determine DST
    return std::mktime(&t);
}

} // namespace TimeUtils