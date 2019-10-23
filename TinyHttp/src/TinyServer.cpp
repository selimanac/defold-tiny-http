#include <TinyServer.hpp>
#include <iostream>
extern void QueueCommand(int id, char *ws_message);


std::string dump_headers(const Headers &headers)
{
    std::string s;
    char buf[BUFSIZ];

    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }

    return s;
}

std::string log(const Request &req, const Response &res)
{
    std::string s;
    char buf[BUFSIZ];

    s += "================================\n";

    snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
             req.version.c_str(), req.path.c_str());
    s += buf;

    std::string query;
    for (auto it = req.params.begin(); it != req.params.end(); ++it)
    {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "%c%s=%s",
                 (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
                 x.second.c_str());
        query += buf;
    }
    snprintf(buf, sizeof(buf), "%s\n", query.c_str());
    s += buf;

    s += dump_headers(req.headers);

    s += "--------------------------------\n";

    snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
    s += buf;
    s += dump_headers(res.headers);
    s += "\n";

    if (!res.body.empty())
    {
        s += res.body;
    }

    s += "\n";

    return s;
}




void TinyServer::initClient(const char *n_host, int n_port)
{

    chost = n_host;
    cport = n_port;

    cli = new Client(chost, cport);
}

void TinyServer::sayHi()
{
    auto res = cli->Get("/hi");
    if (res && res->status == 200)
    {
        char *result = new char[res->body.length() + 1];
        strcpy(result, res->body.c_str());

        QueueCommand(1, result);
    }
}

void TinyServer::startServ(const char *n_host, int n_port)
{

    host = n_host;
    port = n_port;

    serverThread = new std::thread([&]() {
        Server svr;

        svr.Get("/hi", [this](const Request &req, Response &res) {
            res.set_content("{ \"test\": \"Defold says hi!\" }", contentType);
            QueueCommand(0, "{ \"result\": 1 }");
        });

        svr.Post("/post", [&](const Request &req, Response &res) {
            printf("req.get_param_value %s\n", req.get_param_value("data").c_str());
            
            printf("%s", log(req, res).c_str());

            char *data_cstr = new char[req.get_param_value("data").length() + 1];
            strcpy(data_cstr, req.get_param_value("data").c_str());
            QueueCommand(0, data_cstr);
        });

        svr.Get("/stop", [&](const Request &req, Response &res) {
            svr.stop();
        });

        svr.listen(host, port);
    });
}