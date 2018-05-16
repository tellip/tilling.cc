#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <X11/Xlib.h>

#include "config.hc"

namespace matrix_wm {
	Display *display;
	bool looping;

	inline void error(const char *const &fn) {
		std::cout << "ERROR on " << fn << '\n';
		throw true;
	}

	inline void sendSocket(const char *const &msg) {
		auto sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (sockfd < 0) matrix_wm::error("socket");
		else {
			sockaddr_in serv_addr;
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = inet_addr(matrix_wm::Config::socket_host);
			serv_addr.sin_port = htons(matrix_wm::Config::socket_port_base);
			if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) matrix_wm::error("connect");
			else {
				if (write(sockfd, msg, strlen(msg)) < 0) matrix_wm::error("write");
			}
			close(sockfd);
		}
	}
}