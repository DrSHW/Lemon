#include <set>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

typedef pair<string, vector<string>> Production; // 产生式的数据结构，左部为string，右部为vector<string>

// 词法分析的token，用于预测符的判断
string all_terminals[] = {
    "_AUTO", "_BREAK", "_CASE", "_CHAR", "_CONST", "_CONTINUE", "_DEFAULT", "_DO", "_DOUBLE",
    "_ELSE", "_ENUM", "_EXTERN", "_FLOAT", "_FOR", "_GOTO", "_IF", "_INT", "_LONG", "_REGISTER",
    "_RETURN", "_SHORT", "_SIGNED", "_SIZEOF", "_STATIC", "_STRUCT", "_SWITCH", "_TYPEDEF",
    "_UNION", "_UNSIGNED", "_VOID", "_VOLATILE", "_WHILE", "+", "-", "*", "/", "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "==", "!=",
    ">", "<", ">=", "<=", "&", "|", "!", "&&", "||", "^", "~", "<<", ">>", "?", ":",
    ",", ";", "(", ")", "[", "]", "{", "}", "\"", "\'", "INTCON", "CHARCON", "STRCON", "IDENFR"}; // length = 7

struct Project
{                       // 项目集
  string left;          // 左部
  vector<string> right; // 右部
  set<string> expect;   // 展望串

  // 重载运算符
  const bool operator<(const Project &p) const
  {
    if (left < p.left)
      return true;
    if (left > p.left)
      return false;
    if (right < p.right)
      return true;
    if (right > p.right)
      return false;
    if (expect < p.expect)
      return true;
    return false;
  }

  const bool operator==(const Project &p) const
  {
    if (left == p.left && right == p.right && expect == p.expect)
      return true;
    return false;
  }
};

struct ItemSet
{
  // 用于产生项目集的产生式
  vector<string> terminal;        // 终结符，对应上方的all_terminals
  set<string> non_terminal;       // 非终结符，使用set去重存储
  vector<string> all_symbols;     // 所有的符号，包括终结符和非终结符
  map<string, int> symbol_hash;   // 符号哈希，靠前的符号为终结符，对应词法分析的token，靠后的符号为非终结符
  vector<Production> productions; // 所有的产生式
  set<Production> items;          // 所有的项目集，即所有加·后的产生式

  void gen_poj()
  {
    ifstream fin("./Grammar.txt"); // 读取文法
    // 若文件打开失败，输出错误信息并退出
    if (!fin)
    {
      cout << "ERROR: grammar file not found" << endl;
      exit(0);
    }
    string a, b;                   // 产生式左部(a)和箭头(b)，不处理b
    vector<string> c;              // 产生式右部，使用vector存储
    while (fin >> a >> b)
    {
      non_terminal.insert(a); // 产生式左部一定是非终结符，加入到非终结符集合中
      string str;
      getline(fin, str); // 读取产生式右部（该行剩余部分）
      stringstream ss;   // 用于分割产生式右部
      ss.str(str);       // 将产生式右部赋值给ss
      c.clear();         // 清空c，用于下一次存储产生式右部
      while (ss >> str)  // 分割产生式右部
      {
        c.push_back(str); // 将每个产生式右部的符号存入c
        bool flag = true; // 判断是否是终结符(非终结符由小写字母和下划线组成)
        for (int i = 0; i < str.size(); i++)
          if (!(str[i] >= 'a' && str[i] <= 'z' || str[i] == '_')) // 如果不是小写字母或下划线，说明是终结符
          {
            flag = false;
            break;
          }
        if (flag)
          non_terminal.insert(str); // 如果是终结符，加入到非终结符集合中
      }
      productions.push_back(Production(a, c)); // 将产生式加入到产生式集合中
    }
    // 将所有的终结符加入到终结符集合中
    for (int i = 0; i < 76; i++)
      terminal.push_back(all_terminals[i]);
    terminal.push_back("$");        // 加入结束符$
    non_terminal.erase("program'"); // 删掉program'
    for (auto it : terminal)        // 将所有的终结符加入到all_symbols中
      all_symbols.push_back(it);
    for (auto it : non_terminal) // 将所有的非终结符加入到all_symbols中
      all_symbols.push_back(it);
    for (int i = 0; i < all_symbols.size(); i++) // 将所有的符号转化为对应的哈希值，从0开始
      symbol_hash[all_symbols[i]] = i;
    // 打印表头
    for (int i = 0; i < all_symbols.size(); i++)
      printf("\"%s\", ", all_symbols[i].c_str());
    printf("\n");
    for (auto it : productions) // 遍历所有的产生式，生成所有的项目集
    {
      a = it.first, c = it.second;        // 取出产生式左部和右部
      for (int i = 0; i <= c.size(); i++) // 产生式右部的每个位置都可以加·
      {
        vector<string> d = c;
        d.insert(d.begin() + i, ".");   // 在第i个位置加·
        items.insert(Production(a, d)); // 将加·后的产生式加入到项目集中
      }
    }
  }
};

struct LR1
{
  // 生成lr表
  ItemSet is;                                             // 项目集信息
  vector<set<Project>> can_col = vector<set<Project>>(1); // 产生式项目集规范族，项目集规范族的英文为canonical_collection
  string lr1[560][115];                                   // lr1分析表

  set<string> _first(vector<string> X)
  {
    // 求取非终结符X的FIRST集
    set<string> res;                                                             // 用于保存返回结果
    if (find(is.terminal.begin(), is.terminal.end(), X[0]) != is.terminal.end()) // 如果是终结符，直接插入FIRST集并返回
    {
      res.insert(X[0]);
      return res;
    }
    else
    {
      for (int j = 0; j < is.productions.size(); j++) // 遍历所有的产生式
      {
        if (is.productions[j].first == X[0]) // 找到产生式左部为X的产生式
        {
          if (find(is.terminal.begin(), is.terminal.end(), is.productions[j].second[0]) != is.terminal.end()) // 如果第一个是终结符
            res.insert(is.productions[j].second[0]);                                                          // 将其插入到res中
          else
          {
            set<string> t = _first(is.productions[j].second); // 否则递归求FIRST集
            res.insert(t.begin(), t.end());                   // 将结果插入到res中
          }
        }
      }
    }
    return res; // 返回结果
  }

  set<Project> _go(set<Project> I, string X)
  {
    // GO函数
    set<Project> res; // 用于保存返回结果
    for (auto it : I) // 遍历项目集I中的每一个项目
    {
      vector<string> vs = it.right;               // 取出项目的右部
      auto pos = find(vs.begin(), vs.end(), "."); // 找到·的位置
      if (pos == vs.end() - 1)                    // 如果·是最后一个，进入下一个项目
        continue;
      if (*(pos + 1) == X) // 如果·后面的符号是X
      {
        swap(*pos, *(pos + 1));                      // 交换·和后面的一个字符串
        res.insert(Project{it.left, vs, it.expect}); // 将交换后的产生式加入到res中
      }
    }
    return res; // 返回结果
  }

  set<Project> _closure(set<Project> I)
  {
    // 闭包函数
    while (true)
    {
      bool update = false; // 用于判断是否更新
      for (auto it : I)    // 遍历项目集I中的每一个项目
      {
        vector<string> X = it.right;              // 取出项目的右部
        auto pos = find(X.begin(), X.end(), "."); // 找到·的位置
        if (pos == X.end() - 1)                   // 如果·是最后一个，即A->α·，进入下一个项目
          continue;
        string c = *(pos + 1); // 取出·后面的一个字符
        if (find(is.terminal.begin(), is.terminal.end(), c) != is.terminal.end())
          continue;                  // 如果c是终结符，即A->α·a，进入下一个项目
        X.erase(X.begin(), pos + 2); // 否则A->α·Bβ，删除·后面的字符和·
        string last;                 // 记录上一次保存的FIRST集的第一个字符串
        for (auto ite : it.expect)   // 遍历项目的expect集，即FIRST(βa)
        {
          X.push_back(ite); // 将expect集中的每一个字符串加入到X的末尾
          if (last == X[0]) // 如果上一次保存的FIRST集的第一个字符串和这次相同，进入下一个字符串
            continue;
          else
            last = X[0];                                // 否则，更新last
          set<string> First = _first(X);                // 求取X的FIRST集
          X.pop_back();                                 // 删除X的末尾字符串，因为X是一个临时变量，不会改变项目的right
          for (auto it1 : is.items)                     // 遍历所有的产生式
            if (it1.first == c && it1.second[0] == ".") // 如果产生式的左部是c，右部以·开头，即B->·γ
            {
              set<string> temp; // 用于保存expect集
              Project p{it1.first, it1.second, temp};
              auto pos = I.lower_bound(p);                                                  // 二分查找第一个大于等于p的位置
              if (pos != I.end() && (*pos).left == it1.first && (*pos).right == it1.second) // 如果找到了，则更新expect集
              {
                p = *pos;
                I.erase(p);                                  // 删除原来的项目，以便进行更新
                p.expect.insert(First.begin(), First.end()); // 更新expect集
                I.insert(p);                                 // 插入更新后的项目
              }
              else
              {
                update = true;                                   // 如果没找到，则说明是新的项目，需要插入，即更新了项目集
                I.insert(Project{it1.first, it1.second, First}); // 插入新的项目
              }
            }
        }
      }
      if (!update) // 如果没有更新，则退出循环
        break;
    }
    return I;
  }

  void gen_lr1()
  {
    for (auto it : is.items) // 遍历所有项目
    {
      if (it.first == "program'" && it.second[0] == ".") // 找到项目program'->·S
      {
        set<string> temp;                                      // 用于保存expect集
        temp.insert("$");                                      // expect集中加入$
        can_col[0].insert(Project{it.first, it.second, temp}); // 将项目插入到第一个项目集中
        break;
      }
    }
    can_col[0] = _closure(can_col[0]);       // 求取第一个项目集的闭包
    for (int i = 0; i < can_col.size(); i++) // 遍历所有的项目集
    {
      cout << "已生成第" << i + 1 << "个项目集！" << endl; // 输出提示信息
      for (auto it : can_col[i])                           // 遍历项目集族，进行规约
      {
        int len = it.right.size();    // 项目的右部长度
        if (it.right[len - 1] == ".") // 若最后一个字符串是·，即可进行规约
        {
          it.right.erase(it.right.end() - 1);             // 将·删除
          Production p(it.left, it.right);                // 构造产生式
          for (int j = 0; j < is.productions.size(); j++) // 遍历所有的产生式
          {
            if (is.productions[j] == p) // 如果产生式相同，即找到了目标产生式
            {
              string t;
              if (j == 0) // 如果是第一个产生式，即program'->·program，则为接受项目，动作为acc
                t = "acc";
              else
                t = "r" + to_string(j); // 否则为归约项目，构造归约的动作为rj

              for (auto its : it.expect)
                lr1[i][is.symbol_hash[its]] = t; // 将动作插入到lr1表中
            }
          }
        }
      }

      for (auto X : is.all_symbols) // 遍历所有的符号
      {
        set<Project> J = _go(can_col[i], X); // 求取项目集族的移进项目集
        if (!J.empty())                      // 若J非空
        {
          J = _closure(J);                                    // 求取J的闭包
          int k;                                              // 记录J的位置
          auto pos = find(can_col.begin(), can_col.end(), J); // 在已经生成的项目集族中查找J
          if (pos != can_col.end())                           // 若在项目集族中找到了J
            k = pos - can_col.begin();                        // 记录J的位置
          else
          {
            k = can_col.size();   // 否则J是新的项目集，记录J的位置
            can_col.push_back(J); // 将J插入到项目集族中
          }
          int j = is.symbol_hash[X];                                                // 记录X的位置
          if (find(is.terminal.begin(), is.terminal.end(), X) != is.terminal.end()) // 如果X是终结符
            lr1[i][j] = "s" + to_string(k);                                         // 则为移进项目，动作为sj
          else
            lr1[i][j] = to_string(k); // 否则为待约项目，出现在goto表中，对应状态j
        }
      }
    }
  }

  void save_lr1()
  {
    // 输出lr1表到文件LR(1).txt中，便于主分析程序读取
    ofstream fout("./LR(1).txt");
    int n = can_col.size();        // 项目集的个数
    int m = is.all_symbols.size(); // 符号的个数
    fout << n << " " << m << endl; // 输出项目集的个数和符号的个数
    // 输出lr1数组的内容
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < m; j++)
      {
        if (lr1[i][j] == "")
          fout << "err "; // 如果lr1[i][j]为空，则输出err，报错
        else
          fout << lr1[i][j] << ' ';
      }
      fout << endl; // 换行
    }
  }

  void main()
  {
    // 主函数
    is.gen_poj();
    gen_lr1();
    save_lr1();
  }
};

int main()
{
  LR1 lr;
  lr.main();
  return 0;
}
