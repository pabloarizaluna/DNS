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
  char q_Name[NAME];
  uint q_Type;
  uint q_Class;
} Query;
