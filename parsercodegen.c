// Carlos Cuns, Wilson Gil
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SIZE 100
#define PROG_SIZE 1000
#define MAX_LEX 100
#define NOT_USED NULL

char *reserved[] = {
    NULL, "odd", NULL, NULL, "+", "-", "*", "/", NOT_USED, "=", "!=", "<", "<=", ">", ">=", "(",
    ")", ",", ";", ".", ":=", "begin", "end", "if", "then", "while", "do", NOT_USED, "const",
    "var", NOT_USED, "write", "read", NOT_USED};

int progLen = 0;

// returns 1 if lexeme si a number, 0 if it is not
int isNum(char *lexeme)
{
    if (lexeme == NULL)
        return 0;

    int len = strlen(lexeme);

    for (int i = 0; i < len; i++)
    {
        if (!isdigit(lexeme[i]))
            return 0;
    }

    return 1;
}
// returns index of reserved word or -1 if is not
int isROrS(char *lexeme)
{
    if (lexeme == NULL)
        return -1;

    for (int i = 1; i <= 33; i++)
    {
        if (reserved[i] == NULL)
            continue;
        if (strcmp(lexeme, reserved[i]) == 0)
            return i;
    }

    return -1;
}
// returns 1 if lexeme is a proper identifier, 0 otherwise
int isIden(char *lexeme)
{
    if (lexeme == NULL)
        return 0;

    int len = strlen(lexeme);

    if (len == 0 || !isalpha(lexeme[0]))
        return 0;

    for (int i = 1; i < len; i++)
    {
        if (!(isalpha(lexeme[i]) || isdigit(lexeme[i])))
            return 0;
    }

    return 1;
}
// takes a set of lexemes and labels each in a parallel array of size progLen
int *createTokenArr(char **lexems)
{
    if (progLen == 0)
        return NULL;

    int *tokenArr = malloc(sizeof(int) * progLen);

    for (int i = 0; i < progLen; i++)
    {
        char *lex = lexems[i];

        // check if lex is reserved, then an identifier then a number (check for proper length)
        if (isROrS(lex) > 0)
        {
            tokenArr[i] = isROrS(lex);
            continue;
        }
        else if (isIden(lex))
        {
            if (strlen(lex) <= 11)
                tokenArr[i] = 2;
            else
                tokenArr[i] = -1; // too long (identifier)
            continue;
        }
        else if (isNum(lex))
        {
            if (strlen(lex) <= 5)
                tokenArr[i] = 3;
            else
                tokenArr[i] = -2; // too long (number)
            continue;
        }
        else
            tokenArr[i] = -3; // invalid symbol
    }

    return tokenArr;
}

void insertLex(char **lexems, char *workingStr, int length, int index)
{
    if (workingStr == NULL || length < 2)
        return;
    workingStr[length - 1] = '\0';
    if (strcmp(workingStr, "") == 0)
        return;
    lexems[index] = malloc(sizeof(char) * (length));
    strcpy(lexems[index], workingStr);

    return;
}
// takes in a file name and stores each lexeme into an array
void readELF(char *filename, char **lexems)
{
    char buffer;
    char workingStr[MAX_LEX];
    int number = 0;
    fpos_t pos;
    int i = 0;
    int j = 0;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("failed to open file\n");
        exit(1);
    }

    do
    {
        buffer = fgetc(file);

        // only read char if it isn't end of file, otherwise insert current lexeme
        if (feof(file))
        {
            insertLex(lexems, workingStr, j + 1, i);
            break;
        }
        // skip over blank characters and new lines
        if (buffer == ' ' || buffer == '\t' || buffer == '\n')
        {
            if (j > 0)
            {
                insertLex(lexems, workingStr, j + 1, i++);
                j = 0;
            }
            continue;
        }
        if (isalpha(buffer))
        {
            if (number)
            {
                number = 0;
                if (j > 0)
                    insertLex(lexems, workingStr, j + 1, i++);
                j = 0;
            }
            workingStr[j] = buffer;
            j++;
            continue;
        }
        else if (isdigit(buffer))
        {
            if (j == 0)
            {
                number = 1;
            }
            workingStr[j] = buffer;
            j++;
            continue;
        }
        else
        {
            number = 0;
            switch (buffer)
            {
            case ':':
                if (j > 0)
                {
                    insertLex(lexems, workingStr, j + 1, i++);
                    j = 0;
                }
                workingStr[0] = buffer;
                workingStr[1] = fgetc(file);
                insertLex(lexems, workingStr, 3, i++);
                j = 0;
                break;
            // check if these are paired symbols, if not treat as common symbols
            case '>':
            case '<':
            case '!':
                if (j > 0)
                {
                    insertLex(lexems, workingStr, j + 1, i++);
                    j = 0;
                }
                char temp = buffer;
                fgetpos(file, &pos);
                buffer = fgetc(file);
                if (buffer == '=')
                {
                    workingStr[0] = temp;
                    workingStr[1] = buffer;
                    insertLex(lexems, workingStr, 3, i++);
                }
                else
                {
                    workingStr[0] = temp;
                    insertLex(lexems, workingStr, 2, i++);
                    fsetpos(file, &pos);
                }
                j = 0;
                break;
            // check to see if this is a division or a comment
            case '/':
                temp = buffer;
                fgetpos(file, &pos);
                buffer = fgetc(file);
                if (buffer == '*')
                {
                    while (1)
                    {
                        temp = buffer;
                        buffer = fgetc(file);
                        if (temp == '*' && buffer == '/')
                            break;
                    }
                }
                else
                {
                    if (j > 0)
                    {
                        insertLex(lexems, workingStr, j + 1, i++);
                        j = 0;
                    }
                    workingStr[0] = temp;
                    insertLex(lexems, workingStr, 2, i++);
                    j = 0;
                    fsetpos(file, &pos);
                }
                break;
            // normal characters are just added to working string
            default:
                if (j > 0)
                {
                    insertLex(lexems, workingStr, j + 1, i++);
                    j = 0;
                }
                workingStr[0] = buffer;
                insertLex(lexems, workingStr, 2, i++);
                j = 0;
                break;
            }
        }
    } while (1);

    progLen = i;

    fclose(file);

    return;
}
// debugging function
void printLex(char **lexems)
{
    for (int i = 0; i < progLen; i++)
    {
        char *lex = lexems[i];
        printf("\"%s\" , isRorS %-2d, isIden %-3s, isNum %-3s\n",
               lex, isROrS(lex), isIden(lex) ? "Yes" : "No", isNum(lex) ? "Yes" : "No");
    }

    return;
}
// takes tokenArray and prints tokens with numbers and identifiers
void printTokens(int *tokenArr, char **lexems)
{
    printf("Token List:\n");
    for (int i = 0; i < progLen; i++)
    {
        if (tokenArr[i] >= 0)
            printf("%d ", tokenArr[i]);
        if (tokenArr[i] == 2 || tokenArr[i] == 3) // print out identifiers and numbers
            printf("%s ", lexems[i]);
    }
    printf("\n");

    return;
}

void printTable(int *tokenArr, char **lexems)
{
    // printf("Proglen at table: %d\n", progLen);
    printf("Lexeme Table:\n\n");
    printf("%-15s%-15s\n", "lexeme", "token type");
    for (int i = 0; i < progLen; i++)
    {
        printf("%-15s", lexems[i]);
        if (tokenArr[i] > 0) // print out identifiers and numbers
            printf("%-15d", tokenArr[i]);
        else
        {
            switch (tokenArr[i]) // print error codes
            {
            case -1:
                printf("Name too long.");
                break;
            case -2:
                printf("Number too long.");
                break;
            case -3:
                printf("Invalid Symbols.");
                break;
            }
        }
        printf("\n");
    }
    printf("\n");

    return;
}
// opens a source program and outputs it the screen
void printSource(char *filename)
{
    char buffer;
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("failed to open file");
        exit(1);
    }
    printf("Source Program:\n");
    while (fscanf(file, "%c", &buffer) != EOF)
    {
        printf("%c", buffer);
    }
    printf("\n\n");

    fclose(file);

    return;
}

// ================================================ HW 3 now =======================================
#define MAX_SYMBOL_TABLE_SIZE 500
typedef struct
{
    int kind;      // const = 1, var = 2, proc = 3
    char name[10]; // name up to 11 chars
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    int mark;      // to indicate unavailable or deleted
} symbol;

symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
int numSym = 0;
char TOKEN;
int numVars;

FILE *out;
int line = 0;

void ERROR(int code)
{
    printf("Error: ");
    switch (code)
    {
    case 1:
        printf("program must end with a period");
        break;
    case 2:
        printf("const, var, and read keywords must be followed by an identifier");
        break;
    case 3:
        printf("symbol name has already been declared");
        break;
    case 4:
        printf("constants must be assigned with =");
        break;
    case 5:
        printf("constants must be assigned an integer value");
        break;
    case 6:
        printf("constant and variable declarations must be followed by a semicolon");
        break;
    case 7:
        printf("undeclared identifier");
        break;
    case 8:
        printf("only variable values may be altered");
        break;
    case 9:
        printf("assignment statements must use :=");
        break;
    case 10:
        printf("begin must be followed by end");
        break;
    case 11:
        printf("if must be followed by then");
        break;
    case 12:
        printf("while must be followed by do");
        break;
    case 13:
        printf("condition must contain comparison operator");
        break;
    case 14:
        printf("right parenthesis must follow left parenthesis");
        break;
    case 15:
        printf("arithmetic equations must contain operands, parentheses, numbers, or symbols");
        break;
    case 16:
        printf("code too long");
        break;
    default:
        printf("error msg");
        break;
    }
    printf("\n");
    exit(1);
}

void printSymTable()
{
    printf("Symbol Table:\n\n");
    printf("%6s|%13s|%7s|%7s|%9s|%6s\n", "Kind ", "Name ", "Value ", "Level ", "Address ", "Mark ");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < numSym; i++)
    {
        symbol s = symbolTable[i];
        printf("%5d |%12s |%6d |%6d |%8d |%5d\n", s.kind, s.name, s.val, s.level, s.addr, s.mark);
    }
}

void emit(int sym, int l, int m)
{
    char *op;
    switch (sym)
    {
    case 1:
        op = "LIT";
        break;
    case 2:
        // figure out which operation is being done
        op = "OPR";
        break;
    case 3:
        op = "LOD";
        break;
    case 4:
        op = "STO";
        break;
    case 5:
        op = "CAL";
        break;
    case 6:
        op = "INC";
        break;
    case 7:
        op = "JMP";
        break;
    case 8:
        op = "JPC";
        break;
    case 9:
        op = "SYS";
        break;
    default:
        break;
    }
    fprintf(out, "%4d %5s %3d %3d", line, op, l, m);
    line++;
}

void printAssembly()
{
    char buffer;
    FILE *file = fopen("assembly.txt", "r");

    if (!file)
    {
        printf("failed to open assembly file");
        exit(1);
    }
    printf("Assembly code:\n\n");
    printf("%4s %5s %3s %3s\n", "Line", "OP", "L", "M");
    while (fscanf(file, "%c", &buffer) != EOF)
    {
        printf("%c", buffer);
    }
    printf("\n\n");

    fclose(file);

    return;
}

// int SYMBOLTABLECHECK(char *name)
// {
//     for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
//     {
//         if (strcmp(symbolTable[i].name, name) == 0)
//         {
//             return i;
//         }
//     }
//     return -1;
// }

// void PROGRAM()
// {
//     BLOCK();
//     if (TOKEN != reserved[19])
//     {
//         // err0r
//     }
//     // emit HALT
// }

// void BLOCK()
// {
//     CONST - DECLARATION();
//     numVars = VAR - DECLARATION();
//     // emit INC (M = 3 + numVars)
//     STATEMENT();
// }

// int VAR - DECLARATION()
// {
// }

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Include a file name\n");
        exit(1);
    }

    char filename[MAX_SIZE];
    char **lexems = malloc(sizeof(char *) * PROG_SIZE);

    strcpy(filename, argv[1]);
    readELF(filename, lexems);
    int *tokenArr = createTokenArr(lexems);

    // printSource(filename);
    // printTable(tokenArr, lexems);
    // printTokens(tokenArr, lexems);

    out = fopen("assembly.txt", "w");
    symbol s1, s2;
    s1.kind = 2;
    strcpy(s1.name, "x");
    s1.val = 0;
    s1.level = 0;
    s1.addr = 3;
    s1.mark = 1;
    symbolTable[0] = s1;
    s2.kind = 2;
    strcpy(s2.name, "y");
    s2.val = 0;
    s2.level = 0;
    s2.addr = 4;
    s2.mark = 1;
    symbolTable[1] = s2;
    numSym = 2;

    emit(1, 2, 3);
    fclose(out);
    printAssembly();
    printSymTable();
    return 0;
}