# NFA2DFA
将NFA转换成DFA的程序
## Build
只有一个cpp文件, NFA2DFA.cpp

你可以直接编译它, 比如

`g++ NFA2DFA.cpp -o NFA2DFA -std=c++11`

## Usage
NFA2DFA 将从标准输入读入NFA的相关信息. 当然,一般我们会将NFA信息写入一个文件然后通过重定向输入信息. 得到正确的NFA信息后, NFA2DFA将NFA转换成DFA,
并将DFA信息以csv格式打印到标准输出. 一般我们会将标准输出重定向到某个.csv文件内. 比如

`./NFA2DFA < input.txt > output.csv`

输入格式:

Section 1: 从第一个不以`//`或空白行为开头的行开始,读取NFA状态名,每个状态名占一行. 状态名读取以开头为`//`的行结束.

Section 2: 状态名读取结束后第一个不以`//`开头或空白行的行开始,读取accepting状态. accepting状态读取以开头为`//`的行结束.
如果accepting状态的状态名没在Section 1中的话,将会报错.

Section 3: accepting状态读取结束后第一个不以`//`开头或空白行开始,读取start状态. 读取以开头为`//`的行结束. 仅能有一个start状态, 如果超过一个将报错. 如果start状态没在section 1
中定义的话将报错.

Section 4: start状态读取结束后第一个不以`//`开头或空白行的行开始,读取输入符号. 读取以开头为`//`的行结束. 输入符号读取以开头为`//`的行结束. ε(空符号) 是内置的符号,不需要输入.

Section 5: 输入符号读取结束后第一个不以`//`开头或者空白行的行开始,读取NFA中的transition, 每行读取一条transition. 格式为

`from to symbol`

`from`为起点状态名,`to`为终点状态名,`symbol`为输入符号. 对于输入符号,如果要表示ε(空符号), 用`##`表示. 其他状态名或符号如果这些没在以上section中定义,将会报错.

所有空格行将被忽略.

### 输入示范
```
//nfa states
a
b
c
d
//nfa accepting states
d
//nfa start state
//only one start state
a

//input symbols
0

1
//edges
a d 0
a b ##
a c 1
```

上面的输入将构建一个如下图所示的NFA

![NFA](/example1.svg)

输出格式:

当DFA构建完成后,输出的格式为csv格式. DFA的每个状态以数字为名称. 假设DFA有n个状态那么输出的前n行为

`i, {, s1, s2, s3, s4 ...}`
表示DFA中状态i所代表的NFA转DFA构建中的subset.
n+1行到2n+1行为DFA的transition table.
最后一行为DFA的starting state.

### 输出范例

以下为上面输入范例的输出结果

```
0, {,}
1, {,a , b , }
2, {,d , }
3, {,c , }
state\input , 0, 1, accept
0, 0, 0, false
1, 2, 3, false
2, 0, 0, true
3, 0, 0, false
start state, 1

```

这对应着如下图的DFA

![NFA](/example2.svg)

或者

![NFA](/example3.svg)

