#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define   BUFFER_SIZE     512
#define   HDR_OFFSET      12

void *get_in_addr(struct sockaddr *sa);
int get16bits(const char*);
void decode(const char*);
void decode_qname(const char* buffer);
char *respuesta(char[INET6_ADDRSTRLEN]);

int main(int argc, char* argv[])
{
  int socket_id, n_bytes;
  struct addrinfo hints, *server_addr;
  struct sockaddr_storage client_addr;
  char buffer[BUFFER_SIZE], client_ip[INET6_ADDRSTRLEN];
  socklen_t addrLen = sizeof (client_addr);

  if(argc != 2)
  {
    printf("Error en ejecucion\n Debe ejecutarse: %s [archivo_host]", argv[1]);
    return EXIT_FAILURE;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if(getaddrinfo(NULL, "4950", &hints, &server_addr) != 0)
  {
    perror("Error en getaddrinfo");
    return EXIT_FAILURE;
  }

  if((socket_id = socket(server_addr->ai_family, server_addr->ai_socktype,
                         server_addr->ai_protocol)) == -1)
  {
    perror("Error en socket");
    return EXIT_FAILURE;
  }
  if(bind(socket_id, server_addr->ai_addr, server_addr->ai_addrlen) != 0)
  {
    close(socket_id);
    perror("Error en bind");
    return EXIT_FAILURE;
  }
  printf("listener: got packet from %s\n",
    inet_ntop(server_addr->ai_family,	get_in_addr((struct sockaddr *)&server_addr),
              client_ip, sizeof(client_ip)));
  freeaddrinfo(server_addr);

  while(1)
  {
    printf("server: waiting to recvfrom...\n");
    if( n_bytes = recvfrom(socket_id, buffer, BUFFER_SIZE, 0,
                           (struct sockaddr *) &client_addr, &addrLen) == -1)
    {
      perror("Error en recvfrom");
      return EXIT_FAILURE;
    }

    printf("listener: got packet from %s\n",
  		inet_ntop(client_addr.ss_family,	get_in_addr((struct sockaddr *)&client_addr),
  			        client_ip, sizeof(client_ip)));

    printf("listener: packet is %zu bytes long\n", sizeof(buffer));
    //buffer[n_bytes] = '\0';
    printf("listener: packet contains \"%s\"\n", buffer);
    //decode(buffer, n_bytes);
    /*
    m_resolver.process(m_query, m_response);

    m_response.asString();
    memset(buffer, 0, BUFFER_SIZE);
    n_bytes = m_response.code(buffer);*/
    //sendto(socket_id, buffer, n_bytes, 0, (struct sockaddr *) &client_addr, addrLen);
    }

  return EXIT_SUCCESS;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void decode(const char* buffer)
{
  buffer += HDR_OFFSET;

  decode_qname(buffer);

  m_qType = get16bits(buffer);
  m_qClass = get16bits(buffer);
}

int get16bits(const char* buffer)
{

    int value = buffer[0];
    value = value << 8;
    value += buffer[1];
    buffer += 2;

    return value;
}

void decode_qname(const char* buffer){

    m_qName.clear();

    int length = *buffer++;
    while (length != 0) {
        for (int i = 0; i < length; i++) {
            char c = *buffer++;
            m_qName.append(1, c);
        }
        length = *buffer++;
        if (length != 0) m_qName.append(1,'.');
    }
}
