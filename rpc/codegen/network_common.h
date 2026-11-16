#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H
#define EXIT_SUCCESS 0

int create_send_socket(const char *ip_addr, const int port);
int create_listening_socket(const char *ip_addr, const int port);
int send_data(int sock, char *data, int data_len);
int recv_data(int sock, char *buff, int recv_data_size);

#endif // !NETWORK_COMMON_H
