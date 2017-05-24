#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "DNS.h"

int main(int argc, char *argv[])
{
  FILE *file;
  int socket_id, n_bytes;
  Header header;
  Query query;
  struct addrinfo hints, *server_addr;
  struct sockaddr_storage client_addr;
  socklen_t addrLen = sizeof (client_addr);
  char buffer[BUFFER_SIZE], client_ip[INET6_ADDRSTRLEN], response[INET6_ADDRSTRLEN];

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

  if (getaddrinfo(NULL, "5000", &hints, &server_addr) != 0)
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
    if ((n_bytes = recvfrom(socket_id, buffer, BUFFER_SIZE, 0,
                           (struct sockaddr *) &client_addr, &addrLen)) == -1)
    {
      perror("Error en recvfrom");
      return EXIT_FAILURE;
    }

    printf("listener: got packet from %s\n", inet_ntop(AF_INET,
           get_in_addr((struct sockaddr *)&client_addr), client_ip,
           sizeof(client_ip)));

    printf("listener: packet is %zu bytes long\n", sizeof(buffer));
    decode(buffer, &header, &query);
    printf("listener: packet contains \"%s\"\n", query.q_name);
    resolver(file, query.q_name, response);
      //redirect(query.q_name, response);

    printf("response: %s name: %s\n", response, query.q_name);

    memset(buffer, 0, BUFFER_SIZE);
    n_bytes = code(buffer, header, query,  response);

    printf("envio %d bytes\n", n_bytes);
    sendto(socket_id, buffer, n_bytes, 0, (struct sockaddr *) &client_addr, addrLen);
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

int get16bits(char **buffer)
{
  uint16_t value = (uint8_t)(*buffer)[0];
  value = value << 8;
  value += (uint8_t)(*buffer)[1];
  *buffer += 2;

  return value;
}

void put16bit(char *&buffer, unsigned int value)
{
  buffer[0] = (unsigned char)(value>>8);
  buffer[1] = value & 0xff;
  buffer += 2;
}

void put32bit(char *&buffer, unsigned long value)
{
  buffer[0] = (value & 0xff000000UL) >> 24;
  buffer[1] =(value & 0x00ff0000UL) >> 16;
  buffer[2] =  (value & 0x0000ff00UL) >>  8;
  buffer[3] = (value & 0x000000ffUL)      ;

  buffer += 4;
}

int find(char str[], char substr, int index)
{
  int found = 0, end = 0;
  while (!found && !end)
  {
    if (str[index] == substr)
      found = 1;
    else
      if (str[index] == '\0' || str[index] == '\n')
        end = 1;
    index++;
  }
  if (found)
    return index - 1;
  else
    return -1;
}

void invert_string(char *str, char *resp)
{
  //int pos_n = ip.(".in-addr.arpa");
  int length = strlen(str), index = 0, end, aux = length - 1;
  while ((end = find(str, '.', index)) != -1)
  {
    for (int i = end - 1; i>=index; i--)
    {
      resp[aux] = str[i];
      aux--;
    }
    resp[aux] = '.';
    aux--;
    index = end + 1;
  }
  for (int i=length - 1; i>=index; i--){
    resp[aux] = str[i];
    aux--;
  }
  resp[length + 1] = '\0';
  printf("%s\n", resp);
}

void decode(char *buffer, Header *header, Query *query)
{
  uint m_qType, m_qClass;
  decode_header(buffer, header);
  buffer += HDR_OFFSET;

  decode_qname(&buffer, query->q_name);

  query->q_type = get16bits(&buffer);
  query->q_class = get16bits(&buffer);
}

void decode_header(char *buffer, Header *header)
{
  header->m_id = get16bits(&buffer);
  printf("%d\n", header->m_id);
  uint fields = get16bits(&buffer);
  header->m_qr = fields & QR_MASK;
  header->m_opcode = fields & OPCODE_MASK;
  header->m_aa = fields & AA_MASK;
  header->m_tc = fields & TC_MASK;
  header->m_rd = fields & RD_MASK;
  header->m_ra = fields & RA_MASK;

  header->m_qdCount = get16bits(&buffer);
  header->m_anCount = get16bits(&buffer);
  header->m_nsCount = get16bits(&buffer);
  header->m_arCount = get16bits(&buffer);
}

void decode_qname(char **buffer, char name[])
{
    int length = *(*buffer), i, indice = 0;
    (*buffer)++;
    while (length != 0)
    {
        for ( i = 0; i < length; i++)
        {
            char c = *(*buffer);
            name[indice] = c;
            indice++;
            (*buffer)++;
        }
        length = *(*buffer);
        (*buffer)++;
        if (length != 0)
        {
          name[indice] = '.';
          indice++;
        }
    }
    name[indice] = '\0';
}

void resolver(FILE *file, char name[NAME], char response[INET6_ADDRSTRLEN])
{
  int found = 0;
  char buffer[LINE], *aux;
  struct addrinfo *results;

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
          aux[strlen(name)] = '\0';
          if (strcmp(name, aux) == 0)
            found = 1;
        }
      }
    }
  }
  rewind(file);
  if (found != 1)
    strcpy(response, "Error");
  /*{
    char aux[INET_ADDRSTRLEN];
    if (getaddrinfo(name, NULL, NULL, &results) != 0)
    if(inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&results), aux, sizeof(aux)) == NULL)
      strcpy(response, "Error");
    else
      strcpy(response, aux);
    freeaddrinfo(results);
  }*/
}

int code(char *buffer, Header header, Query query, char response[INET6_ADDRSTRLEN])
{
  struct sockaddr_in aux;
  unsigned long ttl = (unsigned long) 150;
  char *begin = buffer;

  if (strcmp(response, "Error") == 0)
    header.m_rcode = 3;
  else
    header.m_rcode = 0;
  code_hdr(buffer, header);
  buffer += HDR_OFFSET;

  code_domain(buffer, query.q_name);
  put16bit(buffer, query.q_type);
  put16bit(buffer, query.q_class);

  if (strcmp(response, "Error") != 0)
  {
    code_domain(buffer, query.q_name);
    put16bit(buffer, query.q_type);
    put16bit(buffer, query.q_class);
    put32bit(buffer, ttl);
    put16bit(buffer, 4);
    inet_pton(AF_INET, response, &(aux.sin_addr));
    put32bit(buffer, aux.sin_addr.s_addr);
  }

  return buffer - begin;
}

void code_hdr(char *buffer, Header header)
{
  put16bit(buffer, header.m_id);

  unsigned int fields = ((1) << 15);
  fields += (0 << 11);
  fields += (header.m_aa << 10);
  fields += (header.m_tc << 9);
  fields += (header.m_rd << 8);
  fields += ( 0 << 7);
  fields += ( 0 << 4);
  fields += header.m_rcode;
  put16bit(buffer, fields);

  put16bit(buffer, 1);
  put16bit(buffer, 1);
  put16bit(buffer, 0); //Ar
  put16bit(buffer, 0); //NS
}

void code_domain(char *&buffer, char *domain)
{
  int start = 0, end = 0, size; // indexes
  size = strlen(domain);

  while ((end = find(domain, '.', start)) != -1)
  {
    *buffer++ = end - start;
    for (int i=start; i<end; i++)
    {
      *buffer++ = domain[i];
    }
    start = end + 1;
  }

  *buffer++ = size - start;
  for (int i=start; i<size; i++){
    *buffer++ = domain[i];
  }
  *buffer++ = 0;
}
