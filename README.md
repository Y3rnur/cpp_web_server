# HTTP Web Server written in C++

## Description
This project is a basic HTTP web server implemented from scratch in C++. It's designed to demonstrate fundamental concepts of network programming, HTTP protocol parsing, and serving static and dynamic web content. This server is part of my journey to deeply understand how web services work at a low level.

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

## Getting Started

## Prerequisites
To compile and run this server, you will need:
* A C++ compiler like g++, etc.
* `make`, if you use a Makefile.
* Standard C++ libraries.
* Linux/Unix-like operating system

## How to Compile
Navigate to the project root directory in your terminal and compile the `starter_server.cpp` file