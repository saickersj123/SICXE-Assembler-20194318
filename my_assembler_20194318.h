#include <stdio.h>
#include <stdlib.h>	
#include <string.h>
/* 
 * my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다. 
 * 
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

typedef unsigned char uchar;

/* 
 * instruction 목록 파일로 부터 정보를 받아와서 생성하는 구조체 변수이다.
 * 라인 별로 하나의 instruction을 저장한다.
 */
struct inst_unit
{
	uchar str[10];
	int ops; //	0:-		1:M		2:R		3:N		4:RR	5:RN
	int format; // 0:not SIC/XE code	1:format 1	2:format 2		3:format 3/4
	uchar op;
};
typedef struct inst_unit inst;
inst *inst_table[MAX_INST];
int inst_index;

/*
 * 어셈블리 할 소스코드를 입력받는 테이블이다. 라인 단위로 관리할 수 있다.
 fopen input.txt
 */
uchar *input_data[MAX_LINES];
static int line_num;

/* 
 * 어셈블리 할 소스코드를 토큰단위로 관리하기 위한 구조체 변수이다.
 * operator는 renaming을 허용한다.
 */
struct token_unit
{
	uchar *label;
	uchar *operator;
	uchar operand[MAX_OPERAND][20];
	uchar comment[100];
	// 다음과제에 사용될 변수
	// uchar nixbpe;
};


typedef struct token_unit token;
token *token_table[MAX_LINES];
static int token_line;

/*
 * 심볼을 관리하는 구조체이다.
 * 심볼 테이블은 심볼 이름, 심볼의 위치로 구성된다.
 * 추후 과제에 사용 예정
 */
struct symbol_unit
{
	uchar symbol[10];
	int addr;
};

typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];

static int locctr; // loc값 
//--------------
static FILE *intermediate_file;
static int starting_address;
static int program_length;

static uchar *input_file; // 입력 파일 input.txt
static uchar *output_file;// 출력 파일 output.txt
int init_my_assembler(void); // 어셈블러 초기화 파일 init_inst_file()과 init_input_file()도 포함
int init_inst_file(uchar *inst_file); //inst.data를 읽고 inst_table[]에 저장
int init_input_file(uchar *input_file); //intput.txt를 읽고 input_data[]에 저장
int token_parsing(uchar *str); //input_data[]에 저장된 명령어를 토큰화하여 token_table[]에 저장
int search_opcode(uchar *str); //input_data[]에 저장된 명령어의 opcode를 검색 inst_table[]을 참조
static int assem_pass1(void); //SIC/XE Pass1 make_opcode_output과 make_symtab_output을 포함
void make_opcode_output(uchar *file_name); /*Contain the mnemonic operation code & its machine language 
equivalent.
- May also contain information about instruction format and length
- Used to look up & validate mnemonic operation codes (Pass 1)
and translate them to machine language (Pass 2) 
- In SIC, both processes could be done together
- In SIC/XE, search OPTAB to find the instruction length (Pass 1), 
determine instruction format to use in assembling the instruction (Pass 2)
- Usually organized as a hash table & a static table
- Mnemonic operation code as a key
- Provide fast retrieval with a minimum searching
- A static table in most cases
- Entries are not normally added to or deleted from it
*/
void make_symtab_output(uchar *file_name); /*- Used to store values (addresses) assigned to labels
- Contain the name and value (addresses) for each label, 
together with flags for error conditions, 
also information about the data area or instruction labeled
- for example, its the type or length
- Pass 1 : labels are entered with their assigned (from LOCCTR)
- Pass 2 : symbols are used to look up in SYMTAB to obtain the 
addresses to be inserted
- Usually organized as a hash table
- For efficiency of insertion & retrieval
- Heavily used throughout the assembly*/
static int assem_pass2(void); // SIC/XE Pass2 make_objectcode_output을 포함
void make_objectcode_output(uchar *file_name); // 파일 output.txt에 입력된 명령어의 오브젝트 코드 생성


static void write_intermediate_file(uchar *line) {
    // Assuming the FILE pointer is properly initialized before calling this function
    if (intermediate_file != NULL) {
        // Write the line to the intermediate file
        fprintf(intermediate_file, "%s\n", line);
    } else {
        // Handle the case where the FILE pointer is not valid
        printf("Error: Intermediate file pointer is not valid.\n");
        // You may want to add further error-handling logic here
    }
}
static int search_symtab(uchar *label) {
    // Implement the logic to search the symbol table for the given label.
    // Return the index if found, otherwise return -1.

    for (int i = 0; i < inst_index; ++i) {
        // Compare the given label with each symbol in the symbol table
        if (strcmp(sym_table[i].symbol, label) == 0) {
            return i; // Found: return the index
        }
    }

    // If the loop completes without finding a match, return -1
    return -1; // Not found
}