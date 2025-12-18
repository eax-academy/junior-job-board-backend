#include "config/Database.hpp"
#include "models/Company.hpp"
#include "routes/AdminRoutes.hpp"
#include "routes/ApplicationRoutes.hpp"
#include "routes/AuthRoutes.hpp"
#include "routes/CompanyRoutes.hpp"
#include "routes/JobRoutes.hpp"
#include "routes/UserRoutes.hpp"
#include "utils/jwt.hpp"

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>

mongocxx::instance global_mongo_instance{}; // MUST be created once globally!

int main()
{
    // Initialize DB connection via Database class
    Database database;
    auto db = database.getDb();

    // Optional: test list of collections
    try
    {
        auto collections = db.list_collection_names();
        std::cout << "Connected to MongoDB Atlas!" << std::endl;

        if (collections.empty())
            std::cout << "No collections found yet (DB is empty)." << std::endl;
        else
        {
            std::cout << "Collections:" << std::endl;
            for (auto &name : collections)
                std::cout << " - " << name << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "MongoDB connection test error: " << e.what() << std::endl;
    }

    // Get collections
    auto usersCollection = database.getCollection("users");
    auto emailCollection = database.getCollection("email");
    auto companyCollection = database.getCollection("company");
    auto jobCollection = database.getCollection("jobs");
    auto applicationCollection = database.getCollection("applications");

    // Init Server
    httplib::Server server;

    server.Options(".*", [&](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods",
                       "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        res.set_header("Access-Control-Allow-Headers",
                       "Content-Type, Authorization");
        res.status = 200;
    });

    // Services & Controllers
    UserService userService(usersCollection);
    UserController userController(userService);

    UserAuthService userauthservice(usersCollection, emailCollection);
    UserAuthController userauthcontroller(userauthservice);

    CompanyAuthService companyauthservice(companyCollection, emailCollection);
    CompanyAuthController companyauthcontroller(companyauthservice);

    AuthService authservice(companyCollection, usersCollection);
    AuthController authcontroller(authservice);

    JobService jobservice(jobCollection);
    JobController jobcontroller(jobservice);

    CompanyService companyservice(companyCollection, jobCollection);
    CompanyController companycontroller(companyservice);

    ApplicationService applicationservice(applicationCollection);
    ApplicationController applicationController(applicationservice, jobservice);

    AdminService adminservice(jobCollection, applicationCollection);
    AdminController admincontroller(adminservice);

    // Register routes
    registerApplicationRoutes(server, applicationController);
    registerUserRoutes(server, userController);
    registerUserAuthRoutes(server, userauthcontroller);
    registerCompanyAuthRoutes(server, companyauthcontroller);
    login(server, authcontroller);
    registerJobRoutes(server, jobcontroller);
    registerCompanyRoutes(server, companycontroller);
    registerAdminRoutes(server, admincontroller);

    // Start server
    std::cout << "🚀 Server running on http://0.0.0.0:8080\n";
    server.listen("0.0.0.0", 8080);
}
