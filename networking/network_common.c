#include "network_common.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/_endian.h>
#include <sys/_types/_socklen_t.h>
#include <sys/_types/_ssize_t.h>
#include <sys/socket.h>
#include <unistd.h>

static int fill_sockaddr(struct sockaddr_in *addr, const char *ip_addr,
                         const int port) {
  memset(addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);

  if (inet_pton(AF_INET, ip_addr, &addr->sin_addr) <= 0) {
    perror("Invalid IP address");
    return -1;
  }
  return 0;
}

int create_send_socket(const char *ip_addr, const int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }

  struct sockaddr_in server_addr;
  if (fill_sockaddr(&server_addr, ip_addr, port) < 0) {
    close(sock);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Failed to create connection.");
    close(sock);
    return -1;
  }

  return sock;
}

int create_listening_socket(const char *ip_addr, const int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Failed to create socket");
    return -1;
  }

  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Failed to set SO_REUSEADDR");
    close(sock);
    return -1;
  }

  struct sockaddr_in listen_addr;
  socklen_t listen_addr_len = sizeof(listen_addr);
  if (fill_sockaddr(&listen_addr, ip_addr, port) < 0) {
    close(sock);
    return -1;
  }

  if (bind(sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
    perror("Failed to bind socket");
    close(sock);
    return -1;
  }

  if (listen(sock, 1) < 0) {
    perror("Failed to listen on socket");
    close(sock);
    return -1;
  }

  printf("Server listening on port %d\n", port);

  int client_fd =
      accept(sock, (struct sockaddr *)&listen_addr, &listen_addr_len);
  if (client_fd < 0) {
    perror("Failed to accept connection");
    close(sock);
    return -1;
  }

  printf("Client connected succesfully\n");

  return client_fd;
}

int send_data(int sock, char *data, int data_len) {
  ssize_t num_bytes_sent = send(sock, data, data_len, 0);
  if (num_bytes_sent < 0) {
    perror("Failed to send data");
  } else {
    printf("Send %zd bytes\n", num_bytes_sent);
  }

  return num_bytes_sent;
}

int recv_data(int sock, char *recv_buff, int recv_data_size) {
  ssize_t num_bytes_recv = recv(sock, recv_buff, recv_data_size, 0);
  if (num_bytes_recv < 0) {
    perror("Failed to receive data");
  } else {
    printf("Recv %zd bytes\n", num_bytes_recv);
  }

  return num_bytes_recv;
}
