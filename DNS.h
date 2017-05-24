#define   BUFFER_SIZE     512
#define   HDR_OFFSET      12
#define   NAME            40
#define   LINE            100
#define   QR_MASK         0x8000;
#define   OPCODE_MASK     0x7800;
#define   AA_MASK         0x0400;
#define   TC_MASK         0x0200;
#define   RD_MASK         0x0100;
#define   RA_MASK         0x8000;
#define   RCODE_MASK      0x000F;

typedef struct Header
{
  uint m_id;
  uint m_qr;
  uint m_opcode;
  uint m_aa;
  uint m_tc;
  uint m_rd;
  uint m_ra;
  uint m_rcode;

  uint m_qdCount;
  uint m_anCount;
  uint m_nsCount;
  uint m_arCount;
} Header;

typedef struct Query
{
  char q_name[NAME];
  uint q_type;
  uint q_class;
  int length_query;
} Query;


void *get_in_addr(struct sockaddr *);
int get16bits(char **);
void put16bit(char *&buffer, unsigned int value);
void put32bits(char **, long);
int find(char[], char , int);
void invert_string(char *str, char *resp);
void decode(char *, Header *, Query *);
void decode_header(char *, Header *);
void decode_qname(char **, char[]);
void resolver_a(FILE *, char[NAME], char[INET6_ADDRSTRLEN]);
void resolver_ptr(FILE *, char [NAME], char [INET6_ADDRSTRLEN]);
void redirect(char *, char[]);
int code(char *, Header, Query, char[INET6_ADDRSTRLEN]);
void code_hdr(char *, Header);
void code_domain(char *&buffer, char *);
