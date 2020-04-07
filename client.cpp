#include <string.h>
#include <string>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>

using namespace std;

struct ComputeRequestInfo
{
	int			map_id;
	int			src_vertex_idx;
	long long	file_size;
};

struct CalculationResponseInfo
{
 	double		trans_time;
	int			vertex[10];
	int			min_path_dist[10];
	double  	prop_time[10];
	double  	end_to_end[10];
};

void show_results(CalculationResponseInfo);

CalculationResponseInfo received_info;

int main(int argc, const char* argv[]){

	int tcp_sockfd;
	struct sockaddr_in tcp_server_addr;
	struct addrinfo hints;
	ComputeRequestInfo cur_request;
	
	if (argc != 4) {
		fprintf(stderr, "Input Error\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	string input_fisrt = argv[1];
	char id = input_fisrt.c_str()[0];
	cur_request.map_id = id;
	cur_request.src_vertex_idx = atoi(argv[2]);
	cur_request.file_size = (long long)atof(argv[3]);

	// create tcp client;
	// reference: Beej Guide
	if ((tcp_sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Client TCP Socket Create Error");
		exit(1);
	}

	/*// reference: EE450 Fall 2019 Project
	socklen_t tcp_client_len = sizeof tcp_client_addr;

	// reference: Beej Guide
	if(bind(tcp_sockfd, (struct sockaddr*) &tcp_client_addr, tcp_client_len) == -1){
		perror("Client Bind Error");
		exit(1);
	}
	int getsock_check = getsockname(tcp_sockfd,(struct sockaddr*) &tcp_client_addr, &tcp_client_len);
	if (getsock_check == -1) {
		perror("Getsockname Error");
		exit(1);
	}*/
	
	memset(&tcp_server_addr, 0, sizeof tcp_server_addr);
	tcp_server_addr.sin_family = AF_INET;
	tcp_server_addr.sin_port = htons(24532);
	tcp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	printf("The client is up and running.\n");

	// do TCP connection
	// reference: Beej Guide
	if (connect(tcp_sockfd, (struct sockaddr*)&tcp_server_addr, sizeof tcp_server_addr) == -1) {
		perror("Connection Error");
		close(tcp_sockfd);
		exit(1);
	}

	// send message to aws
	// cur_request is the buffer that read info into
	// string send_buf;
	char send_buf[1024];
	memset(send_buf, 0, 1024);
	memcpy(send_buf, &cur_request, sizeof(cur_request));
	if (send(tcp_sockfd, send_buf, sizeof(send_buf), 0) == -1) {
		perror("Message Send Error");
		close(tcp_sockfd);
		exit(1);
	}
	printf("The client has sent query to AWS using TCP: start vertex <%d>; map id <%c>; file size <%lld> bits.\n", cur_request.src_vertex_idx, (char)cur_request.map_id, cur_request.file_size);

	char recv_buf[1024];
	memset(recv_buf, 'z', 1024);
	if (recv(tcp_sockfd, recv_buf, sizeof recv_buf, 0) == -1) {
		perror("Receive Error");
		close(tcp_sockfd);
		exit(1);
	}
	memset(&received_info, 0, sizeof received_info);
	memcpy(&received_info, recv_buf, sizeof received_info);

	show_results(received_info);
	close(tcp_sockfd);
}

/* Show the results from aws */
void show_results(CalculationResponseInfo received_info) {
	// print onscreen message
	printf("The client has received results from AWS:\n");
	printf("------------------------------------------------------------------------------\n");
	printf("%-8s\t%-8s\t%-8s\t%-8s\t%-8s\n", "Destination", "Min Length", "Tt", "Tp", "Delay");
	printf("------------------------------------------------------------------------------\n");
    double trans_time = received_info.trans_time;
	for (int idx = 0; idx < 10; idx++) {
		int cur_des = received_info.vertex[idx];
		int cur_min_len = received_info.min_path_dist[idx];
		double prop_time = received_info.prop_time[idx];
        double end_to_end = received_info.end_to_end[idx];
		if (cur_min_len > 0) {
			printf("%-8d\t%-8d\t%-8.2f\t%-8.2f\t%-8.2f\n", cur_des, cur_min_len ,trans_time, prop_time, end_to_end);
		}
	}
	printf("------------------------------------------------------------------------------\n");
}
