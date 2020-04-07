#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <string>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <map>

using namespace std;

#define AWS_UDP_PORT 23532
#define AWS_TCP_CLIENT_PORT 24532
#define SERVERA_PORT 21532
#define SERVERB_PORT 22532

struct ComputeRequestInfo
{
	int			map_id;
	int			src_vertex_idx;
	long long	file_size;
};

struct PathForwardInfo
{
	int			map_id;
	int			src_vertex_idx;
};

struct PathResponseInfo
{
	int			min_path_vertex[10];
	int 		min_path_dist[10];
	double 		prop_speed;
	double 		tran_speed;
};

struct CalculationRequestInfo
{
	int			min_path_vertex[10];
	int			min_path_dist[10];
	double		prop_speed;
	double		tran_speed;
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

void create_and_bind_udp_socket();

void create_and_bind_tcp_client();

void send_to_serverA();

void receive_from_serverA();

void send_to_serverB();

void receive_from_serverB();

int udp_sockfd, tcp_sockfd_client;
struct sockaddr_in udp_server_addr, udp_client_addr, tcp_server_addr, tcp_cliser_addr;
ComputeRequestInfo input_msg;
PathForwardInfo forward_msg;
PathResponseInfo received_buff;
CalculationRequestInfo cal_request;
CalculationResponseInfo received_delay;

int main(int argc, const char* argv[]){
	create_and_bind_udp_socket();
	create_and_bind_tcp_client();

	printf("The AWS is up and running.\n");

	// listen on the socket just created for tcp client
	listen(tcp_sockfd_client, 5);

	// accept a call
	while (true) {
		// accept an incoming tcp connection
		// reference: Beej Guide
    		socklen_t tcp_client_addr_len = sizeof tcp_cliser_addr;
    		int new_tcp_sockfd = accept(tcp_sockfd_client, (struct sockaddr *)&tcp_cliser_addr, &tcp_client_addr_len);

		// check whether accept successfully
		if(new_tcp_sockfd == -1 ){
			perror("Accept Error");
			exit(1);
		}

		// receive message from the client and put it into input_msg
		// reference: Beej Guide
		char recv_buf1[1024];
		memset(recv_buf1, 'z', 1024);
		int num_of_bytes_read = recv(new_tcp_sockfd, recv_buf1, sizeof recv_buf1, 0);
		if(num_of_bytes_read < 0) {
			perror("Reading Stream Message Error");
			exit(1);
		} else if (num_of_bytes_read == 0) {
			printf("Connection Has Ended\n");
		}

		memset(&input_msg, 0, sizeof input_msg);
		memcpy(&input_msg, recv_buf1, sizeof recv_buf1);

		socklen_t tcp_length = sizeof tcp_cliser_addr;
		getsockname(tcp_sockfd_client,(struct sockaddr*) &tcp_cliser_addr, &tcp_length);
		// print onscreen message
		printf("The AWS has received map ID <%c>, start vertex <%d> and file size <%lld> bits from the client using TCP over port <%d>.\n", (char)input_msg.map_id, input_msg.src_vertex_idx, input_msg.file_size, ntohs(tcp_cliser_addr.sin_port));	

		// send message to serverA
		send_to_serverA();

		// receive from serverA
		receive_from_serverA();

		// construct the information which will be sent to serverB
		cal_request.prop_speed = received_buff.prop_speed;
		cal_request.tran_speed = received_buff.tran_speed;
		cal_request.file_size = input_msg.file_size;

		for (int idx = 0; idx < 10; idx++) {
			cal_request.min_path_vertex[idx] = received_buff.min_path_vertex[idx];
			cal_request.min_path_dist[idx] = received_buff.min_path_dist[idx];
		}

		// send request to serverB
		send_to_serverB();

		// receive from serverB
		receive_from_serverB();

		char send_buf3[1024];
		memset(send_buf3, 0, 1024);
		memcpy(send_buf3, &received_delay, sizeof received_delay);
		int sent = sendto(new_tcp_sockfd, send_buf3, sizeof send_buf3, 0, (struct sockaddr *) &tcp_cliser_addr, sizeof tcp_cliser_addr);
		// check whether send successfully
		if (sent == -1) {
			perror("Connection Error");
			close(tcp_sockfd_client);
			exit(1);
		}
		// print onscreen message
		printf("The AWS has sent calculated delay to client using TCP over port <%d>.\n", ntohs(tcp_cliser_addr.sin_port));
	}
	close(tcp_sockfd_client);
	return 0;
}

/* Create and bind UDP socket */
void create_and_bind_udp_socket() {
	// create a udp socket
	// reference: Beej Guide
	if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("AWS UDP Socket Create Error");
		exit(1);
	}

	memset(&udp_server_addr, 0, sizeof udp_server_addr);
	udp_server_addr.sin_family = AF_INET;
	udp_server_addr.sin_port = htons(AWS_UDP_PORT);
	udp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// bind the created udp socket with udp_serv_addr
	if (bind(udp_sockfd, (struct sockaddr*) &udp_server_addr, sizeof udp_server_addr) == -1) {
		perror("AWS UDP Bind Error");
    		exit(1);
    	}
}

/* Create and bind tcp socket of client */
void create_and_bind_tcp_client() {
	// create a socket
	// reference: Beej Guide
	if ((tcp_sockfd_client = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("AWS TCP Client Socket Create Error");
		exit(1);
	}

	memset(&tcp_server_addr, 0, sizeof tcp_server_addr);
	tcp_server_addr.sin_family = AF_INET;
	tcp_server_addr.sin_port = htons(AWS_TCP_CLIENT_PORT);
	tcp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// bind the created socket with tcp server addr
	if(bind(tcp_sockfd_client, (struct sockaddr*) &tcp_server_addr, sizeof tcp_server_addr) == -1)
	{
       	perror("AWS TCP Client Bind Error");
        exit(1);
    }
}

/* Send message to serverA */
void send_to_serverA() {
	// build message which will be sent to server A
	forward_msg.map_id = input_msg.map_id;
	forward_msg.src_vertex_idx = input_msg.src_vertex_idx;

	char send_buf1[1024];
	memset(send_buf1, 0, 1024);
	memcpy(send_buf1, &forward_msg, sizeof forward_msg);

	// send message to serverA via udp
	memset(&udp_server_addr,0, sizeof udp_server_addr);
	udp_server_addr.sin_family = AF_INET;
	udp_server_addr.sin_port = htons(SERVERA_PORT);
	udp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	sendto(udp_sockfd, (void*) &send_buf1, sizeof send_buf1, 0, (struct sockaddr *) &udp_server_addr, sizeof udp_server_addr);
	
	// print onscreen message
	printf("The AWS has sent Map ID <%c> and starting vertex <%d> to server A using UDP over port <%d>.\n", (char)forward_msg.map_id, forward_msg.src_vertex_idx, AWS_UDP_PORT);
}

/* Receive results from serverA */
void receive_from_serverA() {
	// build the container that will receive the msg from A and receive from A
	socklen_t server_udp_len = sizeof udp_client_addr;
	char recv_buf2[1024];
	memset(recv_buf2, 'z', 1024);
	recvfrom(udp_sockfd, recv_buf2, 1024, 0, (struct sockaddr*)&udp_client_addr, &server_udp_len);
	memset(&received_buff, 0, sizeof received_buff);
	memcpy(&received_buff, recv_buf2, sizeof received_buff);
	// output onscreen message
	printf("The AWS has received shortest paths from serverA:\n");
	printf("------------------------------------------------\n");
	printf("%-16s\t%-16s\t\n", "Destination", "Min Length");
	printf("------------------------------------------------\n");
	// iterate the result
	for (int idx = 0; idx < 10; idx++) {
		int cur_des = received_buff.min_path_vertex[idx];
		int cur_len = received_buff.min_path_dist[idx];
		if (cur_len > 0) {
			printf("%-16d\t%-16d\t\t\n", cur_des, cur_len);
		}
	}
	printf("------------------------------------------------\n");
}

/* Send request to serverB */
void send_to_serverB() {
	memset(&udp_server_addr,0, sizeof(udp_server_addr));
	udp_server_addr.sin_family = AF_INET;
	udp_server_addr.sin_port = htons(SERVERB_PORT);
	udp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	char send_buf2[1024];
	memset(send_buf2, 0, 1024);
	memcpy(send_buf2, &cal_request, sizeof cal_request);
	sendto(udp_sockfd, send_buf2, 1024, 0, (struct sockaddr *) &udp_server_addr, sizeof udp_server_addr);

	//print the onscreen message
	printf("The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <%d>.\n", 
		AWS_UDP_PORT);
}

/* Receive response from serverB */
void receive_from_serverB() {
	// build the container that will receive the msg from A and receive from A
	char recv_buf3[1024];
	memset(recv_buf3, 'z', 1024);
	socklen_t server_udp_len = sizeof udp_client_addr;
	recvfrom(udp_sockfd, recv_buf3, 1024, 0, (struct sockaddr*)&udp_client_addr, &server_udp_len);
	memset(&received_delay, 0, sizeof received_delay);
	memcpy(&received_delay, recv_buf3, sizeof received_delay);

	// print onscreen message
	printf("The AWS has received delays from serverB:\n");
	printf("---------------------------------------------------------------\n");
	printf("%-8s\t%-8s\t%-8s\t%-8s\n", "Destination", "Tt", "Tp", "Delay");
	printf("---------------------------------------------------------------\n");
	double trans_time = received_delay.trans_time;
	for (int idx = 0; idx < 10; idx++) {
		int cur_des = received_delay.vertex[idx];
		int cur_len = received_delay.min_path_dist[idx];
		double prop_time = received_delay.prop_time[idx];
		double end_to_end = received_delay.end_to_end[idx];
		if (cur_len > 0) {
			printf("%-8d\t%-8.2f\t%-8.2f\t%-8.2f\n", cur_des, trans_time, prop_time, end_to_end);
		}
	}
	printf("---------------------------------------------------------------\n");
}
