#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <X11/Xlib.h>

#include "config.hh"

namespace matrix_wm {
	auto error = [&](const char *const &fn) {
		std::cout << "ERROR on " << fn << '\n';
		throw true;
	};

	auto sendSock = [&](const char *const &msg) {
		auto sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (sockfd < 0) matrix_wm::error("socket");
		else {
			sockaddr_in serv_addr;
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = inet_addr(config::socket_host);
			serv_addr.sin_port = htons(config::socket_port_base);
			if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) matrix_wm::error("connect");
			else {
				if (write(sockfd, msg, strlen(msg)) < 0) matrix_wm::error("write");
			}
			close(sockfd);
		}
	};
}

#include "sock.hh"
#include "conn.hh"
//#include "layout.hh"