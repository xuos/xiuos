#include <rtthread.h>
#ifdef PKG_USING_RW007
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#include <sys/socket.h>
static const char send_data[] = "This is TCP Client from AIIT"; 
void rw007_test(int argc, char **argv)
{
    int ret;
    char *recv_data;
    struct hostent *host;
    int sock, bytes_received;
    struct sockaddr_in server_addr;
    const char *url;
    int port;
    extern rt_bool_t rt_wlan_is_connected(void);
    if (rt_wlan_is_connected() != 1)
    {
        printf("Please connect a wifi firstly\n");
        return;
    }
    if (argc < 3)
    {
        printf("Usage: rw007 URL PORT\n");
        printf("Like: rw007 192.168.12.44 5000\n");
        return ;
    }

    url = argv[1];
    port = strtoul(argv[2], 0, 10);
    host = gethostbyname(url);
    recv_data = rt_malloc(1024);
    if (recv_data == RT_NULL)
    {
        printf("No memory\n");
        return;
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Socket error\n");
        rt_free(recv_data);
        return;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Connect fail!\n");
        closesocket(sock);
        rt_free(recv_data);
        return;
    }
    else
    {
        printf("Connect successful\n");
    }
    while (1)
    {
        bytes_received = recv(sock, recv_data, 1024 - 1, 0);
        if (bytes_received < 0)
        {
            closesocket(sock);
            printf("\nreceived error,close the socket.\r\n");
            rt_free(recv_data);
            break;
        }
        else if (bytes_received == 0)
        {
            closesocket(sock);
            printf("\nreceived error,close the socket.\r\n");
            rt_free(recv_data);
            break;
        }
        recv_data[bytes_received] = '\0';
        if (strncmp(recv_data, "q", 1) == 0 || strncmp(recv_data, "Q", 1) == 0)
        {
            closesocket(sock);
            printf("\n got a 'q' or 'Q',close the socket.\r\n");
            rt_free(recv_data);
            break;
        }
        else
        {
            printf("\nReceived data = %s ", recv_data);
        }
        ret = send(sock, send_data, strlen(send_data), 0);
        if (ret < 0)
        {
            closesocket(sock);
            printf("\nsend error,close the socket.\r\n");
            rt_free(recv_data);
            break;
        }
        else if (ret == 0)
        {
            printf("\n Send warning,send function return 0.\r\n");
        }
    }
    return;
}
MSH_CMD_EXPORT(rw007_test, a tcp client sample);
#endif
