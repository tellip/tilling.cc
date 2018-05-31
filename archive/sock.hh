#pragma once

#include "main.hh"

namespace wm {
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
			if (listen(sock_server, SOMAXCONN) < 0) error("listen");

			bool listening = false;
			callback(
					//listenCommands
					[&](const CommandHandlers &command_handlers, const EventHandlers &event_handlers, const auto &callback) {
						if (!listening) {
							listening = true;
							auto thread = std::thread([&]() {
								while (listening) {
									sockaddr_in sai_client;
									socklen_t len = sizeof(sai_client);
									auto sock_client = accept(sock_server, (sockaddr *) &sai_client, &len);
									if (sock_client < 0) error("accept");
									char buffer[256];
									try {
										bzero(buffer, sizeof(buffer));
										if (read(sock_client, buffer, sizeof(buffer)) < 0) error("read");
									} catch (...) {
										close(sock_client);
										throw true;
									}
									close(sock_client);
									if (strcmp(buffer, "-")) {
										if (buffer[0] == 'c' && buffer[1] == '-') {
											auto i = command_handlers.find(&buffer[2]);
											if (i != command_handlers.end()) i->second();
										} else if (buffer[0] == 'e' && buffer[1] == '-') {
											auto event = event_queue.front();
											event_queue.pop();
											auto i = event_handlers.find(event.type);
											if (i != event_handlers.end()) i->second(event);
										}
									}
								}
							});

							callback(
									//joinThread
									[&](const auto &callback) {
										thread.join();
										callback(
												[&]() {
													if (!sock_server_closed) {
														sock_server_closed = true;
														close(sock_server);
													}
												}
										);
									}
							);
						}
					},
					//breakListen
					[&]() {
						if (listening) {
							listening = false;
							sendSock("-");
						}
					}
			);
		} catch (...) {
			if (!sock_server_closed) {
				sock_server_closed = true;
				close(sock_server);
			}
			throw true;
		}
	};
}