#pragma once

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <vector>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <bsoncxx/oid.hpp>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/sub_array.hpp>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/collection.hpp>

using json = nlohmann::json;

class CompanyService
{
    mongocxx::collection companycollection;
    mongocxx::collection jobcollection;

public:
    CompanyService(mongocxx::collection db, mongocxx::collection j) : companycollection(db), jobcollection(j) {}
    std::vector<json> getAllJobs(const bsoncxx::oid &companyId)
    {
        std::vector<json> jobs;

        bsoncxx::builder::basic::document filter{};
        filter.append(
            bsoncxx::builder::basic::kvp("companyId", companyId.to_string()));

        std::cout << "FILTER = " << bsoncxx::to_json(filter.view()) << std::endl;

        for (auto &&doc : jobcollection.find(filter.view()))
        {
            auto tmp = json::parse(bsoncxx::to_json(doc));

            // if (tmp.value("status", "pending") == "approved")
            jobs.push_back(tmp);
        }

        return jobs;
    }

    std::vector<json> getAllCompanies()
    {
        std::vector<json> companies;

        auto cursor = companycollection.find({});
        for (auto &&doc : cursor)
        {
            companies.emplace_back(json::parse(bsoncxx::to_json(doc)));
        }
        std::cout << companies.size() << std::endl;
        return companies;
    }

    json getPublicCompany(const bsoncxx::oid &companyId)
    {
        auto result = companycollection.find_one(
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("_id", companyId)));

        if (!result)
            return json(); // company not found

        json company = json::parse(bsoncxx::to_json(result->view()));

        // Удалить приватную информацию
        company.erase("password");
        return company;
    }
    json updateCompanyProfile(const bsoncxx::oid &companyId, const json &body)
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        // 1) Найти компанию
        auto existing = companycollection.find_one(
            make_document(kvp("_id", companyId)));

        if (!existing)
        {
            return {
                {"error", "Company not found"},
                {"status", 404}};
        }

        // 2) Подготовить поля для обновления
        bsoncxx::builder::basic::document setDoc{};

        for (auto it = body.begin(); it != body.end(); ++it)
        {
            const std::string &key = it.key();

            // запрещённые поля
            if (key == "_id" || key == "createdAt" || key == "password" ||
                key == "role" || key == "email")
                continue;

            append_json_value(setDoc, key, it.value());
        }

        // updatedAt
        setDoc.append(kvp(
            "updatedAt",
            bsoncxx::types::b_date{std::chrono::system_clock::now()}));

        // 3) Выполнить обновление
        auto result = companycollection.update_one(
            make_document(kvp("_id", companyId)),
            make_document(kvp("$set", setDoc.extract())));

        if (!result || result->modified_count() == 0)
        {
            return {
                {"error", "Update failed"},
                {"status", 400}};
        }

        return {{"message", "Company updated successfully"}};
    }
    static void append_json_value(bsoncxx::builder::basic::document &doc,
                                  const std::string &key,
                                  const json &value)
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::sub_array;

        if (value.is_null())
            doc.append(kvp(key, bsoncxx::types::b_null{}));
        else if (value.is_boolean())
            doc.append(kvp(key, value.get<bool>()));
        else if (value.is_number_integer())
            doc.append(kvp(key, static_cast<int64_t>(value.get<int64_t>())));
        else if (value.is_number_unsigned())
            doc.append(kvp(key, static_cast<int64_t>(value.get<uint64_t>())));
        else if (value.is_number_float())
            doc.append(kvp(key, value.get<double>()));
        else if (value.is_string())
            doc.append(kvp(key, value.get<std::string>()));
        else if (value.is_array())
        {
            doc.append(kvp(key, [&](sub_array arr)
                           {
                for (const auto &el : value)
                {
                    if (el.is_null())
                        arr.append(bsoncxx::types::b_null{});
                    else if (el.is_boolean())
                        arr.append(el.get<bool>());
                    else if (el.is_number_integer())
                        arr.append(static_cast<int64_t>(el.get<int64_t>()));
                    else if (el.is_number_unsigned())
                        arr.append(static_cast<int64_t>(el.get<uint64_t>()));
                    else if (el.is_number_float())
                        arr.append(el.get<double>());
                    else if (el.is_string())
                        arr.append(el.get<std::string>());
                    else
                        arr.append(el.dump());
                } }));
        }
        else
            doc.append(kvp(key, value.dump()));
    }
};