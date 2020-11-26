//===================================================================================================================
//  casm1.cc -- the main program of the 1-pass assembler
//
//  ----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Programmer  Description
//  -----------  -------  -------  ----------  ---------------------------------------------------------------------
//  2020-Nov-25           Prj 1-1  ADCL        Initial Version
//
//===================================================================================================================


#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <cstring>

#define yes true
#define no false

#define M 1024
#define N 16
#define base_t uint16_t


//
// -- Output error/warning functions
//    ==============================


//
// -- define some tallies
//    -------------------
int errorCnt = 0;
int warningCnt = 0;


//
// -- Issue a fatal error message and exit
//    ------------------------------------
__attribute__((noreturn))
int fatal(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, "FATAL: ");
    vfprintf (stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    exit(EXIT_FAILURE);
}


//
// -- Issue an error message and continue
//    -----------------------------------
int error(const char *fmt, ...)
{
    va_list args;
    int rv = 0;

    va_start(args, fmt);
    rv = fprintf(stderr, "ERROR: ");
    rv += vfprintf(stderr, fmt, args);
    rv += fprintf(stderr, "\n");
    va_end(args);

    errorCnt ++;

    return rv;
}


//
// -- Issue a warning message and continue
//    ------------------------------------
int warning(const char *fmt, ...)
{
    va_list args;
    int rv = 0;

    va_start(args, fmt);
    rv = fprintf(stderr, "WARNING: ");
    rv += vfprintf(stderr, fmt, args);
    rv += fprintf(stderr, "\n");
    va_end(args);

    warningCnt ++;

    return rv;
}


// ------------------------------------------------------------------------------------------------------------------


//
// -- Symbol Table
//    ============


//
// -- A symbol in the table
//    ---------------------
struct symData_t {
    char stat;
    uint16_t value;
};


//
// -- The actual table (one entry for each letter)
//    --------------------------------------------
struct symData_t symTable[26] = {0};


//
// -- Search the table for the status of the symbol
//    ---------------------------------------------
char symbolSearch(char label)
{
    label = toupper(label);
    if (label < 'A' || label > 'Z') fatal("`symbolSearch()` on an invalid symbol (out of range)");

    return symTable[label - 'A'].stat;
}


//
// -- Get the value from the symbol table
//    -----------------------------------
uint16_t symbolGetValue(char label)
{
    label = toupper(label);
    if (label < 'A' || label > 'Z') fatal("`symbolGetValue()` on an invalid symbol (out of range)");

    if (symTable[label - 'A'].stat == 0) return (uint16_t)-1;
    else return symTable[label - 'A'].value;
}


//
// -- Add a defined symbol at the proper location
//    -------------------------------------------
bool symbolAddDefined(char label, uint16_t val)
{
    label = toupper(label);
    if (label < 'A' || label > 'Z') fatal("`symbolAddDefined()` on an invalid symbol (out of range)");

    if (symTable[label - 'A'].stat == 0) {
        symTable[label - 'A'].stat = 'D';
        symTable[label - 'A'].value = val;

        return true;
    } else if (symTable[label - 'A'].stat == 'U') {
        error("Unimplemented future: defined label unrolling");
        // TODO: handle unrolling the future-defined labels

        return true;
    } else return false;
}


//
// -- Add an undefined symbol at the proper location
//    ----------------------------------------------
bool symbolAddUndefined(char label)
{
    label = toupper(label);
    if (label < 'A' || label > 'Z') fatal("`symbolAddUndefined()` on an invalid symbol (out of range)");

    if (symTable[label - 'A'].stat == 0) {
        symTable[label - 'A'].stat = 'U';
        symTable[label - 'A'].value = lc;

        return true;
    } else if (symTable[label - 'A'].stat == 'U') {
        error("Unimplemented future: pushing additional undefined label");
        // TODO: handle pushing additional undefined label location

        return true;
    } else {
        fatal("Trying to add an undefined label; but it is defined");
    }
}


// ------------------------------------------------------------------------------------------------------------------


//
// -- Assembly OpCode Table
//    =====================


//
// -- The Opcode Table Entry
//    ----------------------
struct OpCode_t {
    const char *mnemonic;
    uint8_t opcode;
    bool hasOperand;
};


//
// -- The actual OpCode Table
//    -----------------------
struct OpCode_t opCodeTable[] = {
    {"LOD", 1, yes},
    {"STO", 2, yes},
    {"ADD", 3, yes},
    {"BZE", 4, yes},
    {"BNE", 5, yes},
    {"BRA", 6, yes},
    {"INP", 7, no},
    {"OUT", 8, no},
    {"CLA", 9, no},
    {"HLT", 0, no},
};


//
// -- This is an invalid opcode
//    -------------------------
struct OpCode_t invalidOpcode = {0};


//
// -- This calculates the size of the opcode table
//    --------------------------------------------
#define OPCODE_CNT (sizeof(opCodeTable) / sizeof(struct OpCode_t))


//
// -- Now a function to get the details of an opcode
//    ----------------------------------------------
struct OpCode_t *GetOpCode(const char *mnem)
{
    if (mnem == NULL) fatal("NULL passed into GetOpCode");

    for (int i = 0; i < OPCODE_CNT; i ++) {
        if (strcmp(mnem, opCodeTable[i].mnemonic) == 0) {
            return &opCodeTable[i];
        }
    }

    return &invalidOpcode;
}


//
// -- quick function to check if the opcode is invalid
//    ------------------------------------------------
inline bool IsInvalid(struct OpCode_t *o) { return (o == &invalidOpcode); }


// ------------------------------------------------------------------------------------------------------------------







// ------------------------------------------------------------------------------------------------------------------


//
// -- Some global variables
//    ---------------------
FILE *inp = NULL;                   // will be changed to stdin if nothing specified on the command line
base_t output[M];                   // this is the output of the assembly
base_t value;                       // the value of the operand
int lc;                             // location counter
char buf[50];                       // a reasonable limit for a line
char *mnem = &buf[2];               // the opcode always appears in col 3 (index 2)
bool labelDefined;
bool symbolUsed;


//
// -- Print the usage and exit
//    ------------------------
void PrintUsage(const char *pgm)
{
    printf("Usage: %s [-h|--help] [filename]\n", pgm);
    printf("\n");
    printf("    -h  (--help)      Print this help and exit\n");
    printf("    <filename>        Optional filename to read and assemble (if not specified, will use stdin)\n");

    exit(EXIT_SUCCESS);
}


//
// -- Parse the command line parameters and set the global variables
//    --------------------------------------------------------------
void ParseCommandLine(int argc, char *argv[])
{
    for (int i = 1; i < argc; i ++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage(argv[0]);
        }

        // -- if we get to this point, the argument is a file name
        if (inp != NULL) {
            inp = fopen(argv[i], "r");

            if (inp == NULL) fatal("Unable to open input file %s", argv[i]);
        } else {
            fatal("Multiple input files specified on the command line");
        }
    }
}


//
// -- Initialize the assembler for use
//    --------------------------------
void InitializeAssembler(void)
{
    if (inp == NULL) inp = stdin;

    lc = 0;
    memset(output, 0, M * (N / 8));
}


//
// -- Read a line from the input (return true when eof)
//    -------------------------------------------------
bool ReadLine(void)
{
    memset(buf, 0, sizeof(buf));
    char *rv = fgets(buf, sizeof(buf), inp);

    if (rv == NULL) return true;

    if (feof(inp)) {
        warning("EOF found before end of line (no trailing newline); adding a newline");
    }

    // drop any trailing newline
    if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;

    // and truncate any trailing spaces
    while (buf[strlen(buf) - 1] == ' ') buf[strlen(buf) - 1] = 0;

    return false;
}


//
// -- clean and parse the line that was read
//    --------------------------------------
void CleanParseLine(void)
{
    // first convert everything to upper case
    for (int i = 0; i < strlen(buf); i ++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') buf[i] = toupper(buf[i]);
    }

    buf[1] = buf[5] = 0;
    symbolUsed = false;

    if (buf[0] == ' ') {
        labelDefined = false;
    } else {
        if (buf[0] < 'A' || buf[0] > 'Z') {
            error("Invalid label defined: %c", buf[0]);
            labelDefined = false;
        } else {
            labelDefined = true;
        }
    }

    if (buf[6] > 'A' && buf[6] < 'Z') {
        symbolUsed = true;
    } else {
        if (buf[6]) {
            value = atoi(&buf[6]);
        } else {
            value = 0;
        }
    }

}


//
// -- Handle new label creation
//    -------------------------
void HandleNewLabel(void)
{
    symbolAddDefined(buf[0], lc);
}


//
// -- Handle the reference to a label
//    -------------------------------
void HandleSymbolUse(void)
{
    char stat = symbolSearch(buf[0]);

    if (stat == 'U') {
        symbolAddUndefined(buf[0]);
    } else if (stat == 'D') {
        value = symbolGetValue(buf[0]);
    }
}


//
// -- Assemble the line into the output
//    ---------------------------------
void AssembleLine(void)
{
    base_t word = 0;

    struct OpCode_t *op = GetOpCode(mnem);

    if (IsInvalid(op)) {
        error("Invalid Opcode %s", mnem);
    } else {
        word = (op->opcode << 10);

        if (op->hasOperand) {
            word |= (op->opcode & 0x3ff);
        }
    }
}


//
// -- This is the entry point
//    -----------------------
int main(int argc, char *argv[])
{
    // -- parse the command line and open the input file
    ParseCommandLine(argc, argv);

    // -- Initialize the assembler
    InitializeAssembler();

    // -- read a line from the file
    while (true) {
        bool eof = ReadLine();

        // -- are we done?
        if (eof) {
            CompleteAssembly();
        }

        // scrub up the line as input
        CleanParseLine();

        // did we define a label?
        if (labelDefined) {
            HandleNewLabel();
        }

        // did we use a label?
        if (symbolUsed) {
            HandleSymbolUse();
        }

        AssembleLine();
        PrintListingLine();

        // move to the next word
        lc ++;
    }
}
