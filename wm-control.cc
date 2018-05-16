#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

#include "config.hc"
#include "helper.hc"

int main(int argc, char *argv[]) {
    for (auto i = 0; i < argc; i++) {
        auto arg = argv[i];
        if (strlen(arg) > 2 && arg[0] == '-' && arg[1] == '-' && arg[3] != '\n') {
            auto msg = &arg[2];

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
    return 0;
};