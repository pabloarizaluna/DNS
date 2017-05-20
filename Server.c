#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "DNS.h"

void *get_in_addr(struct sockaddr *);
int get16bits(char *);
void decode(char *, Header *, char[]);
void decode_header(char *, Header *);
void decode_qname(char *, char[]);
int resolver(FILE*, char[NAME], char[INET6_ADDRSTRLEN]);
int code(char *);

int main(int argc, char *argv[])
{
  FILE* file;
  int socket_id, n_bytes;
  Header header;
  struct addrinfo hints, *server_addr;
  struct sockaddr_storage client_addr;
  socklen_t addrLen = sizeof (client_addr);
  char buffer[BUFFER_SIZE], client_ip[INET6_ADDRSTRLEN], name[NAME], response[INET6_ADDRSTRLEN];

  if (argc != 2)
  {
    printf("Error en ejecucion\n Debe ejecutarse: %s [archivo_host]", argv[0]);
    return EXIT_FAILURE;
  }

  file = fopen(argv[1], "r");
  if (file == NULL)
  {
    printf("Error en apertura de Master File\n");
    return EXIT_FAILURE;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, "4950", &hints, &server_addr) != 0)
  {
    perror("Error en getaddrinfo");
    return EXIT_FAILURE;
  }

  if ((socket_id = socket(server_addr->ai_family, server_addr->ai_socktype,
                         server_addr->ai_protocol)) == -1)
  {
    perror("Error en socket");
    return EXIT_FAILURE;
  }
  if (bind(socket_id, server_addr->ai_addr, server_addr->ai_addrlen) != 0)
  {
    close(socket_id);
    perror("Error en bind");
    return EXIT_FAILURE;
  }

  freeaddrinfo(server_addr);

  while (1)
  {
    printf("server: waiting to recvfrom...\n");
    if (n_bytes = recvfrom(socket_id, buffer, BUFFER_SIZE, 0,
                           (struct sockaddr *) &client_addr, &addrLen) == -1)
    {
      perror("Error en recvfrom");
      return EXIT_FAILURE;
    }

    printf("listener: got packet from %s\n", inet_ntop(client_addr.ss_family,
           get_in_addr((struct sockaddr *)&client_addr), client_ip,
           sizeof(client_ip)));

    printf("listener: packet is %zu bytes long\n", sizeof(buffer));
    decode(buffer, &header, name);
    printf("listener: packet contains \"%s\"\n", name);
    resolver(file, name, response);
    printf("response: %s name: %s\n", response, name);

    /*
    m_resolver.process(m_query, m_response);
    memset(buffer, 0, BUFFER_SIZE);
    n_bytes = code(buffer);*/
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

void decode(char *buffer, Header *header, char name[])
{
  uint m_qType, m_qClass;
  decode_header(buffer, header);
  buffer += HDR_OFFSET;

  decode_qname(buffer, name);

  m_qType = get16bits(buffer);
  m_qClass = get16bits(buffer);
}

void decode_header(char *buffer, Header *header)
{
  header->m_id = get16bits(buffer);

  uint fields = get16bits(buffer);
  header->m_qr = fields & QR_MASK;
  header->m_opcode = fields & OPCODE_MASK;
  header->m_aa = fields & AA_MASK;
  header->m_tc = fields & TC_MASK;
  header->m_rd = fields & RD_MASK;
  header->m_ra = fields & RA_MASK;

  header->m_qdCount = get16bits(buffer);
  header->m_anCount = get16bits(buffer);
  header->m_nsCount = get16bits(buffer);
  header->m_arCount = get16bits(buffer);
}

int get16bits(char *buffer)
{
    int value = buffer[0];

    value = value << 8;
    value += buffer[1];
    buffer += 2;

    return value;
}

void decode_qname(char *buffer, char name[])
{
    char *aux = buffer;
    int length = *aux++, i, indice = 0;

    while (length != 0)
    {
        for ( i = 0; i < length; i++)
        {
            char c = *aux++;
            name[indice] = c;
            indice++;
        }
        length = *aux++;
        if (length != 0)
        {
          name[indice] = '.';
          indice++;
        }
    }
    name[indice] = '\0';
}

int resolver(FILE* file, char name[NAME], char response[INET6_ADDRSTRLEN])
{
  int found = 0;
  char buffer[LINE], *aux;

  while (!feof(file) && !found)
  {
    fgets(buffer, LINE, file);
    if (!feof(file))
    {
      aux = strtok(buffer, " ");
      if (aux != NULL)
      {
        strcpy(response, aux);
        aux = strtok(NULL, " ");
        if (aux != NULL)
        {
          if (strstr(name, aux) != NULL)
            found = 1;
        }
        else
          return -1;
      }
      else
        return -1;
    }
  }
  if (found)
    return 0;
  else
    return -1;
}

/*int code(char* buffer)
{
    char* bufferBegin = buffer;

    code_hdr(buffer);
    buffer += HDR_OFFSET;

    // Code Question section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);

    // Code Answer section
    code_domain(buffer, m_name);
    put16bits(buffer, m_type);
    put16bits(buffer, m_class);
    put32bits(buffer, m_ttl);
    put16bits(buffer, m_rdLength);
    code_domain(buffer, m_rdata);

    int size = buffer - bufferBegin;
    log_buffer(bufferBegin, size);

    return size;
}*/
