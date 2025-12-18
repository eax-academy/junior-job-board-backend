#pragma once
#include "../third_party/httplib.h"
#include "../controllers/CompanyController.hpp"
#include "../middleware/AuthMiddleware.hpp"

void registerCompanyRoutes(httplib::Server &server, CompanyController &companycontroller)
{
    server.Get(R"(/companies/([a-fA-F0-9]{24})/jobs)",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   // 1) Check JWT
                   res.set_header("Access-Control-Allow-Origin", "*");
                   std::string companyIdFromToken;
                   if (!AuthMiddleware::verifyCompany(req, res, companyIdFromToken))
                   {
                       return; // middleware already set error in res
                   }

                   // 2) Extract companyId from URL
                   std::string companyIdStr = req.matches[1].str();

                   // 3) Compare: company can only see its own jobs
                   if (companyIdStr != companyIdFromToken)
                   {
                       res.status = 403;
                       res.set_content("You can view only your own jobs", "text/plain");
                       return;
                   }

                   // 4) Convert to ObjectId
                   try
                   {
                       bsoncxx::oid companyId(companyIdStr);
                       companycontroller.getAllJobs(companyId, res);
                   }
                   catch (...)
                   {
                       res.status = 400;
                       res.set_content("Invalid Company ObjectId", "text/plain");
                   }
               });
    server.Get(R"(/companies/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*");
                   companycontroller.getCompanyPublicProfile(req, res);
               });
    server.Get("/companies",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*");
                   companycontroller.getAllCompanies(req, res);
               });
    server.Put(R"(/companies/([a-fA-F0-9]{24}))",
               [&](const httplib::Request &req, httplib::Response &res)
               {
                   res.set_header("Access-Control-Allow-Origin", "*");

                   std::string companyIdFromToken;
                   if (!AuthMiddleware::verifyCompany(req, res, companyIdFromToken))
                       return;

                   std::string pathId = req.matches[1].str();

                   // Owner check
                   if (companyIdFromToken != pathId)
                   {
                       res.status = 403;
                       res.set_content("Forbidden: cannot update another company", "text/plain");
                       return;
                   }

                   companycontroller.updateCompanyProfile(req, res, companyIdFromToken);
               });
}
