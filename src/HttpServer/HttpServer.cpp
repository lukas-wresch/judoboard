#include <fstream>
#include <sstream>
#include "HttpServer.h"
#include <mongoose/mongoose.h>



const std::string HttpServer::LoadFile(const std::string& Filename)
{
    std::ifstream inFile(Filename, std::ios::binary);//open the input file

    if (inFile)
    {
        std::stringstream strStream;
        strStream << inFile.rdbuf();//read the file
        return strStream.str();//str holds the content of the file
    }

    return "";
}



std::string HttpServer::DecodeURLEncoded(const std::string& Input, const char* VariableName)
{
    char buffer[4096] = {};
    mg_get_var(Input.c_str(), Input.size(), VariableName, buffer, sizeof(buffer));
    return buffer;
}



const std::string HttpServer::CookieHeader(const std::string& Name, const std::string& Value, const std::string& Location)
{
    return "Set-Cookie: " + Name + "=" + Value + "; Path=" + Location;
}



void* HttpServer::Callback(mg_event Event, mg_connection* Connection)
{
    const mg_request_info* request_info = mg_get_request_info(Connection);

    if (Event == MG_NEW_REQUEST)
    {
        std::string request_uri = request_info->uri;
        std::string query = "";
        if (request_info->query_string)
            query = request_info->query_string;
        std::string request_body;

        std::string content = "No data registered with request '" + request_uri + "'";
        ResourceType Type = ResourceType::Plain;


        if (Connection->content_len > 0)
        {
            char* body = new char[(size_t)Connection->content_len + 1];
            mg_read(Connection, body, (size_t)Connection->content_len);
            body[Connection->content_len] = '\0';
            request_body = body;
            delete[] body;
        }


        RequestInfo info;
        info.RemoteIP   = request_info->remote_ip;
        info.RemotePort = request_info->remote_port;

        for (int i = 0; i < request_info->num_headers; i++)
            info.m_Headers.emplace_back(request_info->http_headers[i].name, request_info->http_headers[i].value);

        Request request(query, request_body, info);

        auto item = m_Resources.find(request_uri);

        if (item != m_Resources.end())
        {
            Type = item->second.m_Type;
            if (item->second.m_Callback)
                content = item->second.m_Callback(request);
        }


        std::string MIMEType = "text/plain";
        if (Type == ResourceType::HTML)
            MIMEType = "text/html";
        else if (Type == ResourceType::CSS)
            MIMEType = "text/css";
        else if (Type == ResourceType::JavaScript)
            MIMEType = "text/javascript";
        else if (Type == ResourceType::Image_JPG)
            MIMEType = "image/jpeg";
        else if (Type == ResourceType::Image_PNG)
            MIMEType = "image/png";


        if (request.m_ResponseHeader.length() > 0)
        {
            mg_printf(Connection,
                "HTTP/1.1 200 OK\r\n"
                "Connection: Keep-Alive\r\n"
                "Keep-Alive: timeout=60, max=1000\r\n"
                "Content-Type: %s;\r\n"
                "Content-Length: %d\r\n"//Always set Content-Length
                "%s\r\n"
                "\r\n",
                MIMEType.c_str(), content.length(), request.m_ResponseHeader.c_str());
        }
        else
        {
            mg_printf(Connection,
                "HTTP/1.1 200 OK\r\n"
                "Connection: Keep-Alive\r\n"
                "Keep-Alive: timeout=60, max=1000\r\n"
                "Content-Type: %s;\r\n"
                "Content-Length: %d\r\n"//Always set Content-Length
                "\r\n",
                MIMEType.c_str(), content.length());
        }

        mg_write(Connection, content.c_str(), content.length());
    }

    return nullptr;
}



HttpServer::HttpServer(uint16_t Port) : m_Port(Port)
{
    char port_string[32];
#ifdef _WIN32
    _itoa_s(Port, port_string, 10);
#else
    snprintf(port_string, sizeof(port_string), "%d", Port);
#endif

    const char* options[] = { "listening_ports", port_string, "enable_keep_alive", "yes", nullptr };
    //const char* options[] = { "listening_ports", port_string, "enable_keep_alive", "yes", "num_threads", "1", nullptr};

    m_Context = mg_start([](mg_event Event, mg_connection* Connection) { ((HttpServer*)Connection->ctx->user_data)->Callback(Event, Connection); }, this, options);
}



HttpServer::~HttpServer()
{
    if (m_Context)
        mg_stop(m_Context);
}



void HttpServer::RegisterResource(const std::string& URI, std::function<std::string(Request&)> Callback, ResourceType Type)
{
    m_Resources.insert({ URI, Resource(Type, Callback) });
}