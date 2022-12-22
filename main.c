#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define unsigned long long ull

/* 词法分析部分 */

// 定义c语言所有保留字数组，字母从a开始
static char *key[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double",
                      "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
                      "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
                      "union", "unsigned", "void", "volatile", "while"}; // length = 32

// 定义c语言运算符
static char *op[] = {"+", "-", "*", "/", "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=",
                     ">", "<", ">=", "<=", "&", "|", "!", "&&", "||", "^", "~", "<<", ">>", "?", ":",
                     ",", ";", "(", ")", "[", "]", "{", "}", "\"", "\'"}; // length = 40

// 非终结符，代号从77开始
static char *non_terminal[] = {"additive_expression", "assignment_expression", "compound_statement", "equality_expression", "expression", 
"expression_statement", "function_definition", "function_definition_list", "function_name", "iteration_statement", "jump_statement", 
"multiplicative_expression", "parameter_declaration", "parameter_list", "pointer", "primary_expression", "program", "relational_expression", 
"return_type", "selection_statement", "statement", "statement_list", "struct", "struct_body", "struct_body_item", "struct_name", "type_specifier", 
"unary_expression", "unary_operator", "variable_definition", "variable_definition_list"}; // length = 6

enum
{
  // 保留字
  _AUTO = 0,
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
  CHARCON,
  STRCON,
  // 标识符
  IDENFR,
};

char sourceCode[1000000];

// 定义符号表计数器
int tokenCount = 0;
int curLine = 1;
int curCol = 1;
// 记录错误集
char errorList[10000][100] = {""};
// 记录错误个数
int errorCount = 0;
// 记录变量名哈希值
int global_id_hash[10000] = {0};
// 保存token代号与值
typedef struct
{
  int code;
  char value[100];
} Token;
Token token_list[10000];

// 判断是否是保留字
int isKeyword(char s[])
{
  for (int i = 0; i < 32; i++)
    if (strcmp(s, key[i]) == 0)
      return i;
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
  if (c[0] != '+' && c[0] != '-')
    for (int i = 0; i < strlen(c); i++)
      if (!isDigit(c[i]))
        return 0;
      else
        for (int i = 1; i < strlen(c); i++)
          if (!isDigit(c[i]))
            return 0;

  return 1;
}

// 判断是否为十六进制数字
int isHexDigit(char *c)
{
  if (c[0] != '0' && c[0] != '-' && c[0] != '+')
    return 0;

  if (c[0] == '0' && c[1] != 'x' && c[1] != 'X')
    return 0;
  else
    for (int i = 2; i < strlen(c); i++)
      if (!isDigit(c[i]) && (c[i] < 'a' || c[i] > 'f') && (c[i] < 'A' || c[i] > 'F'))
        return 0;

  if (c[0] == '-' || c[0] == '+')
    if (c[1] != '0')
      return 0;
    else if (c[2] != 'x' && c[2] != 'X')
      return 0;
    else
      for (int i = 3; i < strlen(c); i++)
        if (!isDigit(c[i]) && (c[i] < 'a' || c[i] > 'f') && (c[i] < 'A' || c[i] > 'F'))
          return 0;

  return 1;
}

// 判断是否为八进制数字
int isOctDigit(char *c)
{
  if (c[0] != '0' && c[0] != '-' && c[0] != '+')
    return 0;

  if (c[0] == '0')
    for (int i = 1; i < strlen(c); i++)
      if (c[i] < '0' || c[i] > '7')
        return 0;

  if (c[0] == '-' || c[0] == '+')
    if (c[1] != '0')
      return 0;
    else
      for (int i = 2; i < strlen(c); i++)
        if (c[i] < '0' || c[i] > '7')
          return 0;
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
      return i + 32;
  return 0;
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

  // 删除注释
  if (start == '/' && sourceCode[*codePos + 1] == '/')
  {
    while (sourceCode[*codePos] != '\n')
      (*codePos)++;
    line++;
    *tokenType = -3;
    return;
  }
  else if (start == '/' && sourceCode[*codePos + 1] == '*')
  {
    while (!(sourceCode[*codePos] == '*' && sourceCode[*codePos + 1] == '/'))
    {
      (*codePos)++;
      int originCol = col;
      int sourceLine = line;
      if (sourceCode[*codePos] == '\n')
      {
        sourceLine++;
        token[tokenLen++] = sourceCode[*codePos];
      }
      // 判断是否有未闭合的注释
      if (*codePos == strlen(sourceCode) - 1)
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
        token[tokenLen++] = '\0';
        exit(0);
      }
    }
    (*codePos)++;
    *tokenType = -3;
    return;
  }
  else if (start == '#' && curCol == 1)
  {
    while (sourceCode[*codePos] != '\n')
      (*codePos)++;
    line++;
    *tokenType = -3;
    return;
  }
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
    else
    {
      errorCount++;
      *tokenType = 108;
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
      token[4] = '\0';
      *tokenType = CHARCON;
      col += 4;
      return;
    }
    token[tokenLen++] = sourceCode[(*codePos)++];
    token[tokenLen++] = sourceCode[(*codePos)++];
    token[tokenLen++] = sourceCode[(*codePos)++];
    token[3] = '\0';
    col += 3;
    if (token[2] != '\'')
    {
      errorCount++;
      *tokenType = 108;
      char exception[100] = "ERROR: line ";
      char temp[10];
      sprintf(temp, "%d", tmpLine);
      strcat(exception, temp);
      strcat(exception, " col ");
      sprintf(temp, "%d", tmpCol);
      strcat(exception, temp);
      strcat(exception, ": char should be enclosed by single quotes");
      strcpy(errorList[errorCount], exception);
      printf("%s\n", exception);
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
        *tokenType = 108;
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
    // 若该位是符号，且下一位是数字
    if ((start == '+' || start == '-') && isDigit(sourceCode[*codePos + 1]))
    {
      token[tokenLen++] = sourceCode[(*codePos)++];
      token[tokenLen++] = sourceCode[(*codePos)++];
      col++;
      while (isDigit(sourceCode[*codePos]) || isLetter(sourceCode[*codePos]))
      {
        token[tokenLen++] = sourceCode[(*codePos)++];
        col++;
      }
      token[tokenLen] = '\0';
      // 判断是否为16进制数
      if (isHexDigit(token))
      {
        *tokenType = INTCON;
        return;
      }
      else if (isOctDigit(token))
      {
        *tokenType = INTCON;
        return;
      }
      else if (isDecDigit(token))
      {
        *tokenType = INTCON;
        return;
      }
      else
      {
        errorCount++;
        *tokenType = 108;
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
    // 若该位是符号，且下一位也是符号
    else if (sourceCode[*codePos + 1] == '+' && sourceCode[*codePos + 1] == '-' && sourceCode[*codePos + 1] == '=' && sourceCode[*codePos + 1] == '>' && sourceCode[*codePos + 1] == '<' && sourceCode[*codePos + 1] == '&' && sourceCode[*codePos + 1] == '|' && sourceCode[*codePos + 1] == '\'' && sourceCode[*codePos + 1] == '\"')
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
      *tokenType = 108;
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
    *tokenType = 108;
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

/* 语法分析部分 */
char *lr1[400][120];
int n, m;
int totRule;
int ruleLen[100];
int nodeCnt = 0;

char *left[100];
char *right[100][100];
// 使用邻接表存储语法树
struct Node
{
  int id;
  int type;
  char *value;
  struct Node *child;
  struct Node *sibling;
};
struct Node *root;
struct Node *curNode;
struct Node *newNode(int type, char *value)
{
  struct Node *node = (struct Node *)malloc(sizeof(struct Node));
  node->id = nodeCnt++;
  node->type = type;
  node->value = value;
  node->child = NULL;
  node->sibling = NULL;
  return node;
}
void addNode(struct Node *node)
{
  if (curNode->child == NULL)
  {
    curNode->child = node;
  }
  else
  {
    struct Node *temp = curNode->child;
    while (temp->sibling != NULL)
    {
      temp = temp->sibling;
    }
    temp->sibling = node;
  }
}

void printTree(struct Node *node, int depth)
{
  for (int i = 0; i < depth; i++)
  {
    printf("  ");
  }
  printf("%d %s\n", node->id, node->value);
  if (node->child == NULL)
  {
    printf("%d %s\n", node->id, node->value);
  }
  else
  {
    printf("%d %s\n", node->id, node->value);
    struct Node *temp = node->child;
    while (temp != NULL)
    {
      printTree(temp, depth + 1);
      temp = temp->sibling;
    }
  }
}

void read_lr1()
{
  // 读入产生式
  FILE *production_fp = fopen("./Grammar.txt", "r");
  if (production_fp == NULL)
  {
    printf("ERROR: file not found\n");
    return;
  }
  char *full_production[100];
  // 分配内存
  for (int i = 0; i < 100; i++)
  {
    full_production[i] = (char *)malloc(sizeof(char) * 100);
    left[i] = (char *)malloc(sizeof(char) * 100);
    for (int j = 0; j < 100; j++)
    {
      right[i][j] = (char *)malloc(sizeof(char) * 100);
    }
  }
  // 按行读取产生式，并对其进行分割
  totRule = 0;
  while (fgets(full_production[totRule], 100, production_fp) != NULL)
  {
    // 去除换行符
    full_production[totRule][strlen(full_production[totRule]) - 1] = '\0';
    // 分割产生式
    char *p = strtok(full_production[totRule], " ->");
    left[totRule] = p;
    int cnt = 0;
    p = strtok(NULL, " -> ");
    right[totRule][cnt++] = p;
    while (p != NULL)
    {
      // 从上一个分割点的后一个字符开始分割，并去掉空格
      p = strtok(NULL, " ");
      if (p != NULL)
      {
        right[totRule][cnt] = p;
        cnt++;
      }
    }
    ruleLen[totRule] = cnt;
    totRule++;
  }
  FILE *lr1_fp = fopen("./LR(1).txt", "r");
  if (lr1_fp == NULL)
  {
    printf("ERROR: file not found\n");
    return;
  }
  // 首先读入n, m
  fscanf(lr1_fp, "%d %d", &n, &m);
  // 读入LR(1)分析表
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
    {
      lr1[i][j] = (char *)malloc(sizeof(char) * 3);
      fscanf(lr1_fp, "%s", lr1[i][j]);
    }
}

void parse(int *codePos, char *sourceCode, char *token, int *tokenType)
{
  read_lr1();
  // 使用分析表生成语法树，tokenize函数可获取下一个token

  // 初始化语法树
  root = newNode(0, "root");
  curNode = root;
  // 初始化栈
  int stack[100];
  int top = 0;
  stack[top++] = 0;
  // 初始化输入串
  int input[100];
  int inputLen = 0;
  // tokenList中已保存了所有token
  for (int i = 0; i < tokenCount; i++)
  {
    input[inputLen++] = token_list[i].code;
  }
  input[inputLen++] = 76; // $符号
  // 开始分析
  int curToken = 0; // 当前输入串的位置
  int curState = 0; // 当前状态
  while (1)
  {
    printf("%d %d %s\t", curState, input[curToken], token_list[curToken].value);
    printf("%s\n", lr1[curState][input[curToken]]);
    // 如果是移进操作
    if (lr1[curState][input[curToken]][0] == 's')
    {
      // 移进
      int nextState = atoi(lr1[curState][input[curToken]] + 1);
      stack[top++] = nextState;
      // 生成语法树节点
      struct Node *node = newNode(input[curToken], token_list[curToken].value);
      addNode(node);
      curNode = node;
      // 更新输入串
      curToken++;
      // 更新状态
      curState = nextState;
    }
    // 如果是规约操作
    else if (lr1[curState][input[curToken]][0] == 'r')
    {
      // 找到规约的产生式
      int rule = atoi(lr1[curState][input[curToken]] + 1);
      // 从栈中弹出相应的状态
      for (int i = 0; i < ruleLen[rule]; i++)
      {
        top--;
      }
      // 生成语法树节点
      struct Node *node = newNode(rule, left[rule]);
      addNode(node);
      curNode = node;
      // 查找goto表，更新状态
      curState = stack[top - 1];
      // 找到non_terminal的编号
      int non_terminal_id = 0;
      for (int i = 0; i < 100; i++)
      {
        if (strcmp(non_terminal[i], left[rule]) == 0)
        {
          non_terminal_id = i + 77;
          break;
        }
      }
      curState = atoi(lr1[curState][non_terminal_id]);
      printf("goto %d\n", curState);
      stack[top++] = curState;
    }
    // 如果是接受操作
    else if (lr1[curState][input[curToken]][0] == 'a')
    {
      // 接受
      printTree(root, 0);
      break;
    }
    // 如果是错误操作
    else
    {
      // 错误
      printf("ERROR: syntax error, col %d, row %d\n", curCol, curLine);
      printTree(root, 0);
      break;
    }
  }
}

int main()
{
  // 读取源代码
  FILE *fp = fopen("test.c", "r");
  if (fp == NULL)
  {
    printf("ERROR: file not found\n");
    return 0;
  }
  char ch;
  int codeLen = 0;
  while ((ch = fgetc(fp)) != EOF)
    sourceCode[codeLen++] = ch;
  char token[100];
  int tokenType;
  int codePos = 0;
  read_lr1();
  while (codePos < strlen(sourceCode))
  {
    tokenize(&codePos, sourceCode, token, &tokenType);
    if (tokenType)
      if (tokenType >= 1)
      {
        strcpy(token_list[tokenCount].value, token);
        token_list[tokenCount++].code = tokenType;
      }
    continue;
    // if (tokenType <= 32)
    //     printf("Reserved Word: %s\n", token);
    // else if (tokenType <= 64)
    //     printf("Operator: %s\n", token);
    // else if (tokenType <= 72)
    //     printf("Limit: %s\n", token);
    // else if (tokenType <= 75)
    //     printf("Literal: %s\n", token);
    // else if (tokenType == 76)
    //     printf("Identifier: %s\n", token);
    // else if (tokenType == 108)
    //     break;
    // 将token、tokenType保存至token_list
  }
  read_lr1();
  parse(&codePos, sourceCode, token, &tokenType);
}
