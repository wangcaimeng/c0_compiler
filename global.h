//
//  global.h
//  c0_compiler
//
//  Created by 王蔡勐 on 2016/12/6.
//  Copyright © 2016年 王蔡勐. All rights reserved.
//

#ifndef global_h
#define global_h
using namespace::std;
#include <string>
//常量定义
#define MAXSYMTABLENUM      1000
#define DATASIZE            4000
#define MAXLINELEN          500
#define RESERVEDWORDNUM     14
#define MAXOPERATENUMNUM    1000


//单词类型
#define CASESYM     0   //系统保留字数值等于在保留字表中的index
#define CHARSYM     1
#define CONSTSYM    2
#define ELSESYM     3
#define FLOATSYM    4
#define IFSYM       5
#define INTSYM      6
#define MAINSYM     7
#define PRINTFSYM   8
#define RETURNSYM   9
#define SCANFSYM    10
#define SWITCHSYM   11
#define VOIDSYM     12
#define WHILESYM    13

#define IDENT       14  //标识符
#define INTNUMBER   15  //数字
#define FLOATNUMBER 16
#define LEQSYM      17  // <=*
#define GEQSYM      18  //>=
#define NEQSYM      19  //!=
#define EQLSYM      12  //==
#define CHAR        21  //字符
#define STRING      22  //字符串 单个字符直接用ASCII码存贮  与0-22无冲突


//errorCode
#define FILENOTFIND         0
#define ILLEGALCHAR         1
#define DOUBLEQUOMISSING    2
#define QUOTATIONMISSING    3
#define REDECLARE           4
#define NOTDECLARE          5
#define NOTASYMBOL          6
#define SEMICOLONMISSING    7
#define LBRACEMISSING       8
#define RBRACEMISSING       9
#define LBRACKETMISSING     10
#define RBRACKETMISSING     11
#define ASSIGNSYMMISSING    12
#define WRONGTYPE           13
#define SWITCHLISTMISSING   14
#define RSQUAREBRAMISSING   15
#define ILLEGALEXPRESSION   16
#define ILLEGALTERM         17
#define ILLLEGALFACTOR      18
#define MAINFUNCMISSING     19
#define OUTOFSYMTABLE       20
#define WRONGARRAYLEN       21
#define WRONGSENTENCE       22
#define COLONMISSING        23
#define WRONGEXPRESSION     24
#define WRONGPARANUM        25
#define TOMUCHFUN           26
//handleCode
#define CONTINUE                0   //不跳读继续处理
#define EXIT                    1   //退出
#define TOTYPESYM               2   //跳到下一个类型标识符 int  char float void
#define TOCONSTOR2              3   //跳到const或类型标识符
#define TOSEMIOR3               4   //跳到const或类型标识符或分号
#define TOSEMIOR2               5   //跳到类型标识符或分号
#define TORBRACKET              6   //跳到右括号
#define TOSENTENCEHEADOR2       7   //跳到类型标识符或语句列开头
#define TOSENTENCEHEADORRBRACE  8   //跳到语句开头或右大括号
#define TOCASEORRBRACE          9   //跳到下一个case或右大括号
#define TOSENTENCEHEADORRBRACKET    10  //跳到下一个语句开头或右括号
#define TOSENTENCEHEADORADD     11  //跳到语句开头或加减运算符
#define TOSENTENCEHEADORMUL     12  //跳到语句开头或乘除运算符

//符号表定义
#define CONSTINT        0
#define CONSTCHAR       1
#define CONSTFLOAT      2
#define VARINT          3
#define VARCHAR         4
#define VARFLOAT        5
#define FUNCTION        6
#define PARA            7

//P-code生成及运行
#define MAXCODENUM      100000
#define MAXFUNNUM       1000
#define MAXCALLNUM    100000
#define MAXDATASIZE     100000
#define LOAD    0   //LOAD addr    地址内容加载到栈顶
#define LOADI   1   //LOADI num    将num加载到栈顶
#define STO     2   //STO addr  栈顶内容存到指定地址
#define STORIN  3   //STO 次栈顶
#define ADD     4   //栈顶=次栈顶+栈顶
#define SUB     5   //栈顶=次栈顶-栈顶
#define MULT    6   //栈顶=次栈顶*栈顶
#define DIV     7   //栈顶=次栈顶/栈顶
#define CONVER  8
#define BR      9   //BR lab无条件跳转
#define BRF     10  //BRF lab 若栈顶值为0，跳转到lab
#define LODTOP  11  //LOAD 栈顶值
#define CMP     12  //CMP opCode 按照opCode执行比较操作 结果为真栈顶置1，为假栈顶置0
#define ALLOC   13  //分配指定长度的空间
#define JSR     14  //跳转到指定位置并将返回地址压栈
#define WRT     15  //WRT type  以type类型输出栈顶元素
#define READ    16  //READ type
#define BRA     17  //BR 栈顶地址 并销毁当前运行记录


typedef struct {   //符号表项
    string name;  //名字
    int kind; //类型  0-const  1-var 2-fun  3-para
    float value;  //保存常量的值 以及函数的返回值类型
    int address; //保存变量地址
    int paraNum;  //函数参数个数
    int arrayLen; //数组长度 非0说明该符号表示数组
    
}symbol;

typedef struct {   //符号表
    symbol elements[MAXSYMTABLENUM];
    int index;    //当前符号表索引
    int fTotal;  //分程序总数
    int fIndexList[MAXSYMTABLENUM];   //分程序开始位置索引
}symTable;

//函数开头指令索引
typedef struct {
    string name;
    int index;
}funInstrIndex;

//PCODE指令
typedef struct {
    int instrName;
    float operateNum;
}pcode;


//函数定义

//词法分析
void getch();
int getsym();
bool isLetter(char ch);
bool isNum(char ch);
int searchInRW(string s);

//符号表
void addToTable(string name, int kind, float value, int address, int paraNum, int arrayLen);
int searchInTable(string name, string funName);
void addParaNumToTable(int paraNum);

//语法分析递归下降子程序
void program();  //程序处理
void constStatement(); //常量说明处理
void constDeclaration(); //常量定义处理
void varDeclaration(); //变量定义处理
void compoundSentence(); //复合语句处理
void sentenceList(); //语句列处理
void sentence(); //语句处理
void conditionSentence(); //条件语句处理
void loopSentence(); //循环语句处理
void condition(); //条件处理
void functionCallSentence(); //函数调用处理
void valueOfParameterList(string funName); //值参数表
void assignSentence(); //赋值语句处理
void readSentence(); //读语句处理
void writeSentence(); //写语句处理
void switchSentence(); //情况语句处理
void switchList(); //情况表
void caseSentence(); //情况子语句
void returnSentence(); //返回语句
void parameterList(); //参数表处理
void expression(); //表达式处理
void term(); //项处理
void factor(); //因子处理
bool isConst(string name, string funName);
float getConstValue(string name, string funName);
int getIdentType(string name,string funName);
//错误处理
void error(int errorCode, int handleCode); //错误处理

//pcode生成及解释执行
void genPcode(int instrName, float operateNum, int instrIndex);
void addFunInstrIndex(string funName, int index);
int getFunIndex(string funName);
void interpret();


#endif /* global_h */
