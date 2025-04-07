#pragma once
#include <string>
#include <vector>
#include <map>
class Database {
public:
    static void connect();
    static bool execute(const std::string& query, const std::vector<std::string>& params);
    static std::vector<std::map<std::string, std::string>> query(const std::string& query, const std::vector<std::string>& params);
};
