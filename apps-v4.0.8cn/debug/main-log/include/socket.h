int createsocket(void);
int connect_socket(int fd_sock);
int send_record(int fd_sock,char *sendbuff);
void close_socket(int fd_sock);
void get_ip(char ip_buff[16]);
void get_ip_of_all(char ip_buff[2][20]);
