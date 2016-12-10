//
//  main.cpp
//  c0_compiler
//
//  Created by 王蔡勐 on 2016/12/6.
//  Copyright © 2016年 王蔡勐. All rights reserved.
//


#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"
#include <stdio.h>
using namespace::std;


//全局变量


ifstream fin;  //目标文件

//PCODE生成及解释执行
int instrIndex;
pcode pcodes[MAXCODENUM];  //指令数组
int funNum;
funInstrIndex fInstrIndexList[MAXFUNNUM];  //函数指令索引数组
float dataArea[MAXDATASIZE]; //数据区
int dataIndex;
int baseAddr[MAXCALLNUM]; //存每个活动记录基地址
string thisFunName;
const int globalMark = 100000;
int switchExp[MAXCALLNUM]; //存switch的expression的值
int switchIndex=1;
float operateNumStack[MAXOPERATENUMNUM]; //操作数栈
int top = -1;

//词法分析相关
string word[RESERVEDWORDNUM]; //系统保留字表
string a;       //当前单词
int symType;   //读取的单词类型
string symValue;    //读取单词的值
float numberValue;   //读取数的值
char line[MAXLINELEN], lastLine[MAXLINELEN]; //当前行和上一行
int lineNum = 0, charNum = 0, lineLen = 0, lastLineNum = 0, lastCharNum = 0; //行号，字符号，行长度；
char ch; //读取的字符


//语法分析

//符号表项信息；
int expType; //表达式类型
int charCount,intCount,floatCount;
bool isArrIndex = false;
string name;  //名字
int kind; //类型
float value;  //保存常量的值
int address; //保存变量地址
int paraNum;  //函数参数个数
int arrayLen; //数组长度
//符号表
symTable mainTable;




//判断字符是不是字母
bool isLetter(char ch) {
    if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z')) {
        return true;
    }
    return false;
}


//判断字符是不是数字
bool isNum(char ch) {
    if (ch >= '0'&&ch <= '9') {
        return true;
    }
    return false;
}

//在保留字中查找保留字
int searchInRW(string s) {
    for (int i = 0; i<RESERVEDWORDNUM; i++) {
        if (s == word[i]) {
            return i;
        }
    }
    return -1;
}


//读取一个字符 采用行缓冲区
void getch() {
    
    if (charNum>lineLen) {  //该行读完
        strcpy(lastLine, line);
        lineNum++;
        fin.getline(line, MAXLINELEN);
        lineLen = string(line).length() - 1;
        charNum = 0;
        ch = isLetter(line[charNum]) ? tolower(line[charNum]) : line[charNum];
        charNum++;
    }
    else {
        ch = isLetter(line[charNum]) ? tolower(line[charNum]) : line[charNum];
        charNum++;
    }
}
//词法分析，每次读取一个单词
//匹配成功返回0，失败返回-1，文件结束返回1；
int getsym() {
    //初始化变量
    lastCharNum = charNum;
    lastLineNum = lineNum;
    a.assign("");
    if (fin.eof()) {   //判断文件是否结束
        return 1;
    }
    getch();
    while (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\0') { //跳过空格
        if (fin.eof()) {   //判断文件是否结束
            return 1;
        }
        getch();
       
    }
    if (isLetter(ch)||ch=='_') {  //标识符
        do {
            a = a + ch;
            getch();
        } while (isLetter(ch) || isNum(ch)||ch=='_');
        int wordIndex = searchInRW(a);
        if (wordIndex != -1) {
            symValue = word[wordIndex];
            symType = wordIndex;
        }
        else {
            symValue = a;
            symType = IDENT;
        }
        charNum--;
    }
    else if (isNum(ch)) {         //数字
        float num = 0;
        bool point = false; //小数点是否出现过
        int numAfterPoiont = 0; //小数点后数字数
        do {
            if (ch == '.'&&!point) {   //第一次遇到小数点
                point = true;
            }
            else {
                if (point) {
                    numAfterPoiont++;
                    if (ch != '0') {
                        num = num + (ch - '0') / pow(10.0, numAfterPoiont);  //遇到小数点后
                    }
                    
                }
                else {
                    num = num * 10 + (ch - '0');  //没遇到小数点
                }
            }
            getch();
        } while (isNum(ch) || (ch == '.'&&!point)); //遇到非数组或者非第一个小数点退出循环
        numberValue = num;
        if (point) {
            symType = FLOATNUMBER;
        }
        else {
            symType = INTNUMBER;
        }
        
        charNum--;
    }
    else if (ch == '\'') {   //字符
        a = a + ch;
        
        getch();
        if (isLetter(ch) ||ch=='_'|| isNum(ch) || ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            a = a + ch;
            numberValue = ch;
        }
        else {
            charNum--;
            cout << "错误" << ch << "不是字符" << endl;
            return -1;
            
        }
        getch();
        if (ch == '\'') {
            a = a + ch;
            
        }
        else {
            cout << "错误 未匹配到'" << endl;
            return -1;
            
        }
        symValue = a;
        symType = CHAR;
    }
    else if (ch == '\"') {   //字符串
        do {
            a = a + ch;
            getch();
        } while (ch >= 32 && ch <= 126 && ch != 34);
        if (ch == '\"') {
            a = a + ch;
            
            symValue = a;
            symType = STRING;
        }
        else {
            cout << "错误 未匹配到" << '\"' << endl;
            return -1;
            
        }
        
    }
    else if (ch == '<') {
        getch();
        if (ch == '=') {
            symType = LEQSYM;
        }
        else {
            charNum--;
            symType = '<';
        }
    }
    else if (ch == '>') {
        getch();
        if (ch == '=') {
            symType = GEQSYM;
        }
        else {
            charNum--;
            symType = '>';
        }
    }
    else if (ch == '!') {
        getch();
        if (ch == '=') {
            symType = NEQSYM;
        }
        else {
            charNum--;
            cout << "错误 未匹配到=" << endl;
            return -1;
            
        }
    }
    else if (ch == '=') {
        getch();
        if (ch == '=') {
            symType = EQLSYM;
        }
        else {
            symType = '=';
            charNum--;
        }
    }
    else if (ch == '{' || ch == '}' || ch == '(' || ch == ')'|| ch == '*' || ch == '/' || ch == ';' || ch == ',' || ch == ':' || ch == '[' || ch == ']'||ch=='+'||ch=='-') {  //{ } ( ) + - * / 单个字符
        symType = ch;
    }
    else {
        cout << "配到非法单词" << endl;
        return -1;
    }
    return 0;
}


void addFunInstrIndex(string funName, int index) {
    if (funNum<MAXFUNNUM) {
        fInstrIndexList[funNum].name = funName;
        fInstrIndexList[funNum].index = index;
        funNum++;
    }
    else {
        error(TOMUCHFUN, EXIT);
    }
    return;
}




//符号表操作
void addToTable(string name, int kind, float value, int address, int paraNum, int arrayLen) {  //填表
    if (mainTable.index >= MAXSYMTABLENUM) {
        //ERROR 符号表已满
        error(OUTOFSYMTABLE, EXIT);
    }
    if (kind == FUNCTION) {  //函数
        for (int i = 1; i <= mainTable.fTotal; i++) {   //查找有无重名函数
            if (name == mainTable.elements[mainTable.fIndexList[i]].name) {
                //ERROR 函数重复定义
                error(REDECLARE, CONTINUE);
                return;
            }
        }
        mainTable.fTotal++;  //函数数量++
        mainTable.fIndexList[mainTable.fTotal] = mainTable.index;  //保存该函数起始位置索引
        mainTable.elements[mainTable.index].name = name;
        mainTable.elements[mainTable.index].kind = kind;
        mainTable.elements[mainTable.index].address=address;
        mainTable.index++;
    }
    else {  //其他
        //与当前层次符号表比较
        for (int i = mainTable.fIndexList[mainTable.fTotal]; i<mainTable.index; i++) {
            if (mainTable.elements[i].name == name) {
                //ERROR  重复定义
                error(REDECLARE, CONTINUE);
                return;
            }
        }
        
        
        mainTable.elements[mainTable.index].name = name;
        mainTable.elements[mainTable.index].kind = kind;
        mainTable.elements[mainTable.index].value = value;
        mainTable.elements[mainTable.index].address = mainTable.fTotal == 0 ? address + globalMark : address - (mainTable.elements[mainTable.fIndexList[mainTable.fTotal]].address); //局部变量存相对地址  局部变量地址从1开始 0留给返回地址
        mainTable.elements[mainTable.index].paraNum = paraNum;
        mainTable.elements[mainTable.index].arrayLen = arrayLen;
        mainTable.index++;
    }
}

int getFunDataSize(string funName) {
    int i;
    int size = 0;
    for (i = 1; i <= mainTable.fTotal; i++) {
        if (funName == mainTable.elements[mainTable.fIndexList[i]].name) {
            break;
        }
    }
    for (int j = mainTable.fIndexList[i] + 1; j<mainTable.fIndexList[i + 1] && mainTable.elements[j].name != ""; j++) {
        if (mainTable.elements[j].arrayLen == 0) {
            size++;
        }
        else {
            size = size + mainTable.elements[j].arrayLen;
        }
    }
    return size;
}

int searchInTable(string name, string funName) {
    //查找当前函数局部变量位置
    int i = 0;
    if (funName != "") {
        for (i = 1; i <= mainTable.fTotal; i++) {
            if (funName == mainTable.elements[mainTable.fIndexList[i]].name) {
                break;
            }
        }
    }
    
    //查找局部变量
    for (int j = mainTable.fIndexList[i] + 1; j<mainTable.fIndexList[i + 1] && mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            return mainTable.elements[j].address;
        }
    }
    //查找全局变量
    for (int j = 0; j<mainTable.fIndexList[1] && j <= mainTable.index&&mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            return mainTable.elements[j].address;
        }
    }
    error(NOTDECLARE, CONTINUE);
    return -1;
}
bool isConst(string name, string funName) {
    //查找当前函数局部变量位置
    int i = 0;
    if (funName != "") {
        for (i = 1; i <= mainTable.fTotal; i++) {
            if (funName == mainTable.elements[mainTable.fIndexList[i]].name) {
                break;
            }
        }
    }
    
    //查找局部变量
    for (int j = mainTable.fIndexList[i] + 1; j<mainTable.fIndexList[i + 1] && mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            if (mainTable.elements[j].kind == CONSTCHAR || mainTable.elements[j].kind == CONSTINT || mainTable.elements[j].kind == CONSTFLOAT) {
                return true;
            }
            else {
                return false;
            }
            
        }
    }
    //查找全局变量
    for (int j = 0; j<mainTable.fIndexList[1] && j <= mainTable.index&&mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            if (mainTable.elements[j].kind == CONSTCHAR || mainTable.elements[j].kind == CONSTINT || mainTable.elements[j].kind == CONSTFLOAT) {
                return true;
            }
            else {
                return false;
            }
            
        }
    }
    return false;
}

float getConstValue(string name, string funName) {
    //查找当前函数局部变量位置
    int i = 0;
    if (funName != "") {
        for (i = 1; i <= mainTable.fTotal; i++) {
            if (funName == mainTable.elements[mainTable.fIndexList[i]].name) {
                break;
            }
        }
    }
    
    //查找局部变量
    for (int j = mainTable.fIndexList[i] + 1; j<mainTable.fIndexList[i + 1] && mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            if (mainTable.elements[j].kind == CONSTCHAR || mainTable.elements[j].kind == CONSTINT || mainTable.elements[j].kind == CONSTFLOAT)
                return mainTable.elements[j].value;
        }
    }
    //查找全局变量
    for (int j = 0; j<mainTable.fIndexList[1] && j <= mainTable.index&&mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            if (mainTable.elements[j].kind == CONSTCHAR || mainTable.elements[j].kind == CONSTINT || mainTable.elements[j].kind == CONSTFLOAT)
                return mainTable.elements[j].value;
        }
    }
    error(NOTDECLARE, CONTINUE);
    return -1;
    
}
void addParaNumToTable(int paraNum) {
    mainTable.elements[mainTable.fIndexList[mainTable.fTotal]].paraNum = paraNum;
}
//判断参数个数
void findParaNumInTable(int i, string funName) {
    int j = 0;
    if (funName != "") {
        for (j = 1; j <= mainTable.fTotal; j++) {
            if (funName == mainTable.elements[mainTable.fIndexList[j]].name) {
                if (i != mainTable.elements[mainTable.fIndexList[j]].paraNum) {
                    error(WRONGPARANUM, TOSENTENCEHEADORRBRACKET);
                }
                return;
            }
        }
    }
}

int getIdentType(string name,string funName){
    //查找当前函数局部变量位置
    int i = 0;
    if (funName != "") {
        for (i = 1; i <= mainTable.fTotal; i++) {
            if (funName == mainTable.elements[mainTable.fIndexList[i]].name) {
                break;
            }
        }
    }
    
    //查找局部变量
    for (int j = mainTable.fIndexList[i] + 1; j<mainTable.fIndexList[i + 1] && mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            return mainTable.elements[j].kind;
        }
    }
    //查找全局变量
    for (int j = 0; j<mainTable.fIndexList[1] && j <= mainTable.index&&mainTable.elements[j].name != ""; j++) {
        if (name == mainTable.elements[j].name) {
            return mainTable.elements[j].kind;
        }
    }
    error(NOTDECLARE, CONTINUE);
    return -1;

}
int getFunParaNum(string funName) {
    int i;
    int paraNum = 0;
    for (i = 1; i <= mainTable.fTotal; i++) {
        if (funName == mainTable.elements[mainTable.fIndexList[i]].name) {
            break;
        }
    }
    return mainTable.elements[mainTable.fIndexList[i]].paraNum;
}

//词法分析递归子程序
void program() {   // <程序> ::= [<常量说明>] {<变量定义>;} {<有返回值函数定义>|<无返回值函数定义>}<主函数>  文法改写了  方便program中回朔判断。
    instrIndex = 1; //第一条指令留出,之后插入无条件挑战到MAIN的指令
    getsym();
    if (symType == CONSTSYM) {
        constStatement();
    }
    //变量说明和有返回值函数FIRST集合相同，预读字符
    while (symType == INTSYM || symType == CHARSYM || symType == FLOATSYM) {
        int tempSymType = symType; //保存现场 用于回朔
        int tempch = ch;
        int tempCharNum = charNum;
        getsym();
        if (symType == IDENT) {
            name = symValue;
            getsym();
            if (symType == ';') {    //如果是; 是变量定义 且只定义了单个变量，直接填表；
                value = 0;    //填表
                address = dataIndex++;
                paraNum = 0;
                kind = tempSymType == INTSYM ? VARINT : (tempSymType == CHARSYM ? VARCHAR : VARFLOAT);
                addToTable(name, kind, value, address, 0, 0);
                getsym();
                continue;
            }
            else if (symType == ',' || symType == '[') {   //预读到第三个单词 是,说明是变量定义
                symType = tempSymType; //恢复现场，继续变量说明处理
                ch = tempch;
                charNum = tempCharNum;
                varDeclaration();
                if (symType == ';') {
                    getsym();
                }
                else {
                    //ERROR 少分号
                    error(SEMICOLONMISSING, TOTYPESYM);
                }
            }
            else {   //否则不是变量说明  恢复现场
                symType = tempSymType;
                ch = tempch;
                charNum = tempCharNum;
                break;
            }
        }
        else {
            //ERROR 非法的标识符
            error(NOTASYMBOL, TOTYPESYM);
        }
    }
    //开始函数定义部分
    while (symType == INTSYM || symType == FLOATSYM || symType == CHARSYM || symType == VOIDSYM) {
        string funName;
        int index;
        if (symType != VOIDSYM) { //有返回值函数 <有返回值函数定义> ::= <声明头部>'('<参数>')' '{'<复合语句>'}'
            kind = FUNCTION;
            value = symType == INTSYM ? VARINT : (symType == CHARSYM ? VARCHAR : VARFLOAT); //用value保存返回值类型
            getsym();
            if (symType == IDENT) {
                thisFunName = symValue;
                funName = symValue;  //将函数名及入口
                index = instrIndex++;//保存ALLOC指令的位置
                name = symValue;  //填表
                address=dataIndex++;
                addToTable(name, FUNCTION, value, address, 0, 0);
                addFunInstrIndex(funName, index);//保存函数名及入口
                getsym();
                if (symType == '(') {
                    getsym();
                    parameterList();  //参数表处理
                    genPcode(STO, 0, instrIndex++);//保存返回地址
                    for (int i = getFunParaNum(funName); i>0; i--) {
                        genPcode(STO, i, instrIndex++);
                    }
                    if (symType == ')') {
                        getsym();
                        if (symType == '{') {
                            getsym();
                            compoundSentence();  //复合语句处理
                            if (symType == '}') {
                                getsym();
                                genPcode(ALLOC, getFunDataSize(funName), index);
                                genPcode(LOAD, 0, instrIndex++);//加载返回地址 相对地址0存储返回地址
                                genPcode(BRA, 0, instrIndex++);
                                cout << "这是一个函数定义" << endl;
                            }
                            else {
                                //ERROR 少}
                                error(RBRACEMISSING, TOTYPESYM);
                            }
                        }
                        else {
                            //ERROR 少{
                            error(LBRACEMISSING, TOTYPESYM);
                        }
                    }
                    else {
                        //ERROR 少）
                        error(RBRACKETMISSING, TOTYPESYM);
                    }
                }
                else {
                    //ERROR 少(
                    error(LBRACKETMISSING, TOTYPESYM);
                }
            }
            else {
                //ERROR 非法标识符
                error(NOTASYMBOL, TOTYPESYM);
            }
        }
        else { //无返回值函数及main函数
            getsym();
            if (symType == MAINSYM) {    //＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
                thisFunName = symValue;
                funName = symValue;
                index = instrIndex++; //保存ALLOC指令的位置
                name = symValue;  //填表
                value = 0;
                address = dataIndex++;
                addToTable(name, FUNCTION, value, address, 0, 0);
                genPcode(BR, index, 0);
                getsym();
                if (symType == '(') {
                    getsym();
                    if (symType == ')') {
                        getsym();
                        if (symType == '{') {
                            getsym();
                            compoundSentence();  //复合语句处理
                            if (symType == '}') {
                                genPcode(ALLOC, getFunDataSize(funName), index);
                                getsym();
                                cout << "这是一个函数定义" << endl;
                            }
                            else {
                                //ERROR 少}
                                error(RBRACEMISSING, TOTYPESYM);
                            }
                        }
                        else {
                            // ERROR 少{
                            error(LBRACEMISSING, TOTYPESYM);
                        }
                    }
                    else {
                        //ERROR 少）
                        error(RBRACKETMISSING, TOTYPESYM);
                    }
                }
                else {
                    //ERROR 少（
                    error(LBRACKETMISSING, TOTYPESYM);
                }
            }
            else {   // <无返回值函数定义> ::= void<标识符>'('<参数>')' '{'<复合语句>'}'
                if (symType == IDENT) {
                    thisFunName = symValue;
                    funName = symValue;
                    index = instrIndex++;//保存ALLOC指令的位置
                    name = symValue;  //填表
                    value = 0;
                    address = dataIndex++;
                    addToTable(name, FUNCTION, value, address, 0, 0);
                    addFunInstrIndex(funName, index);
                    getsym();
                    if (symType == '(') {
                        getsym();
                        parameterList();  //参数表处理
                        genPcode(STO, 0, instrIndex++);//保存返回地址
                        for (int i = getFunParaNum(funName); i>0; i--) {
                            genPcode(STO, i, instrIndex++);
                        }
                        if (symType == ')') {
                            getsym();
                            if (symType == '{') {
                                getsym();
                                compoundSentence();  //复合语句处理
                                
                                if (symType == '}') {
                                    getsym();
                                    genPcode(ALLOC, getFunDataSize(funName), index);
                                    genPcode(LOAD, 0, instrIndex++);//加载返回地址 相对地址0存储返回地址
                                    genPcode(BRA, 0, instrIndex++);
                                    cout << "这是一个函数定义" << endl;
                                }
                                else {
                                    //ERROR 少}
                                    error(RBRACEMISSING, TOTYPESYM);
                                }
                            }
                            else {
                                // ERROR 少{
                                error(LBRACEMISSING, TOTYPESYM);
                            }
                        }
                        else {
                            //ERROR 少）
                            error(RBRACKETMISSING, TOTYPESYM);
                        }
                    }
                    else {
                        //ERROR 少（
                        error(LBRACKETMISSING, TOTYPESYM);
                    }
                }
                else {
                    // ERROR 非法标识符
                    error(NOTASYMBOL, TOTYPESYM);
                }
            }
        }
    }
    cout << "这是一个程序" << endl;
    return;
}

void constStatement() {  // <常量说明> ::= const<常量定义>;{const<常量定义>;}
    while (symType == CONSTSYM) {
        getsym();
        if (symType == INTSYM || symType == FLOATSYM || symType == CHARSYM) {
            constDeclaration();
        }
        if (symType == ';') {
            getsym();
        }
        else {
            //ERROR 缺分号
            error(SEMICOLONMISSING, TOCONSTOR2);
        }
    }
    cout << "这是一个常量说明" << endl;
    return;
}

void constDeclaration() {            // <常量定义> ::=
    if (symType == INTSYM) {          // int<标识符> = <整数>{,<标识符> = <整数>}|
        do {
            getsym();
            if (symType == IDENT) {
                name = symValue;
                kind = CONSTINT;
                getsym();
                if (symType == '=') {
                    getsym();
                    if (symType == INTNUMBER) {
                        value = numberValue;  //填表
                        address = dataIndex++;
                        addToTable(name, kind, value, address, 0, 0);
                    }else if (symType == '+' || symType == '-') {
                        int tempSymType = symType;
                        getsym();
                        if (symType == INTNUMBER) {
                            value = tempSymType == '+' ? numberValue : -numberValue;  //填表
                            address = dataIndex++;
                            addToTable(name, kind, value, address, 0, 0);
                        }
                        else {
                            //ERROR 应为整数
                            error(WRONGTYPE, CONTINUE);
                        }
                    }
                    else {
                        //ERROR 应为整数
                        error(WRONGTYPE, CONTINUE);
                    }
                }
                else {
                    //ERROR 应为=
                    error(ASSIGNSYMMISSING, TOSEMIOR3);
                    return;
                }
            }
            else {
                //ERROR 标识符错误
                error(NOTASYMBOL, TOSEMIOR3);
                return;
            }
            getsym();
        } while (symType == ',');
        return;
    }
    else if (symType == FLOATSYM) {    // float<标识符> = <实数>{,<标识符> = <实数>}|
        do {
            getsym();
            if (symType == IDENT) {
                name = symValue;
                kind = CONSTFLOAT;
                getsym();
                if (symType == '=') {
                    getsym();
                    if (symType == FLOATNUMBER) {
                        value = numberValue;  //填表
                        address = dataIndex++;
                        addToTable(name, kind, value, address, 0, 0);
                    }else if (symType == '+' || symType == '-') {
                        int tempSymType = symType;
                        getsym();
                        if (symType == FLOATNUMBER) {
                            value = tempSymType == '+' ? numberValue : -numberValue;  //填表
                            address = dataIndex++;
                            addToTable(name, kind, value, address, 0, 0);
                        }
                        else {
                            //ERROR 应为实数
                            error(WRONGTYPE, CONTINUE);
                        }
                    }
                    else {
                        //ERROR 应为实数
                        error(WRONGTYPE, CONTINUE);
                    }
                }
                else {
                    //ERROR 应为=
                    error(ASSIGNSYMMISSING, TOSEMIOR3);
                    return;
                }
            }
            else {
                //ERROR 标识符错误
                error(NOTASYMBOL, TOSEMIOR3);
                return;
            }
            getsym();
        } while (symType == ',');
        return;
    }
    else if (symType == CHARSYM) {     // char<标识符> = <字符>{,<标识符> = <字符>}
        do {
            getsym();
            if (symType == IDENT) {
                name = symValue;
                kind = CONSTCHAR;
                getsym();
                if (symType == '=') {
                    getsym();
                    if (symType == CHAR) {
                        value = numberValue;  //填表
                        address = dataIndex++;
                        addToTable(name, kind, value, address, 0, 0);
                    }
                    else {
                        //ERROR 应为字符
                        error(WRONGTYPE, CONTINUE);
                    }
                }
                else {
                    //ERROR 应为=
                    error(ASSIGNSYMMISSING, TOSEMIOR3);
                    return;
                }
            }
            else {
                //ERROR 标识符错误
                error(NOTASYMBOL, TOSEMIOR3);
                return;
            }
            getsym();
        } while (symType == ',');
        return;
    }
    
}

void varDeclaration() {  // <变量定义> ::= <类型标识符>(<标识符>|<标识符>'['<无符号整数>']'）{,(<标识符>|<标识符>'['<无符号整数>']'）}
    if (symType == INTSYM || symType == CHARSYM || symType == FLOATSYM) {
        kind = symType == INTSYM ? VARINT : (symType == CHARSYM ? VARCHAR : VARFLOAT);
        do {
            getsym();
            if (symType == IDENT) {
                name = symValue;
                getsym();
                if (symType == '[') { //是数组定义
                    getsym();
                    if (symType == INTNUMBER) {
                        arrayLen = numberValue;
                        getsym();
                        if (symType == ']') {
                            value = 0;    //填表
                            address = dataIndex;
                            dataIndex=dataIndex+arrayLen;
                            paraNum = 0;
                            addToTable(name, kind, value, address, paraNum, arrayLen);
                            getsym();
                        }
                        else {
                            //ERROR  无]
                            error(RSQUAREBRAMISSING, TOSEMIOR2);
                        }
                    }
                    else {
                        //ERROR 数组长度应为整数
                        error(WRONGARRAYLEN, TOSEMIOR2);
                    }
                }
                else {
                    value = 0;    //填表
                    address = dataIndex++;
                    paraNum = 0;
                    addToTable(name, kind, value, address, 0, 0);
                }
            }
            else {
                //ERROR 非法的标识符
                error(NOTASYMBOL, TOSEMIOR2);
            }
        } while (symType == ',');
    }
    cout << "这是一个变量定义" << endl;
}
void parameterList() {    // <参数表> ::= <类型标识符><标识符>{,<类型标识符><标识符>}|<空>
    int i = 0;
    if (symType == INTSYM || symType == CHARSYM || symType == FLOATSYM) {
        if (symType == INTSYM || symType == CHARSYM || symType == FLOATSYM) {
            kind = symType == INTSYM ? VARINT : (symType == CHARSYM ? VARCHAR : VARFLOAT);
            getsym();
            if (symType == IDENT) {
                name = symValue;     //填表
                paraNum = 1;
                address=dataIndex++;
                addToTable(name, kind, i, address, paraNum, 0);
                i++;
                getsym();
            }
            else {
                //ERROR  非法标识符
                error(NOTASYMBOL, TORBRACKET);
                return;
            }
        }
        while (symType == ',') {
            getsym();
            if (symType == INTSYM || symType == CHARSYM || symType == FLOATSYM) {
                kind = symType == INTSYM ? VARINT : (symType == CHARSYM ? VARCHAR : VARFLOAT);
                getsym();
                if (symType == IDENT) {
                    name = symValue;     //填表
                    paraNum = 1;
                    i++;
                    address = dataIndex++;
                    addToTable(name, kind, i, address, paraNum, 0);
                    getsym();
                }
                else {
                    //ERROR  非法标识符
                    error(NOTASYMBOL, TORBRACKET);
                    return;
                }
            }
        }
    }
    addParaNumToTable(i);
    cout << "这是一个参数表" << endl;
}


void compoundSentence() {    // <复合语句> ::= [<常量说明>] {<变量定义>;}<语句列>
    if (symType == CONSTSYM) {
        constStatement();
    }
    while (symType == INTSYM || symType == CHARSYM || symType == FLOATSYM) {
        varDeclaration();
        if (symType == ';') {
            getsym();
        }
        else {
            error(SEMICOLONMISSING, TOSENTENCEHEADOR2);
        }
    }
    sentenceList();
    cout << "这是一个复合语句" << endl;
}

void sentenceList() {  // <语句列>   ::= ｛<语句>｝
    while (true) {
        if (symType == IFSYM || symType == SWITCHSYM || symType == IDENT || symType == WHILESYM || symType == RETURNSYM || symType == PRINTFSYM || symType == SCANFSYM || symType == '{' || symType == ';') {
            sentence();
        }
        else if (symType == '}') {
            break;
        }
        else {
            //  ERROR 语句非法
            error(WRONGSENTENCE, TOSENTENCEHEADORRBRACE);
        }
    }
    
    cout << "这是一个语句列" << endl;
}


void sentence() {// <语句> ::= <条件语句> | <循环语句> | '{'<语句列>'}'|<有返回值函数调用语句>;|<无返回值函数调用语句>;|<赋值语句>;|<读语句>;|<写语句>;|<空>;|<情况语句>|<返回语句>;
    if (symType == IFSYM) {
        conditionSentence();
        return;
    }
    else if (symType == SWITCHSYM) {
        switchSentence();
        return;
    }
    else if (symType == WHILESYM) {
        loopSentence();
        return;
    }
    else if (symType == RETURNSYM) {
        returnSentence();
        if (symType == ';') {
            getsym();
        }
        else {
            //ERROR 缺分号
            error(SEMICOLONMISSING, TOSENTENCEHEADORRBRACE);
        }
        return;
    }
    else if (symType == PRINTFSYM) {
        writeSentence();
        if (symType == ';') {
            getsym();
        }
        else {
            //ERROR 缺分号
            error(SEMICOLONMISSING, TOSENTENCEHEADORRBRACE);
        }
        return;
    }
    else if (symType == SCANFSYM) {
        readSentence();
        if (symType == ';') {
            getsym();
        }
        else {
            //ERROR 缺分号
            error(SEMICOLONMISSING, TOSENTENCEHEADORRBRACE);
        }
        return;
    }
    else if (symType == '{') {
        getsym();
        sentenceList();
        if (symType == '}') {
            getsym();
        }
        else {
            //ERROR 缺}
            error(RBRACEMISSING, TOSENTENCEHEADORRBRACE);
        }
        return;
    }
    else if (symType == IDENT) {
        int tempSymType = symType;  //保存现场 预读字符 判断赋值语句还是函数调用
        int tempCharNum = charNum;
        getsym();
        if (symType == '(') {  //是函数调用
            symType = tempSymType;
            charNum = tempCharNum;
            functionCallSentence();
            if (symType == ';') {
                getsym();
            }
            else {
                //ERROR 缺分号
                error(SEMICOLONMISSING, TOSENTENCEHEADORRBRACE);
            }
            return;
        }
        else if (symType == '=' || symType == '[') {  //是赋值语句
            symType = tempSymType;
            charNum = tempCharNum;
            assignSentence();
            if (symType == ';') {
                getsym();
            }
            else {
                //ERROR 缺分号
                error(SEMICOLONMISSING, TOSENTENCEHEADORRBRACE);
            }
            return;
        }
    }
    else if (symType == ';') { //空语句
        getsym();
        return;
    }
}
void conditionSentence() {  // <条件语句> ::= if '(' <条件> ')'<语句>[else<语句>]1
    int brfIndex;
    int brIndex;
    if (symType == IFSYM) {
        getsym();
        if (symType == '(') {
            getsym();
            condition(); //条件分析
            brfIndex = instrIndex++; //保留要插入跳转指令的位置 等标签位置计算出后插入
            if (symType == ')') {
                getsym();
                sentence();
                if (symType == ELSESYM) {
                    brIndex = instrIndex++;
                    genPcode(BRF, instrIndex, brfIndex); //确定else开始位置 插入跳转语句
                    getsym();
                    sentence();
                    genPcode(BR, instrIndex, brIndex);//else前无条件跳转到结束
                }
                else {
                    genPcode(BRF, instrIndex, brfIndex); //确定结束位置 插入跳转语句
                }
            }
            else {
                //ERROR 右括号缺失
                error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
            }
        }
        else {
            //ERROR 左括号缺失
            error(LBRACKETMISSING, TOSENTENCEHEADORRBRACE);
        }
    }
    cout << "这是一个条件语句" << endl;
}

void switchSentence() { // <情况语句>  ::=  switch ‘(’<表达式>‘)’ ‘{’<情况表> ‘}’
    if (symType == SWITCHSYM) {
        getsym();
        if (symType == '(') {
            getsym();
            expression(); //表达式分析
            genPcode(STO, -(++switchIndex), instrIndex++);
            if (symType == ')') {
                getsym();
                if (symType == '{') {
                    getsym();
                    switchList();
                    if (symType == '}') {
                        getsym();
                    }
                    else {
                        //ERROR 应为}
                        error(RBRACEMISSING, TOSENTENCEHEADORRBRACE);
                    }
                }
                else {
                    //ERROR 应为{
                    error(LBRACEMISSING, TOSENTENCEHEADORRBRACE);
                }
            }
            else {
                //ERROR //应为）
                error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
            }
        }
        else {
            //ERROR 应为（
            error(LBRACKETMISSING, TOSENTENCEHEADORRBRACE);
        }
    }
    cout << "这是一个switch语句" << endl;
}

void switchList() {    // <情况表>   ::=  <情况子语句>{<情况子语句>}
    if (symType == CASESYM) {
        do {
            caseSentence();
        } while (symType == CASESYM);
        switchIndex--;
    }
    else {
        //ERROR 情况表为空
        error(SWITCHLISTMISSING, TOSENTENCEHEADORRBRACE);
    }
    cout << "这是一个情况表" << endl;
}

void caseSentence() {    // <情况子语句>  ::=  case<可枚举常量>：<语句>
    int brfIndex;
    if (symType == CASESYM) {
        getsym();
        if (symType == INTNUMBER || symType == CHAR) {
            genPcode(LOAD, -(switchIndex), instrIndex++);
            genPcode(LOADI, numberValue, instrIndex++);
            genPcode(CMP, EQLSYM, instrIndex++);
            brfIndex = instrIndex++;
            getsym();
            if (symType == ':') {
                getsym();
                sentence();
                genPcode(BRF, instrIndex, brfIndex);
            }
            else {
                //ERROR 缺：
                error(COLONMISSING, TOCASEORRBRACE);
            }
        }
        else {
            //ERROR case后不是可枚举常量
            error(WRONGTYPE, TOCASEORRBRACE);
        }
    }
    cout << "这是一个情况子语句" << endl;
}
void loopSentence() {   // <循环语句>   ::=  while ‘(’<条件>‘)’<语句>
    int startLoopIndex;
    int brfIndex;
    if (symType == WHILESYM) {
        getsym();
        if (symType == '(') {
            getsym();
            startLoopIndex = instrIndex++;
            condition();
            brfIndex = instrIndex++;
            if (symType == ')') {
                getsym();
                sentence();
                genPcode(BR, startLoopIndex, instrIndex++);
                genPcode(BRF, instrIndex, brfIndex);
            }
            else {
                //ERROR 应为)
                error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
            }
        }
        else {
            //ERROR 应为（
            error(LBRACKETMISSING, TOSENTENCEHEADORRBRACE);
        }
    }
    cout << "这是一个循环语句" << endl;
}

void condition() {    // <条件> ::= <表达式> <关系运算符> <表达式> | <表达式>
    expression();
    if (symType == LEQSYM || symType == GEQSYM || symType == EQLSYM || symType == NEQSYM || symType == '>' || symType == '<') {
        int opCode = symType;
        getsym();
        expression();
        genPcode(CMP, opCode, instrIndex++);
    }
    cout << "这是一个条件" << endl;
    return;
    
}

void returnSentence() {   // <返回语句>   ::=  return[‘(’<表达式>‘)’]
    if (symType == RETURNSYM) {
        getsym();
        if (symType == '(') {
            getsym();
            expression();
            if (symType == ')') {
                getsym();
            }
            else {
                //ERROR 应为)
                error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
            }
        }
    }
    genPcode(LOAD, 0, instrIndex++);//加载返回地址 相对地址0存储返回地址
    genPcode(BRA, 0, instrIndex++);
    cout << "这是一个返回语句" << endl;
}

void writeSentence() {   // <写语句>    ::= printf ‘(’ <字符串>,<表达式> ‘)’| printf ‘(’<字符串> ‘)’| printf ‘(’<表达式>‘)’
    string stringValue;
    if (symType == PRINTFSYM) {
        getsym();
        if (symType == '(') {
            getsym();
            if (symType == STRING) {
                stringValue = symValue;
                getsym();
                if (symType == ')') {    //printf ‘(’<字符串> ‘)’
                    getsym();
                    for (int i = 0; i<stringValue.length(); i++) {
                        if (stringValue[i] != '\"') {
                            genPcode(LOADI, stringValue[i], instrIndex++);
                            genPcode(WRT, CHAR, instrIndex++);
                        }
                        
                    }
                    cout << "这是一个写语句" << endl;
                    return;
                }
                else if (symType == ',') {    //printf ‘(’ <字符串>,<表达式> ‘)’
                    getsym();
                    charCount=0;
                    intCount=0;
                    floatCount=0;
                    expression();
                    if(floatCount==0){
                        if(intCount==0){
                            if(charCount>0){
                                expType=CHAR;
                            }else{
                                expType=INTNUMBER;
                            }
                        }else{
                            expType=INTNUMBER;
                        }
                    }else{
                        expType=FLOATNUMBER;
                    }
                    if (symType == ')') {
                        getsym();
                        if (stringValue == "\"%d\"") {
                            genPcode(WRT, INTNUMBER, instrIndex++);
                        }
                        else if (stringValue == "\"%f\"") {
                            genPcode(WRT, FLOATNUMBER, instrIndex++);
                        }
                        else if (stringValue == "\"%c\"") {
                            genPcode(WRT, CHAR, instrIndex++);
                        }
                        else {
                            for (int i = 0; i<stringValue.length(); i++) {
                                if (stringValue[i] != '\"') {
                                    genPcode(LOADI, stringValue[i], instrIndex++);
                                    genPcode(WRT, CHAR, instrIndex++);
                                }
                            }
                            
                            genPcode(WRT, expType, instrIndex++);
                        }
                        cout << "这是一个写语句" << endl;
                        return;
                    }
                    else {
                        //ERROR 应为)
                        error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
                    }
                }
                else {
                    //ERROR 应为)
                    error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
                }
            }
            else {     //printf ‘(’<表达式>‘)’
                charCount=0;
                intCount=0;
                floatCount=0;
                expression();
                if(floatCount==0){
                    if(intCount==0){
                        if(charCount>0){
                            expType=CHAR;
                        }else{
                            expType=INTNUMBER;
                        }
                    }else{
                        expType=INTNUMBER;
                    }
                }else{
                    expType=FLOATNUMBER;
                }
                if (symType == ')') {
                    getsym();
                    genPcode(WRT, expType, instrIndex++);
                    cout << "这是一个写语句" << endl;
                    return;
                }
                else {
                    //ERROR 应为)
                    error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
                }
            }
        }
    }
}

void readSentence() {    // <读语句>    ::=  scanf ‘(’<标识符>{,<标识符>}‘)’
    if (symType == SCANFSYM) {
        getsym();
        if (symType == '(') {
            getsym();
            if (symType == IDENT) {
                genPcode(LOADI, searchInTable(symValue, thisFunName), instrIndex++);
                genPcode(READ, 0, instrIndex++);
                getsym();
                while (symType == ',') {
                    //查表
                    getsym();
                    if (symType == IDENT) {
                        genPcode(LOADI, searchInTable(symValue, thisFunName), instrIndex++);
                        genPcode(READ, 0, instrIndex++);
                        getsym();
                    }
                    else {
                        break;
                    }
                }
                if (symType == ')') {
                    getsym();
                    cout << "这是一个读语句" << endl;
                    return;
                }
                else {
                    //ERROR 缺）
                    error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
                }
            }
            else {
                //ERROR 非法标识符
                error(NOTASYMBOL, TOSENTENCEHEADORRBRACE);
            }
        }
        else {
            //ERROR 缺（
            error(LBRACKETMISSING, TOSENTENCEHEADORRBRACE);
        }
    }
}


void assignSentence() {    // <赋值语句> ::= <标识符> = <表达式>|<标识符>'['<表达式>']' = <表达式>
    if (symType == IDENT) {
        string tempSymValue = symValue;
        getsym();
        if (symType == '=') {  // <标识符> = <表达式>
            getsym();
            expression();
            genPcode(STO, searchInTable(tempSymValue, thisFunName), instrIndex++);
            cout << "这是一个赋值语句" << endl;
            return;
        }
        else if (symType == '[') {  // <标识符>'['<表达式>']' = <表达式>    需要判断标识符定义时是不是数组，若是，数组是否越界
            genPcode(LOADI, searchInTable(tempSymValue, thisFunName), instrIndex++);
            getsym();
            expression();
            genPcode(ADD, 0, instrIndex++);
            if (symType == ']') {
                getsym();
                if (symType == '=') {
                    getsym();
                    expression();
                    genPcode(STORIN, 0, instrIndex++);
                }
                else {
                    //ERROR 缺=
                    error(ASSIGNSYMMISSING, TOSENTENCEHEADORRBRACE);
                }
            }
            else {
                //ERROR 缺]
                error(RSQUAREBRAMISSING, TOSENTENCEHEADORRBRACE);
            }
        }
        else {
            
        }
    }
    cout << "这是一个赋值语句" << endl;
}


void functionCallSentence() {   // <有返回值函数调用语句> ::= <标识符>‘(’<值参数表>‘)’  <无返回值函数调用语句> ::= <标识符>‘(’<值参数表>‘)’
    
    if (symType == IDENT) {
        string tempSymValue = symValue;
        getsym();
        if (symType == '(') {
            getsym();
            valueOfParameterList(tempSymValue);  //需要判断参数个数
            if (symType == ')') {
                getsym();
                genPcode(JSR, getFunIndex(tempSymValue), instrIndex++);
                cout << "这是一个函数调用语句" << endl;
                return;
            }
            else {
                //ERROR 缺）
                error(RBRACKETMISSING, TOSENTENCEHEADORRBRACE);
            }
        }
    }
    
}

void valueOfParameterList(string funName) {    // <值参数表>   ::= <表达式>{,<表达式>}｜<空>
    int i = 0;
    if (symType == '+' || symType == '-' || symType == IDENT || symType == INTNUMBER || symType == FLOATNUMBER || symType == CHAR || symType == '(') {
        expression();
        i++;
        while (symType == ',') {
            getsym();
            expression();
            i++;
        }
    }
    findParaNumInTable(i, funName);
    cout << "这是一个值参数表" << endl;
}


void expression() {    // <表达式> ::= [+|-]<项>{<加法运算符><项>}
    char tempSymType;
    if (symType == '+' || symType == '-') {
        tempSymType = symType;
        if (tempSymType == '-') {
            genPcode(LOADI, 0, instrIndex++);
        }
        getsym();
        term();
        if (tempSymType == '-') {
            genPcode(SUB, 0, instrIndex++);
        }
        while (symType == '+' || symType == '-') {
            tempSymType = symType;
            getsym();
            term();
            tempSymType == '+' ? genPcode(ADD, 0, instrIndex++) : genPcode(SUB, 0, instrIndex++);
        }
    }
    else if (symType == IDENT || symType == INTNUMBER || symType == FLOATNUMBER || symType == CHAR || symType == '('||symType=='+'||symType=='-') {
        term();
        while (symType == '+' || symType == '-') {
            tempSymType = symType;
            getsym();
            term();
            tempSymType == '+' ? genPcode(ADD, 0, instrIndex++) : genPcode(SUB, 0, instrIndex++);
        }
    }
    else {
        //ERROR 非法表达式
        error(WRONGEXPRESSION, TOSENTENCEHEADORRBRACKET);
    }
    cout << "这是一个表达式" << endl;
}

void term() {  // <项> ::= <因子>{<乘法运算符><因子>}
    char tempSymType;
    if (symType == IDENT || symType == INTNUMBER || symType == FLOATNUMBER || symType == CHAR || symType == '('||symType=='+'||symType=='-') {
        factor();
        while (symType == '*' || symType == '/') {
            tempSymType = symType;
            getsym();
            factor();
            tempSymType == '*' ? genPcode(MULT, 0, instrIndex++) : genPcode(DIV, 0, instrIndex++);
        }
    }
    else {
        //ERROR 非法项
        error(WRONGEXPRESSION, TOSENTENCEHEADORADD);
    }
}

void factor() {  //因子> ::= <标识符>|<标识符>'[' <表达式>']'|<整数>|<实数>|<字符>|<有返回值函数调用语句>|'('<表达式>')'
    string tempSymValue;
    int sign = 0;
    if(symType=='+'||symType=='-'){
        if(symType=='-'){
            sign='-';
            genPcode(LOADI, 0, instrIndex++);
        }else{
            sign='+';
        }
        getsym();
    }
    if (symType == IDENT) {  // <标识符>|<标识符>'[' <表达式>']'|<有返回值函数调用语句>
        tempSymValue = symValue;
        int tempSymType = symType;
        int tempCharNum = charNum;
        getsym();
        if (symType == '[') {  // <标识符>'[' <表达式>']'
            if (!isArrIndex&&(getIdentType(tempSymValue, thisFunName)==VARFLOAT||getIdentType(tempSymValue, thisFunName)==CONSTFLOAT)) {
                floatCount++;
            }else if(!isArrIndex&&(getIdentType(tempSymValue, thisFunName)==VARINT||getIdentType(tempSymValue, thisFunName)==CONSTINT)){
                intCount++;
            }else if(!isArrIndex&&(getIdentType(tempSymValue, thisFunName)==VARCHAR||getIdentType(tempSymValue, thisFunName)==CONSTCHAR)){
                charCount++;
            }
            genPcode(LOADI, searchInTable(tempSymValue, thisFunName), instrIndex++);//加载数组首地址到栈顶
            getsym();
            isArrIndex = true;
            expression();
            isArrIndex = false;
            genPcode(ADD, 0, instrIndex++); //首地址加相对地址
            genPcode(LODTOP, 0, instrIndex++);
            if (symType == ']') {
                getsym();
                if(sign!=0){
                    error(WRONGEXPRESSION, TOSENTENCEHEADORMUL);
                }
                return;
            }
            else {
                error(WRONGEXPRESSION, TOSENTENCEHEADORMUL);
                return;
            }
        }
        else if (symType == '(') {  // <有返回值函数调用语句>   查表判断
            symType = tempSymType;
            charNum = tempCharNum;
            functionCallSentence();
            if(sign!=0){
                error(WRONGEXPRESSION, TOSENTENCEHEADORMUL);
            }
            return;
        }
        else {
            if (!isArrIndex&&(getIdentType(tempSymValue, thisFunName)==VARFLOAT||getIdentType(tempSymValue, thisFunName)==CONSTFLOAT)) {
                floatCount++;
            }else if(!isArrIndex&&(getIdentType(tempSymValue, thisFunName)==VARINT||getIdentType(tempSymValue, thisFunName)==CONSTINT)){
                intCount++;
            }else if(!isArrIndex&&(getIdentType(tempSymValue, thisFunName)==VARCHAR||getIdentType(tempSymValue, thisFunName)==CONSTCHAR)){
                charCount++;
            }
            if (isConst(tempSymValue, thisFunName)) {
                genPcode(LOADI, getConstValue(tempSymValue, thisFunName), instrIndex++);
            }
            else {
                genPcode(LOAD, searchInTable(tempSymValue, thisFunName), instrIndex++);
            }
            if(sign!=0){
                error(WRONGEXPRESSION, TOSENTENCEHEADORMUL);
            }
            return;
        }
    }
    else if (symType == INTNUMBER) {
        genPcode(LOADI, numberValue, instrIndex++);
        if(!isArrIndex){
            intCount++;
        }
        if(sign=='-'){
            genPcode(SUB, 0, instrIndex++);
        }
        getsym();
        return;
    }
    else if (symType == FLOATNUMBER) {
        genPcode(LOADI, numberValue, instrIndex++);
        if(!isArrIndex){
            floatCount++;
        }

        if(sign=='-'){
            genPcode(SUB, 0, instrIndex++);
        }
        getsym();
        return;
    }
    else if (symType == CHAR) {
        genPcode(LOADI, numberValue, instrIndex++);
        if(!isArrIndex){
            charCount++;
        }
        getsym();
        return;
    }
    else if (symType == '(') {
        getsym();
        expression();
        if (symType == ')') {
            getsym();
            return;
        }
        else {
            //ERROR 缺)
            error(WRONGEXPRESSION, TOSENTENCEHEADORMUL);
            return;
        }
    }
}

int getFunIndex(string funName) {
    for (int i = 0; i<instrIndex; i++) {
        if (fInstrIndexList[i].name == funName)
            return fInstrIndexList[i].index;
    }
    return -1;
}

void printPcode() {
    char* PcodeFileName = "//Users//wangcaimeng//Desktop/Pcode.txt";
    ofstream fout(PcodeFileName);
    for (int i = 0; i<instrIndex; i++) {
        switch (pcodes[i].instrName) {
            case LOAD:
                fout << i << ":\t" << "LOAD\t" << pcodes[i].operateNum << endl;
                break;
            case LOADI:
                fout << i << ":\t" << "LOADI\t" << pcodes[i].operateNum << endl;
                break;
            case STO:
                fout << i << ":\t" << "STO\t" << pcodes[i].operateNum << endl;
                break;
            case STORIN:
                fout << i << ":\t" << "STORIN\t" << pcodes[i].operateNum << endl;
                break;
            case ADD:
                fout << i << ":\t" << "ADD\t" << pcodes[i].operateNum << endl;
                break;
            case SUB:
                fout << i << ":\t" << "SUB\t" << pcodes[i].operateNum << endl;
                break;
            case MULT:
                fout << i << ":\t" << "MULT\t" << pcodes[i].operateNum << endl;
                break;
            case DIV:
                fout << i << ":\t" << "DIV\t" << pcodes[i].operateNum << endl;
                break;
            case BR:
                fout << i << ":\t" << "BR\t" << pcodes[i].operateNum << endl;
                break;
            case BRA:
                fout << i << ":\t" << "BRA\t" << pcodes[i].operateNum << endl;
                break;
            case BRF:
                fout << i << ":\t" << "BRF\t" << pcodes[i].operateNum << endl;
                break;
            case LODTOP:
                fout << i << ":\t" << "LODTOP\t" << pcodes[i].operateNum << endl;
                break;
            case CMP:
                fout << i << ":\t" << "CMP\t" << pcodes[i].operateNum << endl;
                break;
            case ALLOC:
                fout << i << ":\t" << "ALLOC\t" << pcodes[i].operateNum << endl;
                break;
            case JSR:
                fout << i << ":\t" << "JSR\t" << pcodes[i].operateNum << endl;
                break;
            case WRT:
                fout << i << ":\t" << "WRT\t" << pcodes[i].operateNum << endl;
                break;
            case READ:
                fout << i << ":\t" << "READ\t" << pcodes[i].operateNum << endl;
                break;
            default:
                break;
        }
    }
}

void genPcode(int instrName, float operateNum, int instrIndex) {
    pcodes[instrIndex].instrName = instrName;
    pcodes[instrIndex].operateNum = operateNum;
}
//错误处理
//para:int errorCode    错误类型码
//para:int handleCode   处理方式码
void error(int errorCode, int handleCode) {
    switch (errorCode) {
        case FILENOTFIND:
            cout << "文件读取失败！" << endl;
            break;
        case ILLEGALCHAR:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   标识符非法！" << endl;
            break;
        case DOUBLEQUOMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   双引号缺失！" << endl;
            break;
        case QUOTATIONMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   单引号缺失！" << endl;
            break;
        case REDECLARE:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   标识符重复生命！" << endl;
            break;
        case NOTDECLARE:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   标识符未声明！" << endl;
            break;
        case NOTASYMBOL:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   不是标识符！" << endl;
            break;
        case SEMICOLONMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   分号缺失！" << endl;
            break;
        case LBRACEMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   左大括号缺失！" << endl;
            break;
        case RBRACEMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   右大括号缺失！" << endl;
            break;
        case LBRACKETMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   左括号缺失！" << endl;
            break;
        case RBRACKETMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   右括号缺失！" << endl;
            break;
        case ASSIGNSYMMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   赋值符号缺失！" << endl;
            break;
        case WRONGTYPE:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   标识符类型错误！" << endl;
            break;
        case SWITCHLISTMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   情况表缺失！" << endl;
            break;
        case RSQUAREBRAMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   右方括号缺失！" << endl;
            break;
        case ILLEGALEXPRESSION:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   非法的表达式！" << endl;
            break;
        case MAINFUNCMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   标识符未声明！" << endl;
            break;
        case OUTOFSYMTABLE:
            cout << "符号表溢出" << endl;
            break;
        case WRONGARRAYLEN:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   数组大小应为整数！" << endl;
            break;
        case WRONGSENTENCE:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   语句错误！" << endl;
            break;
        case COLONMISSING:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   冒号缺失！" << endl;
            break;
        case WRONGEXPRESSION:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   表达式错误！" << endl;
            break;
        case WRONGPARANUM:
            cout << "行号:" << lastLineNum << " 字符号:" << lastCharNum << "   函数调用参数数量错误！" << endl;
            break;
        case TOMUCHFUN:
            cout << "函数个数过多（不能超过500个）" << endl;
            break;
        default:
            break;
    }
    switch (handleCode) {
        case EXIT:
            exit(-1);
            break;
        case TOTYPESYM:
            while (!(symType == INTSYM || symType == CHARSYM || symType == FLOATSYM || symType == VOIDSYM)){
                getsym();
            }
            break;
        case TOCONSTOR2:
            while (!(symType == INTSYM || symType == CHARSYM || symType == FLOATSYM || symType == CONSTSYM)) {
                getsym();
            }
            break;
        case TOSEMIOR3:
            while (!(symType == INTSYM || symType == CHARSYM || symType == FLOATSYM || symType == VOIDSYM || symType == CONSTSYM || symType == ';')) {
                getsym();
            }
            break;
        case TOSEMIOR2:
            while (!(symType == INTSYM || symType == CHARSYM || symType == FLOATSYM || symType == VOIDSYM || symType == ';')) {
                getsym();
            }
            break;
        case TORBRACKET:
            while (symType != ')') {
                getsym();
            }
            break;
        case TOSENTENCEHEADOR2:
            while (!(symType == IFSYM || symType == SWITCHSYM || symType == IDENT || symType == WHILESYM || symType == RETURNSYM || symType == PRINTFSYM || symType == SCANFSYM || symType == '{' || symType == INTSYM || symType == CHARSYM || symType == FLOATSYM || symType == VOIDSYM)) {
                getsym();
            }
            break;
        case TOSENTENCEHEADORRBRACE:
            while (!(symType == IFSYM || symType == SWITCHSYM || symType == IDENT || symType == WHILESYM || symType == RETURNSYM || symType == PRINTFSYM || symType == SCANFSYM || symType == '{' || symType == '}')) {
                getsym();
            }
            break;
        case TOCASEORRBRACE:
            while (!(symType == CASESYM || symType == '}')) {
                getsym();
            }
            break;
        case TOSENTENCEHEADORRBRACKET:
            while (!(symType == IFSYM || symType == SWITCHSYM || symType == IDENT || symType == WHILESYM || symType == RETURNSYM || symType == PRINTFSYM || symType == SCANFSYM || symType == '{' || symType == ')')) {
                getsym();
            }
            break;
        case TOSENTENCEHEADORADD:
            while (!(symType == IFSYM || symType == SWITCHSYM || symType == IDENT || symType == WHILESYM || symType == RETURNSYM || symType == PRINTFSYM || symType == SCANFSYM || symType == '{' || symType == '+' || symType == '-')) {
                getsym();
            }
            break;
        case TOSENTENCEHEADORMUL:
            while (!(symType == IFSYM || symType == SWITCHSYM || symType == IDENT || symType == WHILESYM || symType == RETURNSYM || symType == PRINTFSYM || symType == SCANFSYM || symType == '{' || symType == '*' || symType == '/')) {
                getsym();
            }
            break;
    }
}


void interpret() {
    int pc = 0; //程序计数器 表示当前执行指令位置
    int ARIndex = 0; //当前活动记录索引
    float a = 0, b = 0;//临时保存运算操作数
    while (pc<instrIndex) {
        pcode p = pcodes[pc];
        switch (p.instrName) {
            case LOAD:  //LOAD addr    地址内容加载到栈顶
                if (p.operateNum >= globalMark) { //全局变量
                    operateNumStack[++top] = dataArea[(int)p.operateNum - globalMark];
                }
                else if (p.operateNum == -1) {
                    error(0, EXIT);
                }
                else if (p.operateNum <= -2) {
                    operateNumStack[++top] = switchExp[switchIndex];
                }
                else {  //局部变量
                    operateNumStack[++top] = dataArea[(int)p.operateNum + baseAddr[ARIndex]];
                }
                pc++;
                break;
            case LOADI: //LOADI num    将num加载到栈顶
                operateNumStack[++top] = p.operateNum;
                pc++;
                break;
            case STO:   //STO addr  栈顶内容存到指定地址
                if (p.operateNum >= globalMark) { //全局变量
                    dataArea[(int)p.operateNum - globalMark] = operateNumStack[top--];
                }
                else if (p.operateNum == -1) {
                    error(0, EXIT);
                }
                else if (p.operateNum <= -2) {
                    switchExp[switchIndex] = operateNumStack[top--];
                }
                else {  //局部变量
                    dataArea[(int)p.operateNum + baseAddr[ARIndex]] = operateNumStack[top--];
                }
                pc++;
                break;
            case STORIN:   //STO 次栈顶
                if (operateNumStack[top - 1] >= globalMark) {
                    int temp = (int)operateNumStack[top - 1] - globalMark;
                    dataArea[temp] = operateNumStack[top--];
                    top--;
                }
                else {
                    int temp = (int)operateNumStack[top - 1] + baseAddr[ARIndex];
                    dataArea[temp] = operateNumStack[top--];
                    top--;
                }
                pc++;
                break;
            case SUB:   //栈顶=次栈顶-栈顶
                a = operateNumStack[top - 1];
                b = operateNumStack[top];
                top--;
                top--;
                operateNumStack[++top] = a - b;
                pc++;
                break;
            case ADD:   //栈顶=次栈顶+栈顶
                a = operateNumStack[top - 1];
                b = operateNumStack[top];
                top--;
                top--;
                operateNumStack[++top] = a + b;
                pc++;
                break;
            case MULT:   //栈顶=次栈顶*栈顶
                a = operateNumStack[top - 1];
                b = operateNumStack[top];
                top--;
                top--;
                operateNumStack[++top] = a*b;
                pc++;
                break;
            case DIV:   //栈顶=次栈顶/栈顶
                a = operateNumStack[top - 1];
                b = operateNumStack[top];
                top--;
                top--;
                operateNumStack[++top] = a / b;
                pc++;
                break;
            case BR:   //BR lab无条件跳转
                pc = p.operateNum;
                break;
            case BRA:   //BR 栈顶地址
                dataIndex = baseAddr[ARIndex--];//销毁当前运行记录
                pc = operateNumStack[top--];
                break;
            case BRF:   //BRF lab 若栈顶值为0，跳转到lab
                if (operateNumStack[top--] == 0) {
                    pc = p.operateNum;
                }
                else {
                    pc++;
                }
                break;
            case LODTOP:   //LOAD 栈顶值
                a = operateNumStack[top--];
                if (a >= globalMark) {
                    operateNumStack[++top] = dataArea[int(a - globalMark)];
                }
                else {
                    operateNumStack[++top] = dataArea[int(a + baseAddr[ARIndex])];
                }
                pc++;
                break;
            case CMP:   //CMP opCode 按照opCode执行比较操作 结果为真栈顶置1，为假栈顶置0
                a = operateNumStack[top - 1];
                b = operateNumStack[top];
                top--;
                top--;
                switch ((int)(p.operateNum)) {
                    case EQLSYM:
                        operateNumStack[++top] = a == b ? 1 : 0;
                        break;
                    case NEQSYM:
                        operateNumStack[++top] = a != b ? 1 : 0;
                        break;
                    case LEQSYM:
                        operateNumStack[++top] = a <= b ? 1 : 0;
                        break;
                    case GEQSYM:
                        operateNumStack[++top] = a >= b ? 1 : 0;
                        break;
                    case '>':
                        operateNumStack[++top] = a>b ? 1 : 0;
                        break;
                    case '<':
                        operateNumStack[++top] = a<b ? 1 : 0;
                        break;
                    default:
                        break;
                }
                pc++;
                break;
            case ALLOC:   //分配指定长度的空间
                ARIndex++;
                baseAddr[ARIndex] = dataIndex;
                dataIndex = dataIndex + p.operateNum + 1; //+1用来存返回地址
                pc++;
                break;
            case JSR:   //跳转到指定位置并将返回地址压栈
                operateNumStack[++top] = pc + 1;
                pc = p.operateNum;
                break;
            case WRT:   //WRT type  以type类型输出栈顶元素
                switch ((int)p.operateNum) {
                    case INTNUMBER:
                        printf("%d", (int)operateNumStack[top--]);
                        break;
                    case FLOATNUMBER:
                        printf("%.2f", operateNumStack[top--]);
                        break;
                    case CHAR:
                        printf("%c", (int)operateNumStack[top--]);
                        break;
                    default:
                        break;
                }
                pc++;
                break;
            case READ:   //读入一个变量存到栈顶的地址里
                char s[MAXLINELEN];
                scanf("%s", s);
                if (isLetter(s[0]) && s[1] == 0) {
                    a = s[0];
                }
                else {
                    a = atof(s);
                }
                b = operateNumStack[top--];
                if (b >= globalMark) {
                    dataArea[(int)(b - globalMark)] = a;
                }
                else {
                    dataArea[(int)(b + baseAddr[ARIndex])] = a;
                }
                pc++;
                break;
                
            default:
                break;
        }
    }
}

int main(int argc, const char * argv[]) {
    char * fileName;
    cout << "请输入要编译的文件的绝对路径:" << endl;
    //cin >> fileName;
    fileName = "//Users//wangcaimeng//Desktop//test.c";
    fin.open(fileName);
    if (!fin.is_open()) {
        cout << "文件打开失败" << endl;
        return 0;
    }
    else {
        cout << "文件打开成功" << endl;
    }
    
    word[0] = "case";
    word[1] = "char";
    word[2] = "const";
    word[3] = "else";
    word[4] = "float";
    word[5] = "if";
    word[6] = "int";
    word[7] = "main";
    word[8] = "printf";
    word[9] = "return";
    word[10] = "scanf";
    word[11] = "switch";
    word[12] = "void";
    word[13] = "while";
    for (int i = 0; i<MAXFUNNUM; i++) {
        mainTable.fIndexList[i] = MAXSYMTABLENUM;
    }
    
    fin.getline(line, MAXLINELEN);
    lineNum++;
    lineLen = string(line).length() - 1;
    program();
    fin.close();
    printPcode();
    interpret();
    return 0;
}
