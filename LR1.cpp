#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <stack>
#include <iomanip>
#include <ctime>
#include <set>
#include <list>
#include <algorithm>

using namespace std;

typedef pair<string, vector<string>> Production; // 产生式结构

// 定义项目集结构体
struct ItemSet
{
  string left;
  vector<string> right;
  set<string> expect;

  ItemSet(string l, vector<string> r, set<string> e)
  {
    left = l;
    right = r;
    expect = e;
  }

  const bool operator<(const ItemSet &item) const
  {
    if (left != item.left)
      return left < item.left;
    if (right != item.right)
      return right < item.right;
    return expect < item.expect;
  }
};

// 产生项目集族
struct project
{
  set<string> termial;
  set<string> nontermial;
  set<string> all_symbol;
  vector<string> table_head;
  // 加·前的产生式集合
  vector<Production> productions;
  // 加·后的项目集族
  set<Production> project_set;
  vector<set<ItemSet>> project_set_final = vector<set<ItemSet>>(1);
  string lr1_table[250][250];

  map<string, int> hash_map = {
      {"_AUTO", 1},
      {"_BREAK", 2},
      {"_CASE", 3},
      {"_CHAR", 4},
      {"_CONST", 5},
      {"_CONTINUE", 6},
      {"_DEFAULT", 7},
      {"_DO", 8},
      {"_DOUBLE", 9},
      {"_ELSE", 10},
      {"_ENUM", 11},
      {"_EXTERN", 12},
      {"_FLOAT", 13},
      {"_FOR", 14},
      {"_GOTO", 15},
      {"_IF", 16},
      {"_INT", 17},
      {"_LONG", 18},
      {"_REGISTER", 19},
      {"_RETURN", 20},
      {"_SHORT", 21},
      {"_SIGNED", 22},
      {"_SIZEOF", 23},
      {"_STATIC", 24},
      {"_STRUCT", 25},
      {"_SWITCH", 26},
      {"_TYPEDEF", 27},
      {"_UNION", 28},
      {"_UNSIGNED", 29},
      {"_VOID", 30},
      {"_VOLATILE", 31},
      {"_WHILE", 32},
      {"PLUS", 33},
      {"MINUS", 34},
      {"STAR", 35},
      {"DIV", 36},
      {"MOD", 37},
      {"PLUSPLUS", 38},
      {"MINUSMINUS", 39},
      {"ASSIGN", 40},
      {"PLUSEQUAL", 41},
      {"MINUSEQUAL", 42},
      {"STAREQUAL", 43},
      {"DIVEQUAL", 44},
      {"MODEQUAL", 45},
      {"EQUAL", 46},
      {"NOTEQUAL", 47},
      {"GREAT", 48},
      {"LESS", 49},
      {"GREATEQUAL", 50},
      {"LESSEQUAL", 51},
      {"AND", 55},
      {"OR", 56},
      {"NOT", 54},
      {"ANDAND", 52},
      {"OROR", 53},
      {"BITXOR", 57},
      {"BITNOT", 58},
      {"LEFTMOVE", 59},
      {"RIGHTMOVE", 60},
      {"QUESTION", 61},
      {"COLON", 62},
      {"COMMA", 63},
      {"SEMICOLON", 64},
      {"LPARENT", 65},
      {"RPARENT", 66},
      {"LBRACKET", 67},
      {"RBRACKET", 68},
      {"LBRACE", 69},
      {"RBRACE", 70},
      {"DOUBLEQUOTE", 71},
      {"SINGLEQUOTE", 72},
      {"INTCON", 73},
      {"CHARCON", 74},
      {"STRCON", 75},
      {"IDENFR", 73},
  };
  // 计算上面的所有参数
  void get_project()
  {

    ifstream fin("./productions.txt");
    string left, arrow;   // 产生式的左部和箭头
    vector<string> right; // 产生式的右部，各个候选式

    while (fin >> left >> arrow)
    { // 读取产生式的左部(a)与箭头(b)，箭头不用

      nontermial.insert(left); // 将左部插入非终结符集合
      string str;
      getline(fin, str);
      stringstream ss;
      ss.str(str);
      while (ss >> str)
      {
        right.push_back(str);
        int is_nonterminal = true; // 产生式中，非终结符都是由小写字母和下划线组成
        for (int i = 0; i < str.size(); i++)
        {
          if (!(str[i] >= 'a' && str[i] <= 'z' || str[i] == '_'))
          {
            is_nonterminal = false;
            break;
          }
        }
        if (is_nonterminal)
          nontermial.insert(str);
        else
          termial.insert(str);
      }
      productions.push_back(Production(left, right));
    }
    termial.insert("$");
    all_symbol.insert(termial.begin(), termial.end());
    all_symbol.insert(nontermial.begin(), nontermial.end());
    for (auto it = all_symbol.begin(); it != all_symbol.end(); it++)
      table_head.push_back(*it);
    for (int i = 0; i < nontermial.size(); i++)
      hash_map.insert(pair<string, int>(*next(nontermial.begin(), i), i + 100));
    for (auto it : productions)
    {
      vector<string> right = it.second;
      for (int i = 0; i <= right.size(); i++)
      {
        vector<string> temp;
        for (int j = 0; j < i; j++)
          temp.push_back(right[j]);
        temp.push_back("·");
        for (int j = i; j < right.size(); j++)
          temp.push_back(right[j]);
        project_set.insert(Production(it.first, temp));
      }
    }

    fin.close();
  }

  // 计算FIRST集
  set<string> get_first_set(vector<string> x)
  {
    set<string> first_set;
    if (termial.find(x[0]) != termial.end())
    {
      first_set.insert(x[0]);
      return first_set;
    }
    else
    {
      for (auto it : productions)
      {
        if (it.first == x[0])
        {
          if (termial.find(it.second[0]) != termial.end())
          {
            first_set.insert(it.second[0]);
          }
          else
          {
            set<string> temp = get_first_set(it.second);
            first_set.insert(temp.begin(), temp.end());
          }
        }
      }
    }
    return first_set;
  }

  // GO函数
  set<ItemSet> _go(set<ItemSet> I, string X)
  {
    set<ItemSet> J;
    for (auto it : I)
    {
      vector<string> right = it.right;
      int pos = find(right.begin(), right.end(), "·") - right.begin();
      if (pos < right.size() - 1 && right[pos + 1] == X)
      { // 如果·在候选式的最后一个位置，那么就不用移动
        set<string> temp;
        for (int i = 0; i < right.size(); i++)
        { // 移动·，即交换·与·后面的符号
          if (i == pos)
            temp.insert(right[i + 1]);
          else if (i == pos + 1)
            temp.insert(right[i - 1]);
          else
            temp.insert(right[i]);
        }
        J.insert(ItemSet(it.left, it.right, temp)); // 将新的产生式插入J
      }
    }
    return J;
  }

  // 计算CLOSURE闭包
  set<ItemSet> get_closure_set(set<ItemSet> I)
  {
    while (true)
    {
      // cout << "123" << endl;
      bool isUpdated = false;
      for (auto it : I)
      {
        vector<string> right = it.right;
        int pos = find(right.begin(), right.end(), "·") - right.begin();
        if (pos < right.size() - 1)
        {
          string X = right[pos + 1];
          if (termial.find(X) != termial.end())
            continue;
          right.erase(right.begin(), right.begin() + pos + 2);
          string last;
          for (auto it1 : it.expect)
          {
            right.push_back(it1);
            if (last == right[0])
              continue;
            else
              last = right[0];
              cout << "123" << endl;
            set<string> first_set = get_first_set(right);
            // 遍历first_set
            for (auto it2 : first_set)
            {
              cout << it2 << ' ';
            }
            cout << endl;
            right.pop_back();
            for (auto it2 : project_set)
            {
              if (it2.first == X && it2.second[0] == "·")
              {
                set<string> _empty;
                ItemSet temp(it2.first, it2.second, _empty);
                auto pos = I.lower_bound(temp);
                if (pos != I.end() && (*pos).left == it2.first && (*pos).right == it2.second)
                {
                  temp = *pos;
                  I.erase(temp);
                  temp.expect.insert(first_set.begin(), first_set.end());
                  I.insert(temp);
                }
                else
                {
                  // cout << "123" << endl;
                  isUpdated = true;
                  I.insert(ItemSet(it2.first, it2.second, first_set));
                  // cout << "123" << endl;
                }
              }
            }
            // cout << "123" << endl;
          }
        }
        // cout << "123" << endl;
      }
    }
    return I;
  }

  // 计算lr1分析表
  void get_lr1_table()
  {
    for (auto it : project_set)
    {
      if (it.first == "program'" && it.second[0] == "·")
      {
        set<string> temp;
        temp.insert("$");
        project_set_final[0].insert(ItemSet{it.first, it.second, temp});
        break;
      }
    }
    project_set_final[0] = get_closure_set(project_set_final[0]);
    for (int i = 0; i < project_set_final.size(); i++)
    {
      for (auto it : project_set_final[i])
      {
        if (it.right[it.right.size() - 1] == "·")
        {
          it.right.erase(it.right.end() - 1);
          Production temp(it.left, it.right);
          for (int j = 0; j < productions.size(); j++)
          {
            if (temp == productions[j])
            {
  
              string t = "r" + to_string(j);

              if (j == 0)
                t = "acc";
              for (auto its : it.expect)
                lr1_table[i][hash_map[its]] = t;
            }
          }
        }
      }
    }
  }
};

int main()
{

  project main;

  main.get_project();
  // for (auto it : main.project_set)
  // {
  //   cout << it.first << " -> ";
  //   for (auto i : it.second)
  //     cout << i << " ";
  //   cout << endl;
  // }
  main.get_lr1_table();
  // 遍历lr1分析表
  for (int i = 0; i < main.all_symbol.size(); i++)
  {
    for (int j = 0; j < main.termial.size(); j++)
    {
      if (main.lr1_table[i][j] != "")
        cout << i << " " << main.lr1_table[i][j] << endl;
    }
    cout << endl;
  }
  cout << "gggg";
  return 0;
}
