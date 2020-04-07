#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <cctype>
#include <ctype.h>
#include <map>
#include <set>

using namespace std;

#define AWS_UDP_PORT 23532
#define SERVERB_PORT 22532

struct CalculationRequestInfo
{
	int     	min_path_vertex[10];
	int     	min_path_dist[10];
	double		prop_speed;
	double		tran_speed;
	long long	file_size;
};

struct CalculationResponseInfo
{
	double	trans_time;
	int     vertex[10];
	int     min_path_dist[10];
	double  prop_time[10];
	double  end_to_end[10];
};

void create_and_bind_udp_socket();

void show_received_msg();

void show_calculation_result();

int udp_sockfd;
struct sockaddr_in udp_server_addr, udp_client_addr;
CalculationRequestInfo received_buff;
CalculationResponseInfo output_msg;

int main(int argc, const char* argv[]) {
	// create a udp socket
	create_and_bind_udp_socket();

	while(true) {
		// receive related message from aws and put it into received_buff
        // reference: Beej Guide
        socklen_t udp_client_addr_len = sizeof udp_client_addr;
        char recv_buff[1024];
        memset(recv_buff, 'z', 1024);
		if (recvfrom (udp_sockfd, recv_buff, 1024,0, (struct sockaddr *) &udp_client_addr, &udp_client_addr_len) == -1){
		perror("ServerB Receive Error");
		exit(1);
		}
        memset(&received_buff, 0, sizeof received_buff);
        memcpy(&received_buff, recv_buff, sizeof received_buff);

        // print the onscreen message
        show_received_msg();
		
        double prop_v = received_buff.prop_speed;
        double tran_v = received_buff.tran_speed;
        long long file_size = received_buff.file_size;

        // fill in some information in output message
        output_msg.trans_time = file_size / (8 * tran_v);

        // iterate through all vertices and calculate prop time and end to end time
        for (int idx = 0; idx < 10; idx++) {
  			int cur_len = received_buff.min_path_dist[idx];
            output_msg.vertex[idx] = received_buff.min_path_vertex[idx];
            output_msg.min_path_dist[idx] = received_buff.min_path_dist[idx];
            output_msg.prop_time[idx] = cur_len / prop_v;
            output_msg.end_to_end[idx] = cur_len / prop_v + output_msg.trans_time;
        }

    	// show the onscreen message of the calculations
        show_calculation_result();

        // send three delays(output message) to AWS server
        // reference: Beej Guide
        char send_buff[1024];
        memset(send_buff, 0, 1024);
        memcpy(send_buff, &output_msg, sizeof output_msg);
		if (sendto(udp_sockfd, send_buff, sizeof send_buff, 0, (const struct sockaddr *) &udp_client_addr, (socklen_t)sizeof udp_client_addr) == -1) {
			perror("ServerB Response Error");
			exit(1);
		}
		printf("The Server B has finished sending the output to AWS.\n");
    	}
	close(udp_sockfd);
	return 0;
}

/* Create a udp socket and bind it with udp server address */
void create_and_bind_udp_socket() {
	// create a udp socket
	// reference: Beej Guide
	if ((udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("ServerA UDP Socket Create Error");
        	exit(1);
    }
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
	udp_server_addr.sin_family = AF_INET;
	udp_server_addr.sin_port   = htons(SERVERB_PORT);
	udp_server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // bind the socket with server address
    if(bind(udp_sockfd, (struct sockaddr*)&udp_server_addr, sizeof udp_server_addr) == -1) {
		perror("ServerA UDP Bind Error");
		exit(1);
	}

    // print the onscreen message
    printf("The Server B is up and running using UDP on port <%d>.\n",SERVERB_PORT);
}

/* Show the received message from aws on screen */
void show_received_msg() {
	printf("The Server B has received data for calculation:\n");
	printf("* Propagation speed: <%.2f> km/s\n", received_buff.prop_speed);
	printf("* Transmission speed: <%.2f> Bytes/s\n", received_buff.tran_speed);
    
	for (int idx = 0; idx < 10; idx++) {
        int cur_des = received_buff.min_path_vertex[idx];
        int cur_len = received_buff.min_path_dist[idx];
        if (cur_len > 0) {
            	printf("* Path length for destination <%d> : <%d>\n", cur_des, cur_len);
        }
    }
}

/* Show the calculation results after calculation */
void show_calculation_result() {
    // print onscreen message
	printf("The Server B has finished the calculation of the delays:\n");
	printf("---------------------------------------------------------------\n");
	printf("%-8s\t%-8s\t%-8s\t%-8s\n", "Destination", "Tt", "Tp", "Delay");
	printf("---------------------------------------------------------------\n");
    double trans_time = output_msg.trans_time;
    for (int idx = 0; idx < 10; idx++) {
        int cur_des = output_msg.vertex[idx];
        int cur_len = output_msg.min_path_dist[idx];
        double prop_time = output_msg.prop_time[idx];
        double end_to_end = output_msg.end_to_end[idx];
        if (cur_len > 0) {
            printf("%-8d\t%-8.2f\t%-8.2f\t%-8.2f\n", cur_des, trans_time, prop_time, end_to_end);
        }
    }
	printf("---------------------------------------------------------------\n");
}
