#pragma once

#include "../utils/Env.hpp"
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>
#include <string>

class Database {
private:
  mongocxx::client client;
  mongocxx::database db;

public:
  Database() {
    // Load env vars
    Env::load();

    std::string uri_str = Env::get("MONGODB_URI", "mongodb://localhost:27017");
    std::string db_name = Env::get("DB_NAME", "testdb");

    std::cout << "Connecting to MongoDB at " << uri_str << " ..." << std::endl;

    mongocxx::uri uri{uri_str};
    client = mongocxx::client{uri};
    db = client[db_name];

    std::cout << "Connected to database: " << db_name << std::endl;
  }

  mongocxx::database &getDb() { return db; }

  mongocxx::collection getCollection(const std::string &name) {
    return db[name];
  }
};
