#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <iomanip>
#include <filesystem>
#include <string>
#include <chrono>
#include <ctime>

std::string getMimeType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")  return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")   return "application/javascript";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")  return "image/png";
    if (path.size() >= 4 && (path.substr(path.size() - 4) == ".jpg" || path.substr(path.size() - 5) == ".jpeg")) return "image/jpeg";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".gif")  return "image/gif";
    return "application/octet-stream";
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

std::string url_decode(const std::string& in) {
    std::ostringstream out;
    for (size_t i = 0; i < in.length(); ++i) {
        if (in[i] == '+') {
            out << ' ';
        } else if (in[i] == '%' && i + 2 < in.length()) {
            std::istringstream hex(in.substr(i + 1, 2));
            int value;
            if (hex >> std::hex >> value) {
                out << static_cast<char>(value);
                i += 2;
            }
        } else {
            out << in[i];
        }
    }
    return out.str();
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size)) {
            return std::string(buffer.begin(), buffer.end());
        }
        return "";
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return "";
    }
}

std::string handleRootRequest(const std::map<std::string, std::string>& headers) {
    std::cout << "Handling root request..." << std::endl;
    // We can also access the values of headers if needed
    // Like for example: std::cout << "User-Agent: " << headers.at("User-Agent") << std::endl;

    std::string file_content = readFile("starter_server.html");
    if (!file_content.empty()) {
        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(file_content.length()) + "\r\n"
               "\r\n"
               + file_content;

    } else {
        return "HTTP/1.1 500 Internal Server Error\r\n"
               "Content-Type: text/plain\r\n"
               "\r\n"
               "Error reading file\r\n";
    }
}

std::string handleSubmitFormRequest(const std::map<std::string, std::string>& headers) {
    std::cout << "Handling Submit Form request..." << std::endl;

    std::string form_content = readFile("submit_form.html");
    if (!form_content.empty()) {
        return "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + std::to_string(form_content.length()) + "\r\n"
                "\r\n"
                + form_content + "\r\n";
    } else {
        return "HTTP/1.1 500 Internal Server Error\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "Error reading from file\r\n";
    }
}

std::string handleAboutRequest(const std::map<std::string, std::string>& headers) {
    std::cout << "Handling About page request..." << std::endl;

    std::string about_content = readFile("about.html");
    if (!about_content.empty()) {
        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(about_content.length()) + "\r\n"
               "\r\n"
               + about_content + "\r\n";
    } else {
        return "HTTP/1.1 500 Internal Server Error\r\n"
               "Content-Type: text/plain\r\n"
               "\r\n"
               "Error reading from file\r\n";
    }
}

std::string handleApiTimeRequest(const std::map<std::string, std::string>& headers) {
    std::cout << "Handling API Time request..." << std::endl;

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* ptm = std::gmtime(&now_c);

    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y-%m-%dT%H:%M:%SZ");
    std::string current_time_str = oss.str();

    std::string json = "{\"server_time\": \"" + current_time_str + "\"}";
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: " + std::to_string(json.length()) + "\r\n"
           "\r\n"
           + json;

}

std::string handleStaticFileRequest(const std::map<std::string, std::string>& headers, std::string uri) {
    std::cout << "Handling static file request..." << std::endl;

    std::string file_path = uri.substr(1);
    std::string file_content = readFile(file_path);
    if (!file_content.empty()) {
        std::string mime_type = getMimeType(file_path);

        return "HTTP/1.1 200 OK\r\n"
               "Content-Type: " + mime_type + "\r\n"
               "Content-Length: " + std::to_string(file_content.length()) + "\r\n"
               "\r\n"
               + file_content;
    } else {
        return "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/plain\r\n"
               "\r\n"
               "File not found.\r\n";
    }
}

std::string handleSubmitDataPostRequest(const std::map<std::string, std::string>& headers, const std::string& request_body) {
    std::cout << "Handling POST data submission..." << std::endl;

    if (request_body.empty()) {
        std::cerr << "POST request received with empty body." << std::endl;
        return "HTTP/1.1 400 Bad Request\r\n"
               "Content-Type: text/plain\r\n"
               "\r\n"
               "Bad Request: Empty POST body.\r\n";
    }

    std::istringstream body_stream(request_body);
    std::string pair;
    std::map<std::string, std::string> post_data;

    while (std::getline(body_stream, pair, '&')) {
        std::istringstream pair_stream(pair);
        std::string key, value;

        if (std::getline(pair_stream, key, '=')) {
            std::getline(pair_stream, value);
            post_data[url_decode(key)] = url_decode(value);
        }
    }

    std::cout << "Parsed POST data:" << std::endl;
    for (const auto& item : post_data) {
        std::cout << item.first << ": " <<  item.second << std::endl;
    }

    std::string response_body = "Data received:\n";
    for (const auto& item : post_data) {
        response_body += item.first + " = " + item.second + "\n";
    }

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: " + std::to_string(response_body.length()) + "\r\n"
           "\r\n"
           + response_body;
}

std::string handleNotFoundRequest(const std::map<std::string, std::string>& headers) {
    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "\r\n"
           "Not Found\r\n";
} 

int main() {
    // Creating the socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {      //Checking if the created socket is created
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Specifying the server address
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;    //listen on all available interfaces
    server_address.sin_port = htons(8080);          // Port number that we will use

    // Binding the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Error binding the socket" << std::endl;
        close(server_fd);
        return -1;
    }

    // Trying to listen (like connecting to the web server)
    if (listen(server_fd, SOMAXCONN) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_fd);
        return -1;
    } 

    std::cout << "Server listening on port 8080..." << std::endl;
    
    while (true) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        if (client_socket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;   // Proceeds to listen to another connections
        }
        std::cout << "Connection accepted from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;

        // Buffer to store the incoming request
        std::vector<char> buffer(1024);     // <-- 1 KB buffer
        ssize_t bytes_received = recv(client_socket, buffer.data(), buffer.size(), 0);

        if (bytes_received > 0) {
            std::cout << "Received " << bytes_received << " bytes:" << std::endl;
            std::cout.write(buffer.data(), bytes_received);
            std::cout << std::endl;

            std::string full_request(buffer.begin(), buffer.begin() + bytes_received);
            std::istringstream request_stream(full_request);
            std::string line;
            std::string method;
            std::string uri;
            std::string http_version;           // Using map headers for saving HTTP request headers
            std::map<std::string, std::string> headers;
            
            // Parse the first line (request line)
            // IMPORTANT: HERE, I EXPERIMENT WITH PARSING THE HEADERS OF HTTP REQUEST
            // IT IS NOT SERVING THE HTTP REQUESTS. FOR THIS, LOOK BELOW FOR COMMENTS

            std::string response;
            std::string request_body;
            if (std::getline(request_stream, line)) {
                std::istringstream line_stream(line);
                line_stream >> method >> uri >> http_version;
                std::cout << "Method: " << method << std::endl;
                std::cout << "URI: " << uri << std::endl;
                std::cout << "HTTP Version: " << http_version << std::endl;

                // Parse headers
                while (std::getline(request_stream, line) && !line.empty() && line != "\r") {
                    std::size_t colon_pos = line.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string header_name = line.substr(0, colon_pos);
                        std::string header_value = line.substr(colon_pos + 1);
                        // Trimming the leading/trailing whitespace from header value
                        size_t first_not_space = header_value.find_first_not_of(" \t");
                        if (std::string::npos != first_not_space) {
                            header_value.erase(0, first_not_space);
                        }
                        size_t last_not_space = header_value.find_last_not_of(" \t\r");
                        if (std::string::npos != last_not_space) {
                            header_value.erase(last_not_space + 1);
                        }
                        headers[header_name] = header_value;
                        std::cout << "Header - " << header_name << ": " << header_value << std::endl;
                    }
                }

                // Now our headers are stored in the "headers" map
                std::cout << "\nAll Headers:" << std::endl;
                for (const auto& header : headers) {
                    std::cout << header.first << ": " << header.second << std::endl;
                }

                if (method == "POST") {
                    // We extract the body (as before)
                    std::size_t body_pos = full_request.find("\r\n\r\n");
                    if (body_pos != std::string::npos) {
                        request_body = full_request.substr(body_pos + 4);
                        std::cout << "Raw POST request body: " << request_body << std::endl;
                        std::istringstream body_stream(request_body);
                        std::string pair;
                        std::map<std::string, std::string> post_data;

                        while (std::getline(body_stream, pair, '&')) {
                            std::istringstream pair_stream(pair);
                            std::string key, value;

                            if (std::getline(pair_stream, key, '=')) {
                                std::getline(pair_stream, value);
                                post_data[url_decode(key)] = url_decode(value); // Consider URL-decoding here
                            }
                        }

                        std::cout << "Parsed POST data:" << std::endl;
                        for (const auto& item : post_data) {
                            std::cout << item.first << ": " << item.second << std::endl;
                        }

                        // Build response
                        std::string response_body = "Data received:\n";
                        for (const auto& item : post_data) {
                            response_body += item.first + " = " + item.second + "\n";
                        }

                        response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: " + std::to_string(response_body.length()) + "\r\n"
                                   "\r\n"
                                   + response_body;
                    }
                }
                
                std::string response_body = "Received your request with the following headers:\n";
                for (const auto& header : headers) {
                    response_body += header.first + ": " + header.second + "\n";
                }
                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: " + std::to_string(response_body.length()) + "\r\n"
                           "\r\n"
                           + response_body;
            } else {
                // Handle error if the first line is not received
                response = "HTTP/1.1 400 Bad Request\r\n"
                           "Content-Type: text/plain\r\n"
                           "\r\n"
                           "Invalid request.\r\n";
            }
            
            // ACTUAL SERVING OF THE HTTP REQUESTS FROM USERS
            // ||||||||||||||||||||||||||||||||||||||||||||||
            // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

            if (method == "GET" && uri == "/") {
                response = handleRootRequest(headers);
            } else if (method == "GET" && uri == "/submit_form") {
                response = handleSubmitFormRequest(headers);
            } else if (method == "GET" && uri == "/about") {
                response = handleAboutRequest(headers);
            } else if (method == "GET" && uri == "/api/time") {
                response = handleApiTimeRequest(headers);
            } else if (method == "GET" && startsWith(uri, "/static/")) {
                response = handleStaticFileRequest(headers, uri);
            } else if (method == "POST" && uri == "/submit-data") {
                response = handleSubmitDataPostRequest(headers, request_body);
            } else {
                response = handleNotFoundRequest(headers);
            }

            // Sending the response back to the client
            ssize_t bytes_sent = send(client_socket, response.c_str(), response.length(), 0);
            if (bytes_sent == -1) {
                std::cerr << "Error sending response" << std::endl;
            } else {
                std::cout << "Sent " << bytes_sent << " bytes" << std::endl;
            }
        } else if (bytes_received == 0) {
            std::cout << "Client disconnected" << std::endl;
        } else {
            std::cerr << "Error receiving data" << std::endl;
        }
        // Closing the connection immediately (of client)
        close(client_socket);
    }
    
    // Closing the server socket
    close(server_fd);
    return 0;
}