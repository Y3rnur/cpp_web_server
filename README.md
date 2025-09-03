# HTTP Web Server written in C++

## Description
This project is a basic HTTP web server implemented from scratch in C++. It's designed to demonstrate fundamental concepts of network programming, HTTP protocol parsing, and serving static and dynamic web content. This server is part of my journey to deeply understand how web services work at a low level.

As of September 2025, this server now also includes persistent data storage using a SQLite3 database. I will use it to learn and interact with database systems.

## Features
* **Socket Programming:** Using TCP sockets to implement basic server-client communication
* **HTTP Request Parsing:**
    * Parses the HTTP request line (Method, URI, HTTP Version).
    * Parses HTTP request headers into a key-value map.
* **Routing:** Handles different URIs and HTTP methods.
* **Static File Serving:** Serves HTML, CSS, Javascript, and image files.
* **MIME Type Detection:** Automatically determines the correct `Content-Type` for served static files.
* **Form Data Handling:** Processes URL-encoded form data submitted via POST requests.
* **Basic API Endpoint:** Serves a simple JSON response for `/api/time`.
* **Modular Design:** Uses separate C++ functions to handle different request types and improve code organization.
* **Database Integration:** Implements a SQLite3 database for persistent note storage.

## Getting Started

## Prerequisites
To compile and run this server, you will need:
* A C++ compiler like g++, etc.
* `make`, for compiling the project using the Makefile
* Standard C++ libraries.
* Apart from it, the SQLite3 as CLI tool. On Linux, you can install it with: "sudo apt install sqlite3". It is useful for manually inserting, deleting, reading the contents of your database file.
* Linux/Unix-like operating system

## How to Compile
After cloning the repository, you should:
Compile the server using the provided Makefile. This will compile both C++ source and the SQLite3 C source code.
For this, you can run "make" in the terminal, while being in the project's root folder.
After the compilation, you can start the server by executing the "server" file like below in the terminal:
"./server"
The server will be running on "http://localhost:8080".
You can type the above URL into your preferred browser's address bar.

## Checking Database contents and its work
Right now, I am working on correct database integration into my C++ web server and I still need some time to add main functionalities. The only working feature now is adding note contents into database. You can check the contents of your submitted note by above mentioned "sqlite3" CLI tool in Linux terminal.

## Further updates
I will try to update this repository sometimes, so other functionalities will be available in future.