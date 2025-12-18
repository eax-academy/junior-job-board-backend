#pragma once

#include "../third_party/httplib.h"
#include "../services/AdminService.hpp"
#include "../third_party/json.hpp"
#include <exception>

using json = nlohmann::json;


class AdminController
{
private:
    AdminService &service;

public:
    AdminController(AdminService &srv) : service(srv) {}

    // GET /admin/jobs/pending
    void getPendingJobs(httplib::Response &res)
    {
        try
        {
            auto jobs = service.getPendingJobs();
            res.status = 200;
            res.set_content(json(jobs).dump(), "application/json");
        }
        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content("Failed to fetch pending jobs", "text/plain");
        }
    }

    void getPendingApplications(httplib::Response &res)
    {
        try
        {
            auto apps = service.getPendingApplications();
            res.status = 200;
            res.set_content(json(apps).dump(), "application/json");
        }
        catch (const std::exception &e)
        {
            res.status = 500;
            res.set_content("Failed to fetch pending applications", "text/plain");
        }
    }

    // PATCH /admin/jobs/:id/approve
    void approveJob(const std::string &id, httplib::Response &res)
    {
        bool ok = service.approveJob(id);

        if (ok)
        {
            res.status = 200;
            res.set_content("Job approved", "text/plain");
        }
        else
        {
            res.status = 400;
            res.set_content("Failed to approve job", "text/plain");
        }
    }

    void approveApplication(const std::string &id, httplib::Response &res)
    {
        bool ok = service.approveApplication(id);

        if (ok)
        {
            res.status = 200;
            res.set_content("application approved", "text/plain");
        }
        else
        {
            res.status = 400;
            res.set_content("Failed to approve application", "text/plain");
        }
    }

    // PATCH /admin/jobs/:id/reject
    void rejectJob(const std::string &id, httplib::Response &res)
    {
        bool ok = service.rejectJob(id);

        if (ok)
        {
            res.status = 200;
            res.set_content("Job rejected", "text/plain");
        }
        else
        {
            res.status = 400;
            res.set_content("Failed to reject job", "text/plain");
        }
    }

    void rejectApplication(const std::string &id, httplib::Response &res)
    {
        bool ok = service.rejectApplication(id);

        if (ok)
        {
            res.status = 200;
            res.set_content("application rejected", "text/plain");
        }
        else
        {
            res.status = 400;
            res.set_content("Failed to reject application", "text/plain");
        }
    }
};
