#pragma once

#include "main.hh"

namespace matrix_wm {
	auto sock = [&](const auto &callback) {
		auto sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (sock_server < 0) error("socket");
		auto sock_server_closed = false;
		try {
			sockaddr_in sai_server;
			bzero((char *) &sai_server, sizeof(sai_server));
			sai_server.sin_family = AF_INET;
			sai_server.sin_addr.s_addr = inet_addr(config::socket_host);
			sai_server.sin_port = htons(socket_port);
			if (bind(sock_server, (sockaddr *) &sai_server, sizeof(sai_server)) < 0) error("bind");

			callback(
					//breakListen
					[&]() {
						sendSock("-");
					},
					//listenCommands
					[&](const CommandHandlers &handlers, const auto &callback) {
						auto thread_listen = std::thread([&]() {
							std::function<void()> iterate = [&]() {
								if (listen(sock_server, SOMAXCONN) < 0) error("listen");
								sockaddr_in sai_client;
								socklen_t len = sizeof(sai_client);
								auto sock_client = accept(sock_server, (sockaddr *) &sai_client, &len);
								if (sock_client < 0) error("accept");
								auto sock_client_closed = false;
								try {
									char buffer[256];
									bzero(buffer, sizeof(buffer));
									if (read(sock_client, buffer, sizeof(buffer)) < 0) error("read");
									sock_client_closed = true;
									close(sock_client);
									if (strcmp(buffer, "-")) {
										auto i = handlers.find(buffer);
										if (i != handlers.end()) {
											i->second();
										}
										iterate();
									}
								} catch (...) {
									if (!sock_client_closed) close(sock_client);
									throw true;
								}
							};
							iterate();
						});

						callback(
								//joinThread
								[&](const auto &callback) {
									thread_listen.join();
									callback(
											[&]() {
												sock_server_closed = true;
												close(sock_server);
											}
									);
								}
						);
					}
			);
		} catch (...) {
			if (!sock_server_closed) close(sock_server);
			throw true;
		}
	};
}