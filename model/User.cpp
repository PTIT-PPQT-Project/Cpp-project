#include "User.h"
#include "../db/Database.h"
#include "../utils/Hash.h"
#include <vector>
bool User::createUser(const std::string& username, const std::string& password) {
    std::string hash = Hash::sha256(password);
    std::string query = "INSERT INTO users (username, password_hash, role) VALUES (?, ?, 'user')";
    return Database::execute(query, {username, hash});
}
User* User::authenticate(const std::string& username, const std::string& password) {
    std::string query = "SELECT id, password_hash, role FROM users WHERE username = ?";
    auto result = Database::query(query, {username});
    if (!result.empty() && result[0]["password_hash"] == Hash::sha256(password)) {
        User* u = new User();
        u->id = std::stoi(result[0]["id"]);
        u->username = username;
        u->password_hash = result[0]["password_hash"];
        u->role = result[0]["role"];
        return u;
    }
    return nullptr;
}
