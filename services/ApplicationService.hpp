#pragma once

#include "../models/Application.hpp"
#include "../third_party/json.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>

using json = nlohmann::json;
using namespace bsoncxx::builder::basic;

class ApplicationService {
private:
  mongocxx::collection collection;
  bool hasUserApplied(const std::string &userId, const std::string &jobId) {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto filter = make_document(kvp("userId", userId), kvp("jobId", jobId));

    auto existing = collection.find_one(filter.view());
    return existing.has_value();
  }

public:
  ApplicationService(mongocxx::collection coll) : collection(coll) {}

  // ---------------------------------------------------------
  // CREATE APPLICATION WITH CHECK
  // ---------------------------------------------------------
  json createApplication(const json &appJson) {
    std::string userId = appJson.value("userId", "");
    std::string jobId = appJson.value("jobId", "");

    // 1. CHECK
    if (hasUserApplied(userId, jobId)) {
      return {{"error", "You have already applied to this job"},
              {"status", 400}};
    }

    // 2. CREATE MODEL
    Application app(appJson);
    auto bson_value = app.toBson();

    // 3. INSERT
    auto result = collection.insert_one(bson_value.view());
    if (!result) {
      return {{"error", "Failed to create application"}, {"status", 500}};
    }

    // 4. RETURN JSON WITH _id
    json saved = app.toJson();
    saved["_id"] = result->inserted_id().get_oid().value.to_string();
    saved["status"] = 200;

    return saved;
  }

  // ---------------------------------------------------------
  // GET APPLICATIONS BY USER
  // ---------------------------------------------------------
  std::vector<json> getApplicationsByUser(const std::string &userId) {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto filter = make_document(kvp("userId", userId));

    std::vector<json> list;
    for (auto &doc : collection.find(filter.view())) {
      list.push_back(json::parse(bsoncxx::to_json(doc)));
    }
    return list;
  }

  // ---------------------------------------------------------
  // GET APPLICATIONS BY JOB
  // ---------------------------------------------------------
  std::vector<json> getApplicationsByJob(const std::string &jobId) {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto filter = make_document(kvp("jobId", jobId));

    std::vector<json> list;
    for (auto &doc : collection.find(filter.view())) {
      list.push_back(json::parse(bsoncxx::to_json(doc)));
    }
    return list;
  }

  // ---------------------------------------------------------
  // GET APPLICATIONS BY COMPANY
  // ---------------------------------------------------------
  std::vector<json> getApplicationsByCompany(const std::string &companyId) {
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto filter = make_document(kvp("companyId", companyId));

    std::vector<json> list;
    for (auto &doc : collection.find(filter.view())) {
      list.push_back(json::parse(bsoncxx::to_json(doc)));
    }
    return list;
  }
};