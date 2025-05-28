
//=======================================================================================================================
// Course: 159.342
// Description: Cross-platform, Active mode FTP SERVER, Start-up Code for
// Assignment 1
//
// This code gives parts of the answers away but this implementation is only
// IPv4-compliant. Remember that the assignment requires a fully IPv6-compliant
// cross-platform FTP server that can communicate with a built-in FTP client
// either in Windows 11, Ubuntu Linux or MacOS.
//
// This program is cross-platform but your assignment will be marked only in
// Windows 11.
//
// You must change parts of this program to make it IPv6-compliant (replace all
// data structures and functions that work only with IPv4).
//
// Hint: The sample TCP server codes show the appropriate data structures and
// functions that work in both IPv4 and IPv6.
//       We also covered IPv6-compliant data structures and functions in our
//       lectures.
//
// Author: n.h.reyes@massey.ac.nz
//=======================================================================================================================
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 

#define USE_IPV6                                                               \
  false // if set to false, IPv4 addressing scheme will be used; you need to set
        // this to true to enable IPv6 later on.  The assignment will be marked
        // using IPv6!

#if defined __unix__ || defined __APPLE__
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netdb.h> //used by getnameinfo()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#elif defined __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h> //required by getaddrinfo() and special constants
//   #include <WinSock2.h>
WSADATA wsadata;
#define WSVERS MAKEWORD(2, 2)
#define USERNAME "napoleon"
#define PASSWORD "342"
/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
// The high-order byte specifies the minor version number;
// the low-order byte specifies the major version number.
// Create a WSADATA object called wsadata.
#endif

#define BUFFER_SIZE 256
enum class FileType { BINARY, TEXT, UNKNOWN };

FileType file_type;
//*MAIN*
int main(int argc, char *argv[]) {
  
#if defined __unix__ || defined __APPLE__
  // nothing to do here

#elif defined _WIN32 // 소켓 생성을 위한 초기화.

  int err =
      WSAStartup(WSVERS, &wsadata); // Vr 2.2, wsadata structure containing
                                    // socket implementation information.

  if (err != 0) {
    WSACleanup();
    // Tell the user that we could not find a usable WinsockDLL
    printf("WSAStartup failed with error: %d\n", err);
    exit(1);
  }
  // Professor's code was used for version check.
  if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
    printf("Could not find a usable version of Winsock.dll\n");
    WSACleanup();
    exit(1);
  } else {
    printf("\nThe Winsock 2.2 dll was initialised.\n");
  }
#endif
  //*INITIALIZATION*//
  file_type = FileType::TEXT;
  char clientHost[NI_MAXHOST];
  char clientService[NI_MAXSERV];

  struct sockaddr_storage localaddr, remoteaddr; 
  // PORT, EPTR
 // struct sockaddr_in ipaddr4;
//   struct sockaddr_in6 ipaddr6; 
  struct sockaddr_storage ipaddr46; 

  struct addrinfo *addrinfo_linked_li = NULL; // getaddrinfo()'s result.
  struct addrinfo hints; // Find all ip addresses based on the hints pls

  char send_buffer[BUFFER_SIZE], receive_buffer[BUFFER_SIZE];

  int active = 0;
  int n, bytes, addrlen;

  char port_num[32];
  int result = 0;

#if defined __unix__ || defined __APPLE__

  int listening_socket, client_socket;              
  int ns_data, server_data_socket; 

#elif defined _WIN32
  SOCKET listening_socket, client_socket; // Control Socekt
  SOCKET ns_data, server_data_socket;     // Data Socket 
#endif
 
#if defined __unix__ || defined __APPLE__
  ns_data = -1;
#elif defined _WIN32
  server_data_socket = INVALID_SOCKET;
  ns_data = INVALID_SOCKET;
#endif

  printf("\n============================================\n");
  printf("   << 159.342 Cross-platform FTP Server >>");
  printf("\n============================================\n");
  printf("   submitted by:    Junyeong Kim");
  printf("\n           date:     14/04/2025");
  printf("\n============================================\n");

  memset(&localaddr, 0, sizeof(localaddr));   // clean up the structure
  memset(&remoteaddr, 0, sizeof(remoteaddr)); // clean up the structure

  memset(&hints, 0, sizeof(struct addrinfo));

  //********************************************************************
  // SOCKET
  //********************************************************************
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol =
      IPPROTO_TCP; // I found taht even there is a ai_socket type, we have to
                   // mention once again that I'm using TCP protocl :)
  listening_socket = socket(AF_INET, SOCK_STREAM, 0); // old programming style, needs replacing
  hints.ai_flags = AI_PASSIVE;
  
  memset(port_num, 0, 32);

  // ** Reference : Professor's tutorials
  if (argc == 2) {
    result = getaddrinfo(NULL, argv[1], &hints, &addrinfo_linked_li); // IPV4 & IPV6-compliant                        
    sprintf(port_num, "%s", argv[1]);
    printf("\nargv[1] = %s\n", argv[1]);
  } 
  else {
    result = getaddrinfo(NULL, "1234", &hints, &addrinfo_linked_li);
    sprintf(port_num, "%s", "1234");
    printf("\nUsing DEFAULT_PORT = 1234\n");
  }

  if (result != 0) {
    printf("getaddrinfo failed and occured an error(s): %d\n", result);

	#if defined _WIN32
		WSACleanup();
	#endif
		return 1;
  }

  listening_socket = socket(addrinfo_linked_li->ai_family, addrinfo_linked_li->ai_socktype,
      addrinfo_linked_li->ai_protocol); // Always remind that addrinfo_linked_li points to the frist node !!! (specific ip address structure)  
                
  int off = 0;            
  setsockopt(listening_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&off, // ******* DUAL STACK ********* :)
             sizeof(off));

#if defined __unix__ || defined __APPLE__
  if (listening_socket < 0) {
    printf("socket creation failed\n");
  }
#elif defined _WIN32
  if (listening_socket == INVALID_SOCKET) {
    printf("socket creation failed\n");
  }
#endif

  //********************************************************************
  // BIND  Reference : Professor's tutorials
  //********************************************************************
  if (bind(listening_socket, addrinfo_linked_li->ai_addr,
           (int)(addrinfo_linked_li->ai_addrlen)) != 0) {
#if defined __unix__ || defined __APPLE__
    printf("\nbind failed\n");
    freeaddrinfo(addrinfo_linked_li);
    close(listening_socket); // close socket
#elif defined _WIN32
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(addrinfo_linked_li);
    closesocket(listening_socket);
    WSACleanup();
#endif
    return 1;
  }

  freeaddrinfo(addrinfo_linked_li);
  printf("\n<<<SERVER>>> is listening at PORT: %s\n", port_num);

  //********************************************************************
  // LISTEN
  //********************************************************************
  listen(listening_socket, 5);

  //********************************************************************
  // INFINITE LOOP Reference : Professor's tutorial materials
  //********************************************************************
  int count = 0;
  while (1) {                     // Start of MAIN LOOP
    addrlen = sizeof(remoteaddr); // IPv4 & IPv6 compatible
    //********************************************************************
    // NEW SOCKET newsocket = accept  //CONTROL CONNECTION
    //********************************************************************
    // printf("\n------------------------------------------------------------------------\n");
    // printf("SERVER is waiting for an incoming connection request at port:%d",
    // ntohs(localaddr.sin_port));
    // printf("\n------------------------------------------------------------------------\n");

#if defined __unix__ || defined __APPLE__
    client_socket = accept(listening_socket, (struct sockaddr *)(&remoteaddr),
                           (socklen_t *)&addrlen);
#elif defined _WIN32
    client_socket =
        accept(listening_socket, (struct sockaddr *)(&remoteaddr), &addrlen);
#endif

#if defined __unix__ || defined __APPLE__
    if (client_socket == -1) {
      printf("\naccept failed\n");
      close(listening_socket0);
      return 1;
    } else {
      printf("\nA <<<CLIENT>>> has been accepted.\n");

      // strcpy(clientHost,inet_ntoa(clientAddress.sin_addr)); //IPV4
      // sprintf(clientService,"%d",ntohs(clientAddress.sin_port)); //IPV4
      // ---
      int returnValue;
      memset(clientHost, 0, sizeof(clientHost));
      memset(clientService, 0, sizeof(clientService));

      returnValue = getnameinfo((struct sockaddr *)&remoteaddr, addrlen,
                                clientHost, sizeof(clientHost), clientService,
                                sizeof(clientService), NI_NUMERICHOST);
      if (returnValue != 0) {
        printf("\nError detected: getnameinfo() failed \n");
        exit(1);
      } else {
        printf("\nConnected to <<<CLIENT>>> with IP address:%s, at Port:%s\n",
               clientHost, clientService);
      }
    }
#elif defined _WIN32

    if (client_socket == INVALID_SOCKET) {
      printf("accept failed: %d\n", WSAGetLastError());
      closesocket(listening_socket);
      WSACleanup();
      return 1;
    } else {
      printf("\nA <<<CLIENT>>> has been accepted.\n");

      DWORD returnValue;
      memset(clientHost, 0, sizeof(clientHost));
      memset(clientService, 0, sizeof(clientService));

      returnValue = getnameinfo((struct sockaddr *)&remoteaddr, addrlen,
                                clientHost, sizeof(clientHost), clientService,
                                sizeof(clientService), NI_NUMERICHOST);
      if (returnValue != 0) {
        printf("\nError detected: getnameinfo() failed with error#%d\n",
               WSAGetLastError());
        exit(1);
      } else {
        printf("\nConnected to <<<Client>>> with IP address:%s, at Port:%s\n",
               clientHost, clientService);
      }
    }

#endif
    //********************************************************************
    // Respond with welcome message
    //*******************************************************************
    snprintf(send_buffer, BUFFER_SIZE, "220 FTP Server ready. \r\n");
	bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
	//printf("bytes : %d", bytes);

#if defined __unix__ || defined __APPLE__      
	if ((bytes == -1) || (bytes == 0)) break;
#elif defined _WIN32      
	if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
#endif
	printf("\nMSG SENT     --->>> :%s\n", send_buffer);
	
 
	memset(&send_buffer,0,BUFFER_SIZE);	
    //********************************************************************
    // COMMUNICATION LOOP per CLIENT
    //********************************************************************

    while (1) {
	  printf("DEBUG:: You are in the communication loop!!!\n");
      n = 0;
	  memset(&receive_buffer,0,BUFFER_SIZE);
	  
	  char c = ' ';
	  // Read Message
      while (1) {
        bytes = recv(client_socket, &c, 1, 0); // receive byte by byte... 

        if ((bytes < 0) || (bytes == 0)){
			printf("[DEBUG] recv() ended. bytes = %d\n", bytes);
			break;
		}
        if (c == '\n') { /*end on a LF*/
          c = '\0';
          break;
        }
		
        if (c != '\r'){
			receive_buffer[n] = c;
		}
		n++;
      }

      if (bytes == 0)
        printf("\nclient has gracefully exited.\n"); // 2022

      if ((bytes < 0) || (bytes == 0))
        break;

      printf("[DEBUG INFO] command received:  '%s\\r\\n' \n", receive_buffer);

      //********************************************************************
      // PROCESS COMMANDS/REQUEST FROM USER
      //********************************************************************
      if (strncmp(receive_buffer, "USER", 4) == 0) {
		char username[BUFFER_SIZE];
        if (sscanf(receive_buffer, "USER %s", username) == 1) {
			if (strcmp(username, USERNAME) == 0){ // napoleon
				snprintf(send_buffer, sizeof(send_buffer), "331 Password required.\r\n");
			}
			else{
				snprintf(send_buffer, sizeof(send_buffer), "530 Invalid user.\r\n");
			}
		}
		else{
			snprintf(send_buffer, sizeof(send_buffer), "501 Syntax error in parameters.\r\n");
		}

		bytes = send(client_socket, send_buffer, strlen(send_buffer), 0); 
		#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);
			bytes = 0;
      }
	
      //---
      if (strncmp(receive_buffer, "PASS", 4) == 0) {

		char password[BUFFER_SIZE];
		if (sscanf(receive_buffer, "PASS %s", password) == 1) {
			if (strcmp(password, PASSWORD) == 0){ // napoleon
				snprintf(send_buffer, sizeof(send_buffer), "230 Public login sucessful \r\n");
			}
			else{
				snprintf(send_buffer, sizeof(send_buffer), "530 Wrong Password.\r\n");
			}
		}
		else{
			snprintf(send_buffer, sizeof(send_buffer), "501 Syntax error in parameters.\r\n");
		}

		bytes = send(client_socket, send_buffer, strlen(send_buffer), 0); 
		#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);
			bytes = 0;
      }
      //---
      if (strncmp(receive_buffer, "SYST", 4) == 0) { // quote SYST from window interface
        printf("Information about the system \n");
        snprintf(send_buffer, BUFFER_SIZE, "215 Windows 64-bit\r\n");

		bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
		#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);	
			bytes = 0;
      }

      //--- ** REFACTORED ** 
      if (strncasecmp(receive_buffer, "TYPE", 4) == 0) {

        char objType;
        int scannedItems = sscanf(receive_buffer, "TYPE %c", &objType);

		if (scannedItems >= 1){
			switch (toupper(objType)) {
				case 'I':
				  file_type = FileType::BINARY;
				  printf("using binary mode to transfer files.\n");
				  snprintf(send_buffer, BUFFER_SIZE, "200 command OK - Image Mode.\r\n");
				  break;
				case 'A':
				  file_type = FileType::TEXT;
				  printf("using ASCII mode to transfer files.\n");
				  snprintf(send_buffer, BUFFER_SIZE, "200 command OK - ASCII Mode.\r\n");
				  break;
				default:
				  snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in arguments\r\n");
				  break;
				}
			bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			#if defined __unix__ || defined __APPLE__      
				if ((bytes == -1) || (bytes == 0)) break;
			#elif defined _WIN32      
				if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
			#endif
				printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				memset(&send_buffer,0,BUFFER_SIZE);	
				memset(&receive_buffer,0,BUFFER_SIZE);	
				bytes = 0;
		}
		else{
			snprintf(send_buffer, BUFFER_SIZE,"501 Syntax error in arguments\r\n");
			bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			#if defined __unix__ || defined __APPLE__      
				if ((bytes == -1) || (bytes == 0)) break;
			#elif defined _WIN32      
				if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
			#endif
				printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				memset(&send_buffer,0,BUFFER_SIZE);	
				memset(&receive_buffer,0,BUFFER_SIZE);	
				bytes = 0;
		}
   
      }
      
	  //---
    //   if (strncmp(receive_buffer, "STOR", 4) == 0) {
    //     printf("unrecognised command \n");
    //     count = snprintf(send_buffer, BUFFER_SIZE,
    //                      "502 command not implemented\r\n");
    //     if (count >= 0 && count < BUFFER_SIZE) {
    //       bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
    //     }
    //     printf("[DEBUG INFO] <-- %s\n", send_buffer);
    //     if (bytes < 0)
    //       break;
    //   }
      //---

      if (strncmp(receive_buffer, "RETR", 4) == 0) {
		char filename[BUFFER_SIZE];
		strncpy(filename, receive_buffer + 5, BUFFER_SIZE); // Only contain this part <fileName>
		filename[BUFFER_SIZE - 1] = '\0';
		printf("eprt_cmd is %s", filename);

		std::ios_base::openmode mode = std::ios::in;
		if (file_type == FileType::BINARY) {
			mode |= std::ios::binary;
			printf("DEBUG -> mode is bnary");
		}
		std::ifstream file(filename, mode);
		if (!file.is_open()) {
			snprintf(send_buffer, BUFFER_SIZE, "550 File not found.\r\n");
			send(client_socket, send_buffer, strlen(send_buffer), 0);
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);	
			bytes = 0;
			break; 	
		}


		snprintf(send_buffer, BUFFER_SIZE,	"150 Opening data connection for file transfer...\r\n");
		bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
		#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);	
			bytes = 0;

			if (connect(server_data_socket, (struct sockaddr *)&ipaddr46, addrlen) != 0) {
                    	   
				snprintf(send_buffer, BUFFER_SIZE,"425 Something went wrong, can't start active connection... \r\n");
				bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);

				#if defined __unix__ || defined __APPLE__      
					if ((bytes == -1) || (bytes == 0)) break;
				#elif defined _WIN32      
					if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
						printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);	
						break;
					}
				#endif
					printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
					memset(&send_buffer,0,BUFFER_SIZE);	
					memset(&receive_buffer,0,BUFFER_SIZE);	
					closesocket(server_data_socket);
					memset(&ipaddr46, 0, sizeof(ipaddr46));
					bytes = 0;
			}
			else{
				snprintf(send_buffer, BUFFER_SIZE,	"200 Data sockets connected successfully...\r\n");
				bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
				#if defined __unix__ || defined __APPLE__      
					if ((bytes == -1) || (bytes == 0)) break;
				#elif defined _WIN32      
					if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
				#endif
					printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
					memset(&send_buffer,0,BUFFER_SIZE);	
					memset(&receive_buffer,0,BUFFER_SIZE);	
					bytes = 0;

				int totalSent = 0; 
				while (file.good()) {
					file.read(send_buffer, BUFFER_SIZE);
					std::streamsize bytesSize = file.gcount();
					if (bytesSize > 0) {
						int totalSent = 0;
						while (totalSent < bytesSize) { /// **** Partial send !!! ***** 
							int bytes = send(server_data_socket, send_buffer + totalSent, (int)(bytesSize - totalSent), 0);
							if (bytes <= 0) {
								printf("\n Error -> send() failed or connection closed.\n");
								break;
							}
							totalSent += bytes;
						}
						// printf("\nDebug ->  Transferred %d bytes\n", totalSent);
						memset(send_buffer, 0, BUFFER_SIZE);
					}
				}
				// ****** The code below didn't consider the "partial send" ***** 
				// while (file.good()) {
				// 	file.read(send_buffer, BUFFER_SIZE);
				// 	std::streamsize bytesSize = file.gcount();
				// 	if (bytesSize > 0) {
				// 		send(server_data_socket, send_buffer, (int)bytesSize, 0); // learnt that strlen(send_buffer) shouldn't be used here. 
				// 		#if defined __unix__ || defined __APPLE__      
				// 			if ((bytes == -1) || (bytes == 0)) break;
				// 		#elif defined _WIN32      
				// 			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
				// 		#endif
				// 			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				// 			memset(&send_buffer,0,BUFFER_SIZE);	
				// 			memset(&receive_buffer,0,BUFFER_SIZE);	
				// 			bytes = 0;
				// 	}
					
				// }
				file.close();
				snprintf(send_buffer, BUFFER_SIZE,	"226 File Transfer Complete..\r\n");
				bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
				#if defined __unix__ || defined __APPLE__      
					if ((bytes == -1) || (bytes == 0)) break;
				#elif defined _WIN32      
					if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
				#endif
					printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
					memset(&send_buffer,0,BUFFER_SIZE);	
					memset(&receive_buffer,0,BUFFER_SIZE);	
					bytes = 0;
			}
			#if defined __unix__ || defined __APPLE__
				close(server_data_socket);

			#elif defined _WIN32
				closesocket(server_data_socket);
			#endif
				memset(&ipaddr46, 0, sizeof(ipaddr46));
      }
      //---
      if (strncmp(receive_buffer, "OPTS", 4) == 0) {
        printf("unrecognised command \n");
        snprintf(send_buffer, BUFFER_SIZE, "502 command not implemented\r\n");
		bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
		#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);	
			bytes = 0;              
      }
      //---
	if (strncmp(receive_buffer, "EPRT", 4) ==0) {
		// EPRT |<protocol>|<address>|<port>|
		// EPRT |1|127.0.0.1|1024| ipv4
		// EPRT |2|2001:db8::1234|1024| ipv6

		char eprt_cmd[BUFFER_SIZE];
		strncpy(eprt_cmd, receive_buffer + 5, BUFFER_SIZE); // Only contain this part |<protocol>|<address>|<port>|
		eprt_cmd[BUFFER_SIZE - 1] = '\0';
		printf("eprt_cmd is %s", eprt_cmd);
	
		char *protocol = strtok(eprt_cmd + 1, "|");
		char *ip       = strtok(NULL, "|");
		char *port     = strtok(NULL, "|");

		int port_int = atoi(port);
		int protocol_int = atoi(protocol); 
		
		if (protocol && ip && port) {
			printf("DEBUG : \n");
			printf("Protocol: %s\n", protocol);
			printf("IP:       %s\n", ip);
			printf("Port:     %s\n", port);
		}
		else{

			printf("ERROR : WRONG EPRT request"); 
			snprintf(send_buffer, BUFFER_SIZE, "501 Syntax error in EPRT parameters.\r\n");    
			bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			#if defined __unix__ || defined __APPLE__      
				if ((bytes == -1) || (bytes == 0)) break;
			#elif defined _WIN32      
				if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
			#endif
				printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				memset(&send_buffer,0,BUFFER_SIZE);	
				memset(&receive_buffer,0,BUFFER_SIZE);	
				bytes = 0;	
		}

		memset(&ipaddr46, 0, sizeof(ipaddr46));
		if (protocol_int== 1){
			server_data_socket = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in *ipaddr4 = (struct sockaddr_in*)&ipaddr46;
			memset(&ipaddr4, 0, sizeof(ipaddr4));
			ipaddr4->sin_family = AF_INET;
			ipaddr4->sin_addr.s_addr =inet_addr(ip);
			ipaddr4->sin_port = htons(port_int);
			addrlen = sizeof(struct sockaddr_in);

			snprintf(send_buffer, BUFFER_SIZE,"200 EPRT command successful\r\n");    
			bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			#if defined __unix__ || defined __APPLE__      
				if ((bytes == -1) || (bytes == 0)) break;
			#elif defined _WIN32      
				if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
			#endif
				printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				memset(&send_buffer,0,BUFFER_SIZE);	
				memset(&receive_buffer,0,BUFFER_SIZE);	
				bytes = 0;	
		}
		else if (protocol[0] == '2'){
			server_data_socket = socket(AF_INET6, SOCK_STREAM, 0);
			struct sockaddr_in6 *ipaddr6 = (struct sockaddr_in6*)&ipaddr46;
			memset(ipaddr6, 0, sizeof(ipaddr6));

			ipaddr6->sin6_family = AF_INET6;
			ipaddr6->sin6_port = htons(port_int);
			if (inet_pton(AF_INET6, ip, &ipaddr6->sin6_addr) != 1) {
				snprintf(send_buffer, BUFFER_SIZE, "501 Invalid IPv6 address.\r\n");
				send(client_socket, send_buffer, strlen(send_buffer), 0);
			}
			addrlen = sizeof(struct sockaddr_in6);

			snprintf(send_buffer, BUFFER_SIZE,"200 EPRT command successful\r\n");    
			bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			#if defined __unix__ || defined __APPLE__      
				if ((bytes == -1) || (bytes == 0)) break;
			#elif defined _WIN32      
				if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
			#endif
				printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				memset(&send_buffer,0,BUFFER_SIZE);	
				memset(&receive_buffer,0,BUFFER_SIZE);	
				bytes = 0;	

		}

	}
      //---
      if (strncmp(receive_buffer, "CWD", 3) == 0) {
        printf("unrecognised command \n");
        count = snprintf(send_buffer, BUFFER_SIZE,
                         "502 command not implemented\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }
      //---
      if (strncmp(receive_buffer, "QUIT", 4) == 0) {
        printf("Quit \n");
        count = snprintf(send_buffer, BUFFER_SIZE,
                         "221 Connection close by client\r\n");
        if (count >= 0 && count < BUFFER_SIZE) {
          bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
        }
        printf("[DEBUG INFO] <-- %s\n", send_buffer);
        if (bytes < 0)
          break;
      }
      //---
      if (strncmp(receive_buffer, "PORT", 4) == 0) {
        //server_data_socket = socket(AF_INET, SOCK_STREAM, 0);

		#if defined __unix__ || defined __APPLE__
			if (listening_socket < 0) {
				printf("socket creation failed\n");
			}
		#elif defined _WIN32
			if (listening_socket == INVALID_SOCKET) {
				printf("socket creation failed\n");
			}
		#endif
        // local variables
        // unsigned char act_port[2];
        int act_port[2];
        int act_ip[4], port_dec;
        char ip_decimal[NI_MAXHOST];
        printf("===================================================\n");
        printf("\tActive FTP mode, the client is listening... \n");
        active = 1; // flag for active connection

        int scannedItems = sscanf(receive_buffer, "PORT %d,%d,%d,%d,%d,%d",
                                  &act_ip[0], &act_ip[1], &act_ip[2],
                                  &act_ip[3], &act_port[0], &act_port[1]);

        if (scannedItems < 6) {
          count = snprintf(send_buffer, BUFFER_SIZE,
                           "501 Syntax error in arguments \r\n");

          if (count >= 0 && count < BUFFER_SIZE) {
            bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
			#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
			#elif defined _WIN32      
				if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
			#endif
				printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
				memset(&send_buffer,0,BUFFER_SIZE);	
				memset(&receive_buffer,0,BUFFER_SIZE);	
				bytes = 0;
          }
        }

		memset(&ipaddr46, 0, sizeof(ipaddr46));
		struct sockaddr_in *ipaddr4 = (struct sockaddr_in*)&ipaddr46;
	
        ipaddr4->sin_family = AF_INET; // Since PORT command only works in ipv4 sockets connetion, I've used sockaddr_in instead of storage. 
            
        count = snprintf(ip_decimal, NI_MAXHOST, "%d.%d.%d.%d", act_ip[0],
                         act_ip[1], act_ip[2], act_ip[3]);

        if (!(count >= 0 && count < BUFFER_SIZE))
          break;

        printf("\tCLIENT's IP is %s\n", ip_decimal); // IPv4 format
        ipaddr4->sin_addr.s_addr =inet_addr(ip_decimal); 
            
        port_dec = act_port[0];
        port_dec = port_dec << 8;
        port_dec = port_dec + act_port[1]; 
        printf("\tCLIENT's Port is %d\n", port_dec);
        printf("===================================================\n");

        ipaddr4->sin_port = htons(port_dec); // At this point (PORT Request) you don't have to make a connection. These actions will be done after LIST/RETR.
		server_data_socket = socket(AF_INET, SOCK_STREAM, 0);
		addrlen = sizeof(struct sockaddr_in);

        snprintf(send_buffer, BUFFER_SIZE, "200 PORT Port command successful\r\n");    
		bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
		#if defined __unix__ || defined __APPLE__      
			if ((bytes == -1) || (bytes == 0)) break;
		#elif defined _WIN32      
			if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
		#endif
			printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
			memset(&send_buffer,0,BUFFER_SIZE);	
			memset(&receive_buffer,0,BUFFER_SIZE);	
			bytes = 0;	

        // Note: the following connect() function is not correctly placed.  It
        // works, but technically, as defined by
        // the protocol, connect() should occur in another place.  Hint:
        // carefully inspect the lecture on FTP, active operations to find the
        // answer.

      }
      //---

      // technically, LIST is different than NLST,but we make them the same here
      if ((strncmp(receive_buffer, "LIST", 4) == 0) || (strncmp(receive_buffer, "NLST", 4) == 0)) {
         
		#if defined __unix__ || defined __APPLE__

				int i = system("ls -la > tmp.txt"); // change that to 'dir', so windows // can understand
									
		#elif defined _WIN32

				int i = system("dir > tmp.txt");
		#endif
				printf("The value returned by system() was: %d.\n", i);

				FILE *fin;
				fin = fopen("tmp.txt", "r"); // open tmp.txt file

				//--Data Connection
				snprintf(send_buffer, BUFFER_SIZE,	"150 Opening data connection for directory list...\r\n");
				bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
				#if defined __unix__ || defined __APPLE__      
					if ((bytes == -1) || (bytes == 0)) break;
				#elif defined _WIN32      
					if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
				#endif
					printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
					memset(&send_buffer,0,BUFFER_SIZE);	
					memset(&receive_buffer,0,BUFFER_SIZE);	
					bytes = 0;
					
				if (connect(server_data_socket, (struct sockaddr *)&ipaddr46, addrlen) != 0) {
                    	   
					snprintf(send_buffer, BUFFER_SIZE,"425 Something is wrong, can't start active connection... \r\n");

					#if defined __unix__ || defined __APPLE__      
						if ((bytes == -1) || (bytes == 0)) break;
					#elif defined _WIN32      
						if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
							printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);	
							break;
						}
					#endif
						printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
						memset(&send_buffer,0,BUFFER_SIZE);	
						memset(&receive_buffer,0,BUFFER_SIZE);	
						closesocket(server_data_socket);
						memset(&ipaddr46, 0, sizeof(ipaddr46));
						bytes = 0;
				}
				else {
					snprintf(send_buffer, BUFFER_SIZE, "200 Data sockets connetcted successfully\r\n"); // Because of the network timing issue, this message data arrived after the direcotry file data in the window ftp client side. 
					bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);				 
					#if defined __unix__ || defined __APPLE__      
						if ((bytes == -1) || (bytes == 0)) break;
					#elif defined _WIN32      
						if ((bytes == SOCKET_ERROR) || (bytes == 0)) break;
					#endif
						printf("\n[DEBUG INFO]     --->>> :%s\n", send_buffer);
						memset(&send_buffer,0,BUFFER_SIZE);	
						memset(&receive_buffer,0,BUFFER_SIZE);	
						bytes = 0;

						char temp_buffer[80];
						printf("transferring list...\n");
						while (!feof(fin)) {
							strcpy(temp_buffer, "");
							if (fgets(temp_buffer, 78, fin) != NULL) {
		
								count = snprintf(send_buffer, BUFFER_SIZE, "%s", temp_buffer);
								if (count >= 0 && count < BUFFER_SIZE) {
									send(server_data_socket, send_buffer, strlen(send_buffer), 0);									
								}
							}
						}
		
						fclose(fin);
						count = snprintf(send_buffer, BUFFER_SIZE,"226 List transfer complete. \r\n");
										
						if (count >= 0 && count < BUFFER_SIZE) {
						bytes = send(client_socket, send_buffer, strlen(send_buffer), 0);
						printf("[DEBUG INFO] <-- %s\n", send_buffer);
						}
				}
				//--
	


		#if defined __unix__ || defined __APPLE__
				close(server_data_socket);

		#elif defined _WIN32
				closesocket(server_data_socket);
		#endif
			memset(&ipaddr46, 0, sizeof(ipaddr46));
		}
      //---
      //=================================================================================
    } // End of COMMUNICATION LOOP per CLIENT
    //=================================================================================
  
    //********************************************************************
    // CLOSE SOCKET
    //********************************************************************

#if defined __unix__ || defined __APPLE__
    close(client_socket);
#elif defined _WIN32
    closesocket(client_socket);
#endif
    printf("\nDisconnected from <<<CLIENT>>> with IP address:%s, Port:%s\n",
           clientHost, clientService);
    ; // IPv4 only, needs replacing

    //====================================================================================
  } // End of MAIN LOOP
  //====================================================================================

printf("\nSERVER SHUTTING DOWN...\n");

#if defined __unix__ || defined __APPLE__
  close(s);

#elif defined _WIN32
closesocket(listening_socket);
  WSACleanup();
#endif
  return 0;
}

