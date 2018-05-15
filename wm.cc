#include <cstring>
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "config.hc"

namespace matrix_wm {
    void error(const char *const &);

    class Socket {
        virtual ~Socket() = 0;

        static int _sockfd;
        static sockaddr_in _addr;
    public:
        static bool initialize();

        static void loop();
    };

    class WM {
        virtual ~WM() = 0;

        static Display *_d;
        static Atom _xia_wmp, _xia_wmdw;
    public:
        static bool initialize();
    };
}

int main(int argc, char *argv[]) {
    if (matrix_wm::Socket::initialize() && matrix_wm::WM::initialize()) {
        matrix_wm::Socket::loop();
    }
}

namespace matrix_wm {
    void error(const char *const &fn) {
        std::cout << "ERROR on " << fn << '\n';
    }

    typeof(Socket::_sockfd) Socket::_sockfd;
    typeof(Socket::_addr) Socket::_addr;

    bool Socket::initialize() {
        static bool called = false, rtn;
        if (!called) {
            called = true;

            _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
            if (_sockfd < 0) {
                error("socket");
                rtn = false;
            } else {
                bzero((char *) &_addr, sizeof(_addr));
                _addr.sin_family = AF_INET;
                _addr.sin_addr.s_addr = INADDR_ANY;
                _addr.sin_port = htons(Config::socket_port_base);
                if (bind(_sockfd, (sockaddr *) &_addr, sizeof(_addr)) < 0) {
                    error("bind");
                    rtn = false;
                } else rtn = true;
            }
        }
        return rtn;
    }

    void Socket::loop() {
        static bool called = false;
        if (!called) {
            called = true;

            std::function<void()> iterate = [&]() {
                if (listen(_sockfd, SOMAXCONN) < 0) {
                    error("listen");
                } else {
                    sockaddr_in addr;
                    socklen_t len = sizeof(addr);
                    auto sockfd = accept(_sockfd, (sockaddr *) &addr, &len);
                    if (sockfd < 0) {
                        error("accept");
                    } else {
                        char buffer[256];
                        bzero(buffer, sizeof(buffer));
                        auto n = read(sockfd, buffer, sizeof(buffer));
                        if (n < 0) {
                            error("read");
                        } else {
                            std::cout << "from client: " << buffer << '\n';
                        }
                    }
                }
            };
            iterate();
        }
    }

    typeof(WM::_d) WM::_d;
    typeof(WM::_xia_wmp) WM::_xia_wmp;
    typeof(WM::_xia_wmdw) WM::_xia_wmdw;

    bool WM::initialize() {
        static bool called = false, rtn;
        if (!called) {
            called = true;

            _d = XOpenDisplay(NULL);
            _xia_wmp = XInternAtom(_d, "WM_PROTOCOLS", False);
            _xia_wmdw = XInternAtom(_d, "WM_DELETE_WINDOW", False);
            rtn = true;
        }
        return rtn;
    }
}