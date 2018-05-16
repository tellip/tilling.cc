#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <X11/Xlib.h>

#include "config.hc"
#include "helper.hc"

namespace matrix_wm {
	class Socket {
		virtual ~Socket() = 0;

		static bool _looping;
		static int _sockfd;
		static sockaddr_in _addr;
	public:
		static void initialize();

		static void loop();

		static void stop();

		static void clear();
	};

	class WM {
		virtual ~WM() = 0;

		static Display *_display;
		static Atom _xia_protocols, _xia_delete_window;
	public:
		static void initialize();

		static void loop();

		static void stop();

		static void clear();
	};
}

int main(int argc, char *argv[]) {
	try {
		matrix_wm::Socket::initialize();
		try {
			matrix_wm::WM::initialize();

			std::thread t_socket(matrix_wm::Socket::loop);
			std::thread t_wm(matrix_wm::WM::loop);

			t_socket.join();
			t_wm.join();
		} catch (...) {}
		matrix_wm::WM::clear();
	} catch (...) {}
	matrix_wm::Socket::clear();
}

namespace matrix_wm {
	typeof(Socket::_looping) Socket::_looping;
	typeof(Socket::_sockfd) Socket::_sockfd;
	typeof(Socket::_addr) Socket::_addr;

	void Socket::initialize() {
		static bool called = false;
		if (!called) {
			called = true;

			_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (_sockfd < 0) error("socket");
			bzero((char *) &_addr, sizeof(_addr));
			_addr.sin_family = AF_INET;
			_addr.sin_addr.s_addr = inet_addr(Config::socket_host);
			_addr.sin_port = htons(Config::socket_port_base);
			if (bind(_sockfd, (sockaddr *) &_addr, sizeof(_addr)) < 0) error("bind");
		}
	}

	void Socket::loop() {
		static bool called = false;
		if (!called) {
			called = true;
			try {
				_looping = true;
				std::function<void()> iterate = [&]() {
					if (listen(_sockfd, SOMAXCONN) < 0) error("listen");
					sockaddr_in addr;
					socklen_t len = sizeof(addr);
					auto sockfd = accept(_sockfd, (sockaddr *) &addr, &len);
					if (sockfd < 0) error("accept");
					char buffer[256];
					bzero(buffer, sizeof(buffer));
					if (read(sockfd, buffer, sizeof(buffer)) < 0) error("read");
					std::cout << "from client: " << buffer << '\n';
					iterate();
				};
				iterate();
			} catch (...) {
				_looping = false;
			}
		}
	}

	void Socket::stop() {

	}

	void Socket::clear() {
		close(_sockfd);
	}

	typeof(WM::_display) WM::_display;
	typeof(WM::_xia_protocols) WM::_xia_protocols;
	typeof(WM::_xia_delete_window) WM::_xia_delete_window;

	void WM::initialize() {
		static bool called = false;
		if (!called) {
			called = true;

			_display = XOpenDisplay(NULL);
			if (_display == NULL) error("XOpenDisplay");
			_xia_protocols = XInternAtom(_display, "WM_PROTOCOLS", False);
			_xia_delete_window = XInternAtom(_display, "WM_DELETE_WINDOW", False);
			if (_xia_protocols == None || _xia_delete_window == None) error("XInternAtom");
			XSelectInput(_display, RootWindow(_display, DefaultScreen(_display)), SubstructureNotifyMask);
		}
	}

	void WM::loop() {
		static bool called = false;
		if (!called) {
			called = true;
			try {
				std::function<void()> iterate = [&]() {

				};
				iterate();
			} catch (...) {}
		}
	}

	void WM::clear() {
		XCloseDisplay(_display);
	}
}