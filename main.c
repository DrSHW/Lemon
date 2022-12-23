#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define unsigned long long ull

/* 词法分析部分 */

char *key[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double",
               "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
               "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
               "union", "unsigned", "void", "volatile", "while"};
char *op[] = {"+", "-", "*", "/", "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=",
              ">", "<", ">=", "<=", "&", "|", "!", "&&", "||", "^", "~", "<<", ">>", "?", ":",
              ",", ";", "(", ")", "[", "]", "{", "}", "\"", "\'"};
enum tokenTypes
{
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
  INTCON,
  CHARCON,
  STRCON,
  IDENFR,
};

char sourceCode[1000000];

int curLine = 1;
int curCol = 1;

char token[100];
int tokenType;
int codePos = 0;

int isKeyword(char s[])
{
  for (int i = 0; i < 32; i++)
    if (strcmp(s, key[i]) == 0)
      return i;
  return 0;
}

int isLetter(char c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
    return 1;
  return 0;
}

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

void tokenize(int *codePos, char *sourceCode, char *token, int *tokenType)
{
  int tokenLen = 0;
  char start = sourceCode[*codePos];

  int col = curCol;
  int line = curLine;

  int tmpCol = col;
  int tmpLine = line;

  strcpy(token, "");

  if (start == '/' && sourceCode[*codePos + 1] == '/')
  {
    while (sourceCode[*codePos] != '\n')
      (*codePos)++;
    *tokenType = -3;
    return;
  }
  else if (start == '/' && sourceCode[*codePos + 1] == '*')
  {
    while (!(sourceCode[*codePos] == '*' && sourceCode[*codePos + 1] == '/'))
    {
      (*codePos)++;
      if (sourceCode[*codePos] == '\n')
      {
        tmpLine++;
        tmpCol = 0;
        token[tokenLen++] = sourceCode[*codePos];
      }
      tmpCol++;
      if (*codePos == strlen(sourceCode) - 1)
      {
        char exception[100] = "ERROR: line ";
        char temp[10];
        sprintf(temp, "%d", tmpLine);
        strcat(exception, temp);
        strcat(exception, " col ");
        sprintf(temp, "%d", tmpCol);
        strcat(exception, temp);
        strcat(exception, ": comments should be closed");
        token[tokenLen++] = '\0';
        printf("%s\n", exception);
        exit(0);
      }
    }
    (*codePos) += 2;
    *tokenType = -3;
    curCol = tmpCol;
    curLine = tmpLine;
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
    if (isHexDigit(token))
      *tokenType = INTCON;
    else if (isOctDigit(token))
      *tokenType = INTCON;
    else if (isDecDigit(token))
      *tokenType = INTCON;
    else
    {
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
      printf("%s\n", exception);
      exit(0);
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
      *tokenType = 108;
      char exception[100] = "ERROR: line ";
      char temp[10];
      sprintf(temp, "%d", tmpLine);
      strcat(exception, temp);
      strcat(exception, " col ");
      sprintf(temp, "%d", tmpCol);
      strcat(exception, temp);
      strcat(exception, ": char should be enclosed by single quotes");
      printf("%s\n", exception);
      exit(0);
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
      if (*codePos >= strlen(sourceCode))
      {
        *tokenType = 108;
        char exception[100] = "ERROR: line ";
        char temp[10];
        sprintf(temp, "%d", tmpLine);
        strcat(exception, temp);
        strcat(exception, " col ");
        sprintf(temp, "%d", tmpCol);
        strcat(exception, temp);
        strcat(exception, ": string should be closed with double quotes");
        printf("%s\n", exception);
        exit(0);
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
        printf("%s\n", exception);
        exit(0);
        return;
      }
    }
    else if (sourceCode[*codePos + 1] == '+' || sourceCode[*codePos + 1] == '-' || sourceCode[*codePos + 1] == '=' || sourceCode[*codePos + 1] == '>' || sourceCode[*codePos + 1] == '<' || sourceCode[*codePos + 1] == '&' || sourceCode[*codePos + 1] == '|')
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
      printf("%s\n", exception);
      exit(0);
      return;
    }
  }
  else if (start == EOF || start == '\0')
  {
    *tokenType = 76;
    token[0] = '$';
  }
  else
  {
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
    printf("%s\n", exception);
    exit(0);
    return;
  }
  curLine = line;
  curCol = col;
}

/* 语法分析部分 */

char *lr1[560][115];
int n, m;
int totRule;
int ruleLen[200];
int nodeCnt = 0;

char *non_terminal[] = {"additive_expression", "argument", "argument_list", "assignment_expression", "compound_statement", "equality_expression", "expression",
                        "expression_statement", "function_call", "function_definition", "function_definition_list", "function_name", "initializer", "initializer_list", "iteration_statement",
                        "jump_statement", "multiplicative_expression", "parameter_declaration", "parameter_list", "primary_expression", "program", "relational_expression", "return_type",
                        "selection_statement", "statement", "statement_list", "struct_definition", "struct_definition_list", "struct_name", "struct_variable_definition", "struct_variable_definition_list",
                        "unary_expression", "unary_operator", "variable_definition", "variable_definition_list"};

char *left[200];
char *right[200][120];
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
  printf("(%d %d %s) -> (%d %d %s)\n", curNode->id, curNode->type, curNode->value, node->id, node->type, node->value);

  FILE *fp = fopen("SyntaxTree.txt", "a");
  if (fp == NULL)
  {
    printf("ERROR: file not found\n");
    return;
  }

  char temp[100];
  sprintf(temp, "(%d %d %s) -> (%d %d %s)", curNode->id, curNode->type, curNode->value, node->id, node->type, node->value);
  strcat(temp, "\n");
  fputs(temp, fp);
  fclose(fp);
}

void load_data()
{
  FILE *production_fp = fopen("./Grammar.txt", "r");
  if (production_fp == NULL)
  {
    printf("ERROR: grammar file not found\n");
    exit(0);
  }
  char *full_production[200];

  for (int i = 0; i < 200; i++)
  {
    full_production[i] = (char *)malloc(sizeof(char) * 100);
    left[i] = (char *)malloc(sizeof(char) * 100);
    for (int j = 0; j < 120; j++)
    {
      right[i][j] = (char *)malloc(sizeof(char) * 100);
    }
  }

  totRule = 0;
  while (fgets(full_production[totRule], 200, production_fp) != NULL)
  {
    full_production[totRule][strlen(full_production[totRule]) - 1] = '\0';
    char *p = strtok(full_production[totRule], " ->");
    left[totRule] = p;
    int cnt = 0;
    p = strtok(NULL, " -> ");
    right[totRule][cnt++] = p;
    while (p != NULL)
    {
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
    printf("ERROR: lr(1) form file not found\n");
    exit(0);
  }
  fscanf(lr1_fp, "%d %d", &n, &m);
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
    {
      lr1[i][j] = (char *)malloc(sizeof(char) * 3);
      fscanf(lr1_fp, "%s", lr1[i][j]);
    }
}

void parse(int *codePos, char *sourceCode, char *token, int *tokenType)
{
  load_data();
  root = newNode(0, "root");
  curNode = root;
  int stack[300];
  int top = 0;
  stack[top++] = 0;
  int curState = 0;
  do
  {
    tokenize(codePos, sourceCode, token, tokenType);
  } while (*tokenType <= 0);
  while (1)
  {
    if (lr1[curState][*tokenType][0] == 's')
    {
      int nextState = atoi(lr1[curState][*tokenType] + 1);
      stack[top++] = nextState;
      struct Node *node = newNode(*tokenType, token);
      addNode(node);
      curNode = node;
      do
      {
        tokenize(codePos, sourceCode, token, tokenType);
      } while (*tokenType <= 0);
      curState = nextState;
    }
    else if (lr1[curState][*tokenType][0] == 'r')
    {
      int rule = atoi(lr1[curState][*tokenType] + 1);
      for (int i = 0; i < ruleLen[rule]; i++)
      {
        top--;
      }
      struct Node *node = newNode(rule, left[rule]);
      addNode(node);
      curNode = node;
      curState = stack[top - 1];
      int non_terminal_id = 0;
      for (int i = 0; i < 35; i++)
      {
        if (strcmp(non_terminal[i], left[rule]) == 0)
        {
          non_terminal_id = i + 77;
          break;
        }
      }
      curState = atoi(lr1[curState][non_terminal_id]);
      stack[top++] = curState;
    }
    else if (lr1[curState][*tokenType][0] == 'a')
    {
      break;
    }
    else if (lr1[curState][*tokenType][0] == 'e')
    {
      printf("ERROR: syntax error, unexpected token: \"%s\", at col %d, row %d\n", token, curCol, curLine);
      break;
    }
    else
    {
      printf("ERROR: wrong lr1 form content\n", token, curCol, curLine);
      break;
    }
  }
}

int main()
{
  FILE *fp = fopen("test.c", "r");
  if (fp == NULL)
  {
    printf("ERROR: source code file not found\n");
    return 0;
  }
  char ch;
  int codeLen = 0;
  while ((ch = fgetc(fp)) != EOF)
    sourceCode[codeLen++] = ch;
  parse(&codePos, sourceCode, token, &tokenType);
  return 0;
}
