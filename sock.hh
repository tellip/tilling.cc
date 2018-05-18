#pragma once

#include "main.hh"

namespace matrix_wm {
	auto sock = [&](const auto &callback) {
		auto sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (sockfd < 0) error("socket");

		sockaddr_in addr;
		bzero((char *) &addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(config::socket_host);
		addr.sin_port = htons(config::socket_port_base);
		if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) < 0) error("bind");

		callback(
				//breakListen
				[&]() {
					sendSock("-");
				},
				//listen
				[&](const CommandHandlers &handlers, const auto &callback) {
					auto thread_listen = std::thread([&]() {
						std::function<void()> iterate = [&]() {
							if (listen(sockfd, SOMAXCONN) < 0) error("listen");
							sockaddr_in addr_client;
							socklen_t len = sizeof(addr_client);
							auto sockfd_client = accept(sockfd, (sockaddr *) &addr_client, &len);
							if (sockfd_client < 0) error("accept");
							char buffer[256];
							bzero(buffer, sizeof(buffer));
							if (read(sockfd_client, buffer, sizeof(buffer)) < 0) error("read");
							close(sockfd_client);
							if (strcmp(buffer, "-")) {
								auto i = handlers.find(buffer);
								if (i != handlers.end()) {
									i->second();
								}
								iterate();
							}
						};
						iterate();
					});

					callback(
							[&](const auto &callback) {
								thread_listen.join();
								callback(
										[&]() {
											close(sockfd);
										}
								);
							}
					);
				}
		);
	};
}