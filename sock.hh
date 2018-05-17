#pragma once

#include "main.hh"

namespace matrix_wm {
	auto sock = [&]() {
		auto sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (sockfd < 0) error("socket");

		sockaddr_in addr;
		bzero((char *) &addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(config::socket_host);
		addr.sin_port = htons(config::socket_port_base);
		if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) < 0) error("bind");

		//serverListen
		return [&](const std::unordered_map<
				std::string,
				std::function<void()>
		> &handlers) {
			auto thread_listen = std::thread([&]() {
				std::function<void()> iterate = [&]() {
					if (listen(sockfd, SOMAXCONN) < 0) error("listen");
					sockaddr_in addr1;
					socklen_t len = sizeof(addr1);
					auto sockfd1 = accept(sockfd, (sockaddr *) &addr1, &len);
					if (sockfd1 < 0) error("accept");
					char buffer[256];
					bzero(buffer, sizeof(buffer));
					if (read(sockfd1, buffer, sizeof(buffer)) < 0) error("read");
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

			//breaskListen
			return [&]() {
				sendSock("-");

				//joinThread
				return [&]() {
					thread_listen.join();
				};
			};
		};
	};
}