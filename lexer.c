#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define unsigned long long ull

// 定义c语言所有保留字数组，字母从a开始
static char *key[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double",
                      "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
                      "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
                      "union", "unsigned", "void", "volatile", "while"}; // length = 32

// 定义c语言运算符
static char *op[] = {"+", "-", "*", "/", "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=",
                     ">", "<", ">=", "<=", "&", "|", "!", "&&", "||", "^", "~", "<<", ">>", "?", ":",
                     ",", ";", "(", ")", "[", "]", "{", "}", "\"", "\'"}; // length = 42

// 定义c语言其他字符
static char *other[] = {"identifier", "number", "string", "character", "comment", "other"}; // length = 6

enum
{
    // 保留字
    _AUTO = 1,
    _BREAK,
    _CASE,
    _CHAR,
    _CONST,
    _CONTINUE,
    _DEFAULT,
    _DO,
    _DOUBLE,
    _ELSE,
    _ENUM,
    _EXTERN,
    _FLOAT,
    _FOR,
    _GOTO,
    _IF,
    _INT,
    _LONG,
    _REGISTER,
    _RETURN,
    _SHORT,
    _SIGNED,
    _SIZEOF,
    _STATIC,
    _STRUCT,
    _SWITCH,
    _TYPEDEF,
    _UNION,
    _UNSIGNED,
    _VOID,
    _VOLATILE,
    _WHILE,
    // 运算符
    PLUS,
    MINUS,
    STAR,
    DIV,
    MOD,
    PLUSPLUS,
    MINUSMINUS,
    ASSIGN,
    PLUSEQUAL,
    MINUSEQUAL,
    STAREQUAL,
    DIVEQUAL,
    MODEQUAL,
    EQUAL,
    NOTEQUAL,
    GREAT,
    LESS,
    GREATEQUAL,
    LESSEQUAL,
    AND,
    OR,
    NOT,
    ANDAND,
    OROR,
    BITXOR,
    BITNOT,
    LEFTMOVE,
    RIGHTMOVE,
    QUESTION,
    COLON,
    COMMA,
    SEMICOLON,
    LPARENT,
    RPARENT,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    DOUBLEQUOTE,
    SINGLEQUOTE,
    // 常量
    INTCON,
    FLOATCON,
    CHARCON,
    STRCON,
    // 标识符
    IDENFR,
    // 异常
    _ERROR,
};

char sourceCode[1000000];

// 定义符号表计数器
int global_id_count = 0;
int curLine = 1;
int curCol = 1;
// 记录错误集
char errorList[10000][100] = {""};
// 记录错误个数
int errorCount = 0;
// 记录变量名哈希值
int global_id_hash[10000] = {0};

// 判断是否是保留字
int isKeyword(char s[])
{
    for (int i = 0; i < 32; i++)
        if (strcmp(s, key[i]) == 0)
            return i + 1;
    return 0;
}

// 判断是否为字母或下划线
int isLetter(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        return 1;
    return 0;
}

// 判断某位是否为数字
int isDigit(char c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

int isIdentifier(char *c)
{
    if (isLetter(*c))
    {
        c++;
        while (isLetter(*c) || isDigit(*c))
            c++;
        if (*c == '\0')
            return 1;
    }
    return 0;
}

int isDecDigit(char *c)
{
    for (int i = 0; i < strlen(c); i++)
        if (!isDigit(c[i]))
            return 0;
    return 1;
}

// 判断是否为十六进制数字
int isHexDigit(char *c)
{
    if (c[0] == '0' && !(c[1] == 'x' || c[1] == 'X'))
        return 0;

    for (int i = 2; i < strlen(c); i++)
        if (!isDigit(c[i]) && (c[i] < 'a' || c[i] > 'f') && (c[i] < 'A' || c[i] > 'F'))
            return 0;
    return 1;
}

// 判断是否为八进制数字
int isOctDigit(char *c)
{
    if (c[0] != '0')
        return 0;

    for (int i = 1; i < strlen(c); i++)
        if (c[i] < '0' || c[i] > '7')
            return 0;
    return 1;
}
// 通过DFA判断是否为科学计数法浮点数
int isFloat(char *c)
{
    int state = 0;
    for (int i = 0; i < strlen(c); i++)
    {
        switch (state)
        {
        case 0:
            if (c[i] == '+' || c[i] == '-')
                state = 1;
            else if (isDigit(c[i]))
                state = 2;
            else
                return 0;
            break;
        case 1:
            if (isDigit(c[i]))
                state = 2;
            else
                return 0;
            break;
        case 2:
            if (isDigit(c[i]))
                state = 2;
            else if (c[i] == '.')
                state = 3;
            else if (c[i] == 'e' || c[i] == 'E')
                state = 5;
            else
                return 0;
            break;
        case 3:
            if (isDigit(c[i]))
                state = 4;
            else
                return 0;
            break;
        case 4:
            if (isDigit(c[i]))
                state = 4;
            else if (c[i] == 'e' || c[i] == 'E')
                state = 5;
            else
                return 0;
            break;
        case 5:
            if (c[i] == '+' || c[i] == '-')
                state = 6;
            else if (isDigit(c[i]))
                state = 7;
            else
                return 0;
            break;
        case 6:
            if (isDigit(c[i]))
                state = 7;
            else
                return 0;
            break;
        case 7:
            if (isDigit(c[i]))
                state = 7;
            else
                return 0;
            break;
        }
    }
    return 1;
}

int isOperatorSign(char c)
{
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || c == '>' || c == '<' || c == '&' || c == '|' || c == '!' || c == '^' || c == '~' || c == '?' || c == ':' || c == ',' || c == ';' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '"' || c == '\'')
        return 1;
    return 0;
}

int isOperator(char *c)
{
    for (int i = 0; i < 42; i++)
        if (strcmp(c, op[i]) == 0)
            return i + 33;
    return 0;
}

// 预编译，过滤所有注释、空格、制表符
int filterStopWord(char *r, int totLen)
{
    char rawCode[100000] = "";
    int pos = 0;  // 记录当前字符位置
    int line = 0; // 行号
    int col = 0;  // 列号
    for (int i = 0; i < totLen; i++)
    {
        if (r[i] == '/' && r[i + 1] == '/')
        {
            while (r[i] != '\n')
                i++;
            line++;
        }
        if (r[i] == '#')
        {
            while (r[i] != '\n')
                i++;
            line++;
            i --;
        }
        else if (r[i] == '\n')
        {
            line++;
            col = 1;
            rawCode[pos++] = r[i];
        }
        else if (r[i] == '\r' || r[i] == '\t')
            col++;
        else if (r[i] == '/' && r[i + 1] == '*')
        {
            while (!(r[i] == '*' && r[i + 1] == '/'))
            {
                i++;
                int originCol = col;
                int sourceLine = line;
                if (r[i] == '\n')
                {
                    sourceLine++;
                    rawCode[pos++] = r[i];
                }
                // 判断是否有未闭合的注释
                if (i == totLen - 1)
                {
                    // 将未闭合的注释和其行号、列号记录到错误集中
                    char exception[100] = "ERROR: line ";
                    // 将行号拼接至错误集中
                    char temp[10];
                    sprintf(temp, "%d", sourceLine);
                    strcat(exception, temp);
                    strcat(exception, " col ");
                    // 将列号拼接至错误集中
                    sprintf(temp, "%d", originCol);
                    strcat(exception, temp);
                    strcat(exception, ": comments should be closed");
                    // 将错误集添加至错误集合中
                    strcpy(errorList[errorCount], exception);
                    printf("%s", exception);
                    errorCount++;
                    rawCode[i++] = '\0';
                    exit(0);
                }
            }
            i++;
        }
        else if (r[i] == ' ' && r[i + 1] == ' ')
        {
            col++;
        }
        else
        {
            rawCode[pos++] = r[i];
            col++;
        }
    }
    rawCode[pos] = '\0';
    strcpy(r, rawCode);
    return 1;
}

// 分析子程序，tokenlize
void tokenize(int *codePos, char *sourceCode, char *token, int *tokenType)
{
    int tokenLen = 0;
    char start = sourceCode[*codePos];

    int col = curCol;
    int line = curLine;

    int tmpCol = col;
    int tmpLine = line;

    strcpy(token, "");
    if (start == '\n')
    {
        line++;
        col = 1;
        *tokenType = -2;
        *codePos += 1;
    }
    else if (isLetter(start))
    {
        while (isLetter(sourceCode[*codePos]) || isDigit(sourceCode[*codePos]))
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
        }
        token[tokenLen] = '\0';
        int flag = isKeyword(token);
        *tokenType = flag ? flag : IDENFR;
    }
    else if (isDigit(start))
    {
        while (isDigit(sourceCode[*codePos]) || isLetter(sourceCode[*codePos]))
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
        }
        token[tokenLen] = '\0';
        // 判断是否为16进制数
        if (isHexDigit(token))
            *tokenType = INTCON;
        else if (isOctDigit(token))
            *tokenType = INTCON;
        else if (isDecDigit(token))
            *tokenType = INTCON;
        else if (isFloat(token))
            *tokenType = FLOATCON;
        else
        {
            errorCount++;
            *tokenType = _ERROR;
            char exception[100] = "ERROR: line ";
            char temp[10];
            sprintf(temp, "%d", tmpLine);
            strcat(exception, temp);
            strcat(exception, " col ");
            sprintf(temp, "%d", tmpCol);
            strcat(exception, temp);
            strcat(exception, ": illegal number: ");
            strcat(exception, token);
            strcpy(errorList[errorCount], exception);
            printf("%s\n", exception);
            return;
        }
    }
    else if (start == ' ')
    {
        *codePos += 1;
        col++;
        *tokenType = -1;
    }
    else if (start == '\'')
    {
        if (sourceCode[(*codePos) + 1] == '\\' && sourceCode[(*codePos) + 3] == '\'')
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen] = '\0';
            *tokenType = CHARCON;
            col += 4;
            return;
        }

        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen] = '\0';
        col += 3;
        if (token[2] != '\'')
        {
            errorCount++;
            *tokenType = _ERROR;
            char exception[100] = "ERROR: line ";
            char temp[10];
            sprintf(temp, "%d", tmpLine);
            strcat(exception, temp);
            strcat(exception, " col ");
            sprintf(temp, "%d", tmpCol);
            strcat(exception, temp);
            strcat(exception, ": char should be enclosed by single quotes");
            strcpy(errorList[errorCount], exception);
            // printf("%s\n", exception);
            return;
        }
        *tokenType = CHARCON;
    }
    else if (start == '\"')
    {
        token[tokenLen++] = sourceCode[(*codePos)++];
        col++;
        while (sourceCode[*codePos] != '\"')
        {
            if (sourceCode[*codePos] == '\\')
            {
                token[tokenLen++] = sourceCode[(*codePos)++];
                col++;
            }

            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
            // 若字符串未闭合
            if (*codePos >= strlen(sourceCode))
            {
                errorCount++;
                *tokenType = _ERROR;
                char exception[100] = "ERROR: line ";
                char temp[10];
                sprintf(temp, "%d", tmpLine);
                strcat(exception, temp);
                strcat(exception, " col ");
                sprintf(temp, "%d", tmpCol);
                strcat(exception, temp);
                strcat(exception, ": string should be closed with double quotes");
                strcpy(errorList[errorCount], exception);
                printf("%s\n", exception);
                return;
            }
        }
        token[tokenLen++] = sourceCode[(*codePos)++];
        token[tokenLen] = '\0';
        col++;
        *tokenType = STRCON;
    }
    else if (isOperatorSign(start))
    {
        // 若该位是符号，且下一位也是符号
        if (sourceCode[*codePos + 1] == '+' && sourceCode[*codePos + 1] == '-' && sourceCode[*codePos + 1] == '=' && sourceCode[*codePos + 1] == '>' && sourceCode[*codePos + 1] == '<' && sourceCode[*codePos + 1] == '&' && sourceCode[*codePos + 1] == '|' && sourceCode[*codePos + 1] == '\'' && sourceCode[*codePos + 1] == '\"')
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            token[tokenLen++] = sourceCode[(*codePos)++];
            col += 2;
        }
        else
        {
            token[tokenLen++] = sourceCode[(*codePos)++];
            col++;
        }
        token[tokenLen] = '\0';
        int flag = isOperator(token);
        if (flag)
        {
            *tokenType = flag;
        }
        else
        {
            errorCount++;
            *tokenType = _ERROR;
            char exception[100] = "ERROR: line ";
            char temp[10];
            sprintf(temp, "%d", tmpLine);
            strcat(exception, temp);
            strcat(exception, " col ");
            sprintf(temp, "%d", tmpCol);
            strcat(exception, temp);
            strcat(exception, ": ilegal operator: ");
            strcat(exception, token);
            strcpy(errorList[errorCount], exception);
            printf("%s\n", exception);
            return;
        }
    }
    else if (start == EOF || start == '\0')
    {
        *tokenType = 0;
    }
    else
    {
        errorCount++;
        *tokenType = _ERROR;
        char exception[100] = "ERROR: line ";
        char temp[10];
        sprintf(temp, "%d", tmpLine);
        strcat(exception, temp);
        strcat(exception, " col ");
        sprintf(temp, "%d", tmpCol);
        strcat(exception, temp);
        strcat(exception, ": illegal word: ");
        char temp2[2];
        temp2[0] = start, temp2[1] = '\0';
        strcat(exception, temp2);
        strcpy(errorList[errorCount], exception);
        printf("%s\n", exception);
        return;
    }
    // 更新行列号
    curLine = line;
    curCol = col;
}

// 判断该串是否为指定的关键字，若是则继续读一个token，否则报错退出
void assert(int wanted_tk, int *codePos, char *sourceCode, char *token, int *tokenType)
{
    if (*tokenType != wanted_tk)
    {
        // printf("%d  %s", *tokenType, token);
        printf("%d", IDENFR);
        printf("line %lld, col %lld: expect token: %d, get: %s\n", curLine, curCol, wanted_tk, token);
        exit(-1);
    }
    // 防止读空格
    do
    {
        tokenize(codePos, sourceCode, token, tokenType);
    } while (*tokenType <= 0);
}

// 判断变量是否重复
void check_new_id(char *token)
{
    int hash = 0, P = 1;
    // 计算哈希值
    for (int i = 0; i < strlen(token); i++)
    {
        hash += (token[i] * P) % 1610612741;
        P = P * 131 % 1610612741;
    }
    printf("%d\n", hash);
    // 判断是否重复
    for (int i = 0; i < global_id_count; i++)
    {
        if (global_id_hash[i] == hash)
        {
            printf("line %lld, col %lld: variable %s has been declared\n", curLine, curCol, token);
            exit(-1);
        }
    }
    // 添加到全局变量表
    global_id_hash[global_id_count++] = hash;
}

// 处理枚举enum的文法
void parse_enum(int *codePos, char *sourceCode, char *token, int *tokenType)
{
    int enum_val = 0;
    while (*tokenType != RBRACE)
    {
        check_new_id(token);
        assert(IDENFR, codePos, sourceCode, token, tokenType);
        if (*tokenType == ASSIGN)
        {
            assert(ASSIGN, codePos, sourceCode, token, tokenType);
            assert(INTCON, codePos, sourceCode, token, tokenType);
        }
        if (*tokenType == COMMA)
            // 防止读空格
            do
            {
                tokenize(codePos, sourceCode, token, tokenType);
            } while (*tokenType <= 0);
    }
}

void parser(char *sourceCode)
{
    int codePos = 0;
    int tokenType = 1;
    char token[100];
    int line = 1;
    int col = 1;
    while (tokenType != 0)
    {   
        // 防止读空格
        do
        {
            tokenize(&codePos, sourceCode, token, &tokenType);
        } while (tokenType < 0);
        // tokenize(&codePos, sourceCode, token, &tokenType);
        if (tokenType == _ENUM)
        {
            // 防止读空格
            do
            {
                tokenize(&codePos, sourceCode, token, &tokenType);
            } while (tokenType < 0); // 取得一个新字符
            if (tokenType != LBRACE)
                assert(IDENFR, &codePos, sourceCode, token, &tokenType);

            assert(LBRACE, &codePos, sourceCode, token, &tokenType);
            parse_enum(&codePos, sourceCode, token, &tokenType);
            assert(RBRACE, &codePos, sourceCode, token, &tokenType);
        }
    }
}

int main()
{
    // 读取源代码
    FILE *fp = fopen("lexer.c", "r");
    if (fp == NULL)
    {
        printf("ERROR: file not found");
        return 0;
    }
    char ch;
    int codeLen = 0;
    while ((ch = fgetc(fp)) != EOF)
        sourceCode[codeLen++] = ch;
    // printf("%d", codeLen);
    int flag = filterStopWord(sourceCode, strlen(sourceCode));
    if (!flag)
    {
        printf("预编译失败！");
        return 0;
    }
    // printf("%s", sourceCode);
    parser(sourceCode);
    // char token[100];
    // int tokenType;
    // int codePos = 0;
    // while (codePos < strlen(sourceCode))
    // {
    //     tokenize(&codePos, sourceCode, token, &tokenType);
    //     if (tokenType)
    //     {
    //         global_id_count++;
    //         if (tokenType <= 29)
    //             printf("Reserved Word: %s\n", token);
    //         else if (tokenType <= 74)
    //             printf("Operator: %s\n", token);
    //         else if (tokenType <= 78)
    //             printf("Literal: %s\n", token);
    //         else if (tokenType == 79)
    //             printf("Identifier: %s\n", token);
    //         else
    //             break;
    //     }
    // }
    return 0;
}
