#include <iostream>
#include <sys/socket.h>         // First version of starter_server
#include <netinet/in.h>         // Here I have used if-else conditional blocks to handle HTTP requests
#include <unistd.h>             // In the next versions, I will try another approaches
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <iomanip>
#include <filesystem>
#include <string>

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
            // Printing the received data to the console
            std::cout.write(buffer.data(), bytes_received);
            std::cout << std::endl;

            // Basic HTTP request parsing below
            std::istringstream request_stream(std::string(buffer.begin(), buffer.begin() + bytes_received));
            std::string method;
            std::string uri;
            std::string http_version;

            request_stream >> method >> uri >> http_version;

            // Showing HTTP response from client
            std::cout << "Method: " << method << std::endl;
            std::cout << "URI: " << uri << std::endl;
            std::cout << "HTTP Version: " << http_version << std::endl;
            
            // Preparing our servers HTTP response
            std::string response;
            if (method == "GET" && uri == "/") {
                std::string file_content = readFile("starter_server.html");
                if (!file_content.empty()) {    // if file is read successfully, HTTP 200 OK
                    response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + std::to_string(file_content.length()) + "\r\n"
                           "\r\n"               // above, we share the HTML file
                           + file_content + "\r\n";
                } else {                // if file reading resulted in error, HTTP 500 error
                    response = "HTTP/1.1 500 Internal Server Error\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "Error reading file\r\n";
                }
            } else if (method == "GET" && uri == "/submit_form") {
                std::string form_content = readFile("submit_form.html");
                if (!form_content.empty()) {
                    response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: " + std::to_string(form_content.length()) + "\r\n"
                               "\r\n"
                               + form_content + "\r\n";
                } else {
                    response = "HTTP/1.1 500 Internal Server Error\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "Error reading from file\r\n";
                }
            } else if (method == "GET" && uri == "/about") {
                std::string content = readFile("about.html");
                if (!content.empty()) {
                    response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: " + std::to_string(content.length()) + "\r\n"
                               "\r\n"
                               + content;
                } else {
                    response = "HTTP/1.1 500 Internal Server Error\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "Error reading from file\r\n";
                }
            } else if (method == "GET" && uri == "/api/time") {
                std::string current_time = "2025-05-10T22:12:50Z";
                std::string json = "{\"server_time\": \"" + current_time + "\"}";

                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: " + std::to_string(json.length()) + "\r\n"
                           "\r\n"
                           + json;
            } else if (method == "GET" && startsWith(uri, "/static/")) {
                std::string file_path = uri.substr(1);
                std::string file_content = readFile(file_path);
                if (!file_content.empty()) {
                    std::string mime_type = getMimeType(file_path);

                    response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: " + mime_type + "\r\n"
                               "Content-Length: " + std::to_string(file_content.length()) + "\r\n"
                               "\r\n"
                               + file_content;
                } else {
                    response = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Type: text/plain\r\n"
                               "\r\n"
                               "File not found.\r\n";
                }
            } else if (method == "POST" && uri == "/submit-data") {
                std::string full_request(buffer.begin(), buffer.begin() + bytes_received);
                std::size_t body_pos = full_request.find("\r\n\r\n");

                std::string request_body;
                if (body_pos != std::string::npos) {
                    request_body = full_request.substr(body_pos + 4); // Extract POST body
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
            } else {
                // For any other request, we will send a 404 Not Found
                response = "HTTP/1.1 404 Not Found\r\n"
                           "Content-Type: text/plain\r\n"
                           "\r\n"
                           "Not Found\r\n";
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