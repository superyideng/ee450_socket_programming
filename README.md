1. Full Name : Yi Deng

2. Student ID : 9696485532

3. What I have done in the assignment : 
	I have implemented a computing system with four components, client, aws, serverA, serverB, in it. Firstly, the client issues tasks to aws server, then aws splits and forward tasks to serveA and serverB. After receiving responses, the aws forwards messages back to the client. 

4. Code files and their functions :
client.cpp
User interface, in which users are able to send computing request and will get the information with messages printed on the screen.

aws.cpp
Task scheduler and relay. Aws will split the requests into path finding and delay computation, forwarding corresponding information to serverA and serverB. And send the response information back to the client.

serverA.cpp
Path finding server which finds the shortest path from the starting vertex to any other vertices using dijkstra’s.

serverB.cpp
Delay computation server. It computes transmission delay, propagation delay and end-to-end delay.

5. The format of all the messages exchanged :
Basically, the format of messages to store exchanged information in my code is struct. But during exchange, I actually use memcpy to convert my struct into string in the sender side and convert from string to struct in the receiver side. So the actual format for message exchange is string.
============client --- aws===============
// compute request message format 
struct ComputeRequestInfo
{
    int     		map_id;
    int     		src_vertex_idx;
    long long		file_size;
};
* I should have used string to store map id, which is more reasonable and straightforward, but I found string in struct performs bad in virtual machine during sending and receiving, so instead I use int to store the ASCII of map id.

// compute response message format
struct CalculationResponseInfo
{
    double	trans_time;
    int		vertex[10];
    int		min_path_dist[10];
    double	prop_time[10];
    double	end_to_end[10];
};

============aws --- serverA==============
// path finding request message format
struct PathForwardInfo
{
    int     map_id;
    int     src_vertex_idx;
};

// path response message format
struct PathResponseInfo
{
    int		min_path_vertex[10];
    int		min_path_dist[10];
    double	prop_speed;
    double	tran_speed;
};

============aws --- serverB================
// delay calculation request message format
struct CalculationRequestInfo
{
    int			min_path_vertex[10];
    int			min_path_dist[10];
    double 		prop_speed;
    double  		tran_speed;
    long long  	file_size;
};

// delay calculation response message format
struct CalculationResponseInfo
{
    double	trans_time;
    int		vertex[10];
    int		min_path_dist[10];
    double  	prop_time[10];
    double 	end_to_end[10];
};

6. Any idiosyncrasy of my project : 
I didn’t consider the situation where we have more than 50 maps. If there are more than 10 maps, I can solve the problem by enlarging the size of map array.
If the client requests too fast, aws may lost some requests.
I didn’t consider UDP packet loss problem, when udp packets between aws and serverA or between aws and serverB lost, system may have problem.
I did’t consider the byte order problem, When client and servers run in different byte order platforms, message will not be decode correctly.

7. Reused Code : 
	I referenced Beej Guide Network Programming to write some functions like sendto, accept.etc and I have them commented in my code.
