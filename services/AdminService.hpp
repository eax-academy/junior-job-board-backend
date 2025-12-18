#pragma once

#include <vector>
#include <chrono>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>
#include "../third_party/json.hpp"
#include <bsoncxx/builder/basic/document.hpp>

using json = nlohmann::json;
using namespace bsoncxx::builder::basic;

class AdminService
{
private:
    mongocxx::collection jobcollection;
    mongocxx::collection applicationcollection;

public:
    AdminService(mongocxx::collection j, mongocxx::collection a) : jobcollection(j), applicationcollection(a) {}

    // GET /admin/jobs/pending
    std::vector<json> getPendingJobs()
    {
        std::vector<json> pending;

        auto filter = make_document(kvp("status", "pending"));
        auto cursor = jobcollection.find(filter.view());

        for (auto &&doc : cursor)
        {
            pending.push_back(json::parse(bsoncxx::to_json(doc)));
        }

        return pending;
    }

    std::vector<json> getPendingApplications()
    {
        std::vector<json> pending;

        auto filter = make_document(kvp("status", "pending"));
        auto cursor = applicationcollection.find(filter.view());

        for (auto &&doc : cursor)
        {
            pending.push_back(json::parse(bsoncxx::to_json(doc)));
        }

        return pending;
    }

    // APPROVE JOB
    bool approveJob(const std::string &id)
    {
        bsoncxx::oid job_oid;
        try {
            job_oid = bsoncxx::oid{id};
        } catch (...) {
            return false;
        }

        auto filter = make_document(kvp("_id", job_oid));

        auto update = make_document(
            kvp("$set", make_document(
                kvp("status", "approved"),
                kvp("updatedAt", bsoncxx::types::b_date{
                     std::chrono::system_clock::now() })
            ))
        );

        auto result = jobcollection.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    }

    bool approveApplication(const std::string &id)
    {
        bsoncxx::oid appl_oid;
        try {
            appl_oid = bsoncxx::oid{id};
        } catch (...) {
            return false;
        }

        auto filter = make_document(kvp("_id", appl_oid));

        auto update = make_document(
            kvp("$set", make_document(
                kvp("status", "approved"),
                kvp("updatedAt", bsoncxx::types::b_date{
                     std::chrono::system_clock::now() })
            ))
        );

        auto result = applicationcollection.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    }

    // REJECT JOB
    bool rejectJob(const std::string &id)
    {
        bsoncxx::oid job_oid;
        try {
            job_oid = bsoncxx::oid{id};
        } catch (...) {
            return false;
        }

        auto filter = make_document(kvp("_id", job_oid));

        auto update = make_document(
            kvp("$set", make_document(
                kvp("status", "rejected"),
                kvp("updatedAt", bsoncxx::types::b_date{
                     std::chrono::system_clock::now() })
            ))
        );

        auto result = jobcollection.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    }

    bool rejectApplication(const std::string &id)
    {
        bsoncxx::oid appl_oid;
        try {
            appl_oid = bsoncxx::oid{id};
        } catch (...) {
            return false;
        }

        auto filter = make_document(kvp("_id", appl_oid));

        auto update = make_document(
            kvp("$set", make_document(
                kvp("status", "rejected"),
                kvp("updatedAt", bsoncxx::types::b_date{
                     std::chrono::system_clock::now() })
            ))
        );

        auto result = applicationcollection.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    }
};
