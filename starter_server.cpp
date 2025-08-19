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

std::map<std::string, std::string> parseUrlEncodedFormData(const std::string& encoded_data) {
    std::map<std::string, std::string> parsed_data;
    if (encoded_data.empty()) {
        return parsed_data;     // Empty map, if no data
    }
    std::istringstream body_stream(encoded_data);
    std::string pair;

    while (std::getline(body_stream, pair, '&')) {
        std::istringstream pair_stream(pair);
        std::string key, value;

        if (std::getline(pair_stream, key, '=')) {
            std::getline(pair_stream, value);
            parsed_data[url_decode(key)] = url_decode(value);
        }
    }
    return parsed_data;
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
    std::tm* ptm = std::localtime(&now_c);

    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y-%m-%d %H:%M:%S");
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
        std::string error_html_body = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Bad Request</title>
            <link rel="stylesheet" href="/static/style.css>
        </head>
        <body>
            <div class="container">
                <h1>Bad Request</h1>
                <p>The message submission failed: The request body was empty.</p>
                <p><a href="/submit_form">Go back to the submission form</a></p>
            </div>
        </body>
        </html>
        )";

        return "HTTP/1.1 400 Bad Request\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(error_html_body.length()) + "\r\n"
               "\r\n"
               + error_html_body;
    }

    std::map<std::string, std::string> post_data = parseUrlEncodedFormData(request_body);
    std::cout << "Parsed POST data:" << std::endl;
    for (const auto& item : post_data) {
        std::cout << item.first << ": " <<  item.second << std::endl;
    }

    // Getting user message from the parsed data (for now, we will save it in submissions.txt)
    std::string user_message = "";
    if (post_data.count("user_message")) {
        user_message = post_data.at("user_message");
    }

    // FIRST OF ALL, we check if the user_message field itself is not empty
    if (user_message.empty()) {
        std::cerr << "User submitted an empty message." << std::endl;
        std::string error_html_body = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Bad Request</title>
            <link rel="stylesheet" href="/static/style.css">
        </head>
        <body>
            <div class="container">
                <h1>Bad Request</h1>
                <p>The message submission failed: You submitted an empty message. Please enter some text.</p>
                <p><a href="/submit_form">Go back to the submission form</a></p>
            </div>
        </body>
        </html>
        )";

        return "HTTP/1.1 400 Bad Request\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: " + std::to_string(error_html_body.length()) + "\r\n"
               "\r\n"
               + error_html_body;
    }
    
    // Simple HTML entity encoding
    size_t pos = user_message.find("&"); while(pos != std::string::npos){user_message.replace(pos, 1, "&amp;"); pos = user_message.find("&", pos + 5); }
    pos = user_message.find("<"); while(pos != std::string::npos) {user_message.replace(pos, 1, "&lt;"); pos = user_message.find("<", pos + 4); }
    pos = user_message.find(">"); while(pos != std::string::npos) {user_message.replace(pos, 1, "&gt;"); pos = user_message.find(">", pos + 4); }
    pos = user_message.find("\""); while (pos != std::string::npos) {user_message.replace(pos, 1, "&quot;"); pos = user_message.find("\"", pos + 6); }
    pos = user_message.find("'"); while (pos != std::string::npos) {user_message.replace(pos, 1, "&#39;"); pos = user_message.find("'", pos + 5); }

    // Getting the current timestamp with chrono
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* ptm = std::localtime(&now_c);

    std::ostringstream timestamp_oss;
    timestamp_oss << std::put_time(ptm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp_str = timestamp_oss.str();

    // Open file in append mode
    std::ofstream outfile("submissions.txt", std::ios::app);
    if (outfile.is_open()) {
        outfile << "--- Submission at " << timestamp_str << " ---\n";
        outfile << "Message: " << user_message << "\n";
        outfile << "-----------------------------------------\n\n";
        outfile.close();
        std::cout << "Successfully saved submission to submissions.txt" << std::endl;
    } else {
        std::cerr << "Error: Unable to open submissions.txt for writing!" << std::endl;
    }

    //std::string response_body = "Data received:\n";
    //for (const auto& item : post_data) {
        //response_body += item.first + " = " + item.second + "\n";
    //}

    std::ostringstream html_response_oss;
    html_response_oss << R"(
    <!DOCTYPE html>
    <html>
    <head>
        <title>Submission Confirmation</title>
        <link rel="stylesheet" href="/static/style.css">
    </head>
    <body>
        <div class="container">
            <h1>Thank You for Your Submission!</h1>
            <p>Your message has been successfully received:</p>
            <p style="font-style: italic; color: #4CAF50; font-size: 1.1em; word-wrap: break-word; border: 1px dashed #ddd; padding: 10px; background-color: #f9f9f9;">"
    )" << user_message << R"( "</p>
            <p style="font-size: 0.9em; color: #777;">Submitted at: )" << timestamp_str << R"(</p>)" <<
            R"(
            <hr>
            <p>
                <a href="/submit_form">Submit another message</a> |
                <a href="/">Go to Home Page</a> |
                <a href="/view-submissions">View all submissions</a>
            </p>
        </div>
    </body>
    </html>
    )";

    std::string response_body = html_response_oss.str();

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " + std::to_string(response_body.length()) + "\r\n"
           "\r\n"
           + response_body;
}

const std::string NOT_FOUND_HTML = R"(
<!DOCTYPE html>
<html>
<head>
    <title>404 Not Found</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f4; color: #333; }
        .container {background-color: #fff; border-radius: 8px; box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); padding: 30px; display: inline-block; }
        h1 { color: #d9534f; }
        p { font-size: 1.1em; }
        a { color: #007bff; text-decoration: none; }
        a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <h1>404 Not Found</h1>
        <p>Oops! The page you are looking for could not be found.</p>
        <p>Please check the URL or return to the <a href="/">homepage</a>.</p>
    </div>
</body>
</html>
)";

std::string handleNotFoundRequest(const std::map<std::string, std::string>& headers) {
    return "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/html; charset=UTF-8\r\n"
           "Content-Length: " + std::to_string(NOT_FOUND_HTML.length()) + "\r\n"
           "Connection: close\r\n"
           "\r\n"
           + NOT_FOUND_HTML;
}

std::string handleViewSubmissionsRequest(const std::map<std::string, std::string>& headers) {
    std::cout << "Handling view submissions request..." << std::endl;

    std::string submissions_raw_content; // Renamed to avoid confusion with processed HTML content
    std::ifstream infile("submissions.txt");    // Opening the submissions file
    if (infile.is_open()) {
        std::string line;
        while (std::getline(infile, line)) {
            submissions_raw_content += line + "\n";     // Appending each line to string (+ newline)
        }
        infile.close();

        if (submissions_raw_content.empty()) {
            // Note: This string won't be used directly now, as we generate HTML for this case
            // submissions_raw_content = "No submissions to display yet. Submit a message from the form page!";
        }
        std::cout << "Successfully read submissions from submissions.txt" << std::endl;
    } else {        
        // In case the file cannot be opened
        submissions_raw_content = "Error:Unable to open submissions.txt! Please ensure the file exists and is accessible.";
        std::cerr << "Error: Unable to open submissions.txt for reading!" << std::endl;
    }

    std::ostringstream html_response_oss;
    html_response_oss << R"(
    <!DOCTYPE html>
    <html>
    <head>
        <title>View Submissions</title>
        <link rel="stylesheet" href="/static/style.css">
        <style>
            /* Your CSS styles here */
            .container {
                max-width: 800px;
                margin: 50px auto;
                padding: 20px;
                background-color: #fff;
                border-radius: 8px;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            }
            h1 {
                text-align: center;
                color: #333;
                margin-bottom: 30px;
            }
            .submission-entry {
                border: 1px solid #eee;
                padding: 15px;
                margin-bottom: 10px;
                background-color: #fcfcfc;
                border-radius: 5px;
                word-wrap: break-word; /* Ensure long words wrap */
            }
            .submission-date {
                font-size: 0.9em;
                color: #888;
                margin-bottom: 5px;
            }
            .submission-message {
                white-space: pre-wrap; /* Preserves whitespace and line breaks */
                font-family: monospace;
                background-color: #f0f0f0;
                padding: 10px;
                border-radius: 3px;
                word-wrap: break-word; /* Ensure long words wrap */
            }
            hr {
                border: 0;
                height: 1px;
                background: #ddd;
                margin: 20px 0;
            }
            p.links {
                text-align: center;
            }
            p.links a {
                margin: 0 10px;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>All Submissions</h1>
    )";

    std::istringstream raw_stream(submissions_raw_content);
    std::string line;
    std::string current_submission_date;
    std::string current_submission_message;
    bool in_message_content = false; // New flag to track if we're inside a message's content

    while (std::getline(raw_stream, line)) {
        // Trim potential trailing carriage return from getline, common with Windows files
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (startsWith(line, "--- Submission at ")) {
            // If we were parsing a message, finalize the previous entry
            if (in_message_content || !current_submission_message.empty()) {
                html_response_oss << "<div class=\"submission-entry\">"
                                  << "<div class=\"submission-date\">" << current_submission_date << "</div>"
                                  << "<div class=\"submission-message\">" << current_submission_message << "</div>"
                                  << "</div>";
                current_submission_message.clear(); // Clear for the next message
            }
            in_message_content = false; // Reset flag

            current_submission_date = line.substr(line.find(" at ") + 4); // Extract full date string
        } else if (startsWith(line, "Message: ")) {
            current_submission_message = line.substr(9) + "\n"; // Start collecting message content
            in_message_content = true; // Set flag: now collecting message lines
        } else if (startsWith(line, "-----------------------------------------")) {
            // End of an entry: if we were collecting a message, finalize it
            if (in_message_content || !current_submission_message.empty()) {
                html_response_oss << "<div class=\"submission-entry\">"
                                  << "<div class=\"submission-date\">" << current_submission_date << "</div>"
                                  << "<div class=\"submission-message\">" << current_submission_message << "</div>"
                                  << "</div>";
                current_submission_message.clear(); // Clear for the next message
            }
            in_message_content = false; // Message content block ended
        } else if (in_message_content) {
            // If we are currently inside a message content block, append this line
            current_submission_message += line + "\n";
        }
    }

    // After the loop, check if there's any pending message (e.g., file ends without '---')
    if (in_message_content || !current_submission_message.empty()) {
        html_response_oss << "<div class=\"submission-entry\">"
                          << "<div class=\"submission-date\">" << current_submission_date << "</div>"
                          << "<div class=\"submission-message\">" << current_submission_message << "</div>"
                          << "</div>";
    }

    // Handle empty submissions file/content case
    // This checks if any 'submission-entry' div was added
    if (submissions_raw_content.empty() || html_response_oss.str().find("<div class=\"submission-entry\">") == std::string::npos) {
        html_response_oss << "<p>No submissions to display yet. Submit a message from the form page!</p>";
    }

    html_response_oss << R"(
            <hr>
            <p class="links">
                <a href="/submit_form">Submit a new message</a> |
                <a href="/">Go to Home Page</a>
            </p>
        </div>
    </body>
    </html>
    )";
    
    std::string response_body = html_response_oss.str();

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html; charset=UTF-8\r\n"
           "Content-Length: " + std::to_string(response_body.length()) + "\r\n"
           "Connection: close\r\n"
           "\r\n"
           + response_body;
}

/////////////////////////////////////////////////
/////////  MAIN PROGRAM STARTS HERE   ///////////
/////////////////////////////////////////////////

// ||||||||||||||||||||||||||||||||||||||||||||||
// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

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
        std::vector<char> buffer(8192);     // <-- 1 KB buffer
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
                        
                        std::map<std::string, std::string> post_data = parseUrlEncodedFormData(request_body);

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
            } else if (method == "GET" && uri == "/view-submissions") {
                response = handleViewSubmissionsRequest(headers);
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