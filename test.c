/* test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void get_next(int next[], char a[], int la) // 求NEXT[]的值
{
  int i = 1, j = 0;
  next[1] = 0;
  while (i <= la || j == 0) // 核心部分
  {
    if (a[i] == a[j])
    {
      ++j;
      ++i;
      if (a[i] == a[j])
        next[i] = next[j];
      else
        next[i] = j;
    }
    else
      j = next[j];
  }
}
int str_kmp(int next[], char A[], char a[], int lA, int la) // KMP主程序
{
  int i;
  int j;
  int k;
  i = 1;
  j = 1;
  while (i <= lA && j <= la)
  {
    if (A[i] == a[j] || j == 0)
    {
      ++i;
      ++j;
    }
    else
      j = next[j];
  }
  if (j > la)
    return i - j + 1;
  else
    return -1;
}
int main()
{
  int n;
  int k;
  int next[1000] = {0};
  int lA = 0;
  int la = 0;
  char A[1000];
  char a[1000];
  scanf("%s %s", A, a);
  lA = strlen(A);
  la = strlen(a);
  for (k = la - 1; k >= 0; --k)
    a[k + 1] = a[k];
  for (k = lA - 1; k >= 0; --k)
    A[k + 1] = A[k];
  get_next(next, a, la);
  k = str_kmp(next, A, a, lA, la);
  if (k == -1)
    printf("Not Soulation!!! ");
  if (k != -1)
    printf("%d ", k);
  system("pause");
  return 0;
}
