#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>
using namespace std;

void print_hex(unsigned char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
        if ((i + 1) % 16 == 0)
            std::cout << std::endl;
    }
    std::cout << std::dec << std::endl;
}

int main() {
    int socket_file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_file_descriptor == -1) {
        perror("failed to create socket");
        return 1;
    }
    
    struct sockaddr_in server_address_info;
    server_address_info.sin_family = AF_INET;
    const int DNS_PORT = 53;
    in_port_t dns_port_network_byte_ordered = htons(DNS_PORT);
    server_address_info.sin_port = dns_port_network_byte_ordered;
    const char* GOOGLE_DNS_IP = "8.8.8.8";
    int did_ip_binary_conversion_succeed = inet_pton(AF_INET, GOOGLE_DNS_IP, &server_address_info.sin_addr);
    if(did_ip_binary_conversion_succeed == 0) {
        cerr << "invalid address format" << endl;
        close(socket_file_descriptor);
        return 1;
    }
    if(did_ip_binary_conversion_succeed == -1) {
        cerr << "address conversion failed" << endl;
        close(socket_file_descriptor);
        return 1;
    }
    
    unsigned char query[] = {
        0x00, 0x01,
        0x01, 0x00,
        0x00, 0x01,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x03, 'w', 'w', 'w',
        0x07, 'n', 'e', 't', 'f', 'l', 'i', 'x',
        0x03, 'c', 'o', 'm',
        0x00,
        0x00, 0x01,
        0x00, 0x01
    };
    
    ssize_t num_bytes_sent = sendto(socket_file_descriptor, query, sizeof(query), 0, (struct sockaddr*)&server_address_info, sizeof(server_address_info));
    if(num_bytes_sent == -1) {
        perror("failed to send query");
        close(socket_file_descriptor);
        return 1;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int did_set_timeout_succeed = setsockopt(socket_file_descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (did_set_timeout_succeed < 0) {
        perror("setting timeout on socket failed");
        close(socket_file_descriptor);
        return 1;
    }
    
    unsigned char received_response_buffer[512];
    socklen_t server_address_len = sizeof(server_address_info);
    ssize_t num_of_bytes_received = recvfrom(socket_file_descriptor, received_response_buffer, sizeof(received_response_buffer), 0, (struct sockaddr*)&server_address_info, &server_address_len);
    if(num_of_bytes_received == -1) {
        perror("failed to recieve response");
    }
    else {
        cout << "received " << num_of_bytes_received << " bytes: " << endl;
        print_hex(received_response_buffer, num_of_bytes_received);
    }
    
    close(socket_file_descriptor);
    
    return 0;
}
