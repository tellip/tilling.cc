#include <X11/Xlib.h>
#include "helper.hc"
#include "layout.hc"

namespace matrix_wm {
	class Socket {
		virtual ~Socket() = 0;

		static int _sockfd;
		static sockaddr_in _addr;
	public:
		static void initialize();

		static void loop();

		static void clear();
	};
}

int main(int argc, char *argv[]) {
	try {
		matrix_wm::Socket::initialize();
		try {
			matrix_wm::display = XOpenDisplay(NULL);
			if (matrix_wm::display == NULL) matrix_wm::error("XOpenDisplay");
			XSelectInput(matrix_wm::display, XDefaultRootWindow(matrix_wm::display), SubstructureNotifyMask);
			matrix_wm::Layout::initialize();

			std::thread t_socket(matrix_wm::Socket::loop);

			try {
				matrix_wm::looping = true;
				while (matrix_wm::looping) {
					XEvent event;
					if (XCheckMaskEvent(matrix_wm::display, SubstructureNotifyMask, &event)) {
						auto i = matrix_wm::Layout::event_handlers.find(event.type);
						if (i != matrix_wm::Layout::event_handlers.end()) i->second();
					}
				}
			} catch (...) {}
			matrix_wm::sendSocket("-");

			t_socket.join();
		} catch (...) {}
		XCloseDisplay(matrix_wm::display);
	} catch (...) {}
	matrix_wm::Socket::clear();
}

namespace matrix_wm {
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
				std::function<void()> iterate = [&]() {
					if (listen(_sockfd, SOMAXCONN) < 0) error("listen");
					sockaddr_in addr;
					socklen_t len = sizeof(addr);
					auto sockfd = accept(_sockfd, (sockaddr *) &addr, &len);
					if (sockfd < 0) error("accept");
					char buffer[256];
					bzero(buffer, sizeof(buffer));
					if (read(sockfd, buffer, sizeof(buffer)) < 0) error("read");
					if (strcmp(buffer, "-")) {
						auto i = Layout::commands.find(buffer);
						if (i != Layout::commands.end()) {
							i->second();
						}
						iterate();
					}
				};
				iterate();
			} catch (...) {
				looping = false;
			}
		}
	}

	void Socket::clear() {
		close(_sockfd);
	}
}