# webserv

Welcome to **webserv**, a custom web server implementation designed as part of the 42 School curriculum. This project demonstrates an in-depth understanding of network programming, HTTP protocols, and server architecture while adhering to the rigorous standards of 42 School.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Usage](#usage)
- [Configuration](#configuration)
- [Technologies Used](#technologies-used)
- [Contributors](#contributors)

---

## Overview

**webserv** is a lightweight HTTP server built from scratch in C++ (or your chosen language) to gain a comprehensive understanding of web server operations. The server handles HTTP requests and responses, supports configuration files, and is designed to be modular and robust.

Key objectives of this project:
- Understand and implement HTTP/1.1.
- Gain hands-on experience with socket programming.
- Manage client connections efficiently.
- Develop a fully functional and configurable web server.

---

## Features

- Supports HTTP methods: `GET`, `POST`, `DELETE` (and others if implemented).
- Custom configuration file to define server behavior.
- Static file hosting.
- CGI support for dynamic content generation.
- Custom error pages.
- Connection handling with a focus on scalability and performance.
- HTTP/1.1 compliance.

---

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed on your system:

- A C++17 (or later) compliant compiler (e.g., `g++`, `clang++`).
- `make` (or an equivalent build tool).
- Docker

### Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/t-pereira06/42_webserv.git
   cd webserv
   ```

2. Build the project:
   ```bash
   make
   ```

3. Ensure the binary is created successfully:
   ```bash
   ls
   ./webserv
   ```

### Usage

Run the server with a configuration file:
```bash
./webserv [config_file]
```

Example:
```bash
./webserv nginx2.conf
```

---

## Configuration

The server behavior is defined using a configuration file. Here’s an example structure:

```conf
server {
    listen 8080;
    server_name localhost;

    root /var/www/html;
    index index.html;

    location /api {
        methods GET POST DELETE;
        root /var/www/html/api;
    }

    error_page 404 /404.html;
}
```

- **`listen`**: Specifies the port to bind the server.
- **`server_name`**: Domain or IP for the server.
- **`root`**: Document root for serving files.
- **`methods`**: Allowed HTTP methods for specific locations.
- **`error_page`**: Custom error page paths.

---

## Technologies Used

- **Language**: C++
- **Networking**: BSD sockets (or equivalent platform-specific implementation)
- **HTTP/1.1 Protocol**: Request parsing and response generation
- **Multi-processing/Threading**: To handle multiple connections (if applicable)
- **CGI (Common Gateway Interface)**: For executing scripts and dynamic content

---

## Contributors

I would like to thank to my amazing colleagues who worked on this project with me. Your dedication, collaboration, and expertise made this journey sucessful and very enjoyable. Thank you for your hard work and teamwork!
- [José Gonçalves](https://github.com/ZPedro99)
- [Luís Pereira](https://github.com/lubuper)
