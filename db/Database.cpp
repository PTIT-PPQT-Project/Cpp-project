#include "Database.h"
#include "../config/db_config.h"
#include <iostream>
void Database::connect() {
    std::cout << "[Database] Connecting to " << DB_HOST << " ...\n";
}
bool Database::execute(const std::string& query, const std::vector<std::string>& params) {
    std::cout << "[Execute] " << query << "\n";
    return true;
}
std::vector<std::map<std::string, std::string>> Database::query(const std::string& query, const std::vector<std::string>& params) {
    std::cout << "[Query] " << query << "\n";
    return {};
}
