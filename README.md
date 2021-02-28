# miniSQL

2019-2020 ZJU 数据库系统组队大作业  

一个精简型单用户SQL引擎 (DBMS) MiniSQL，通过字符界面输入 SQL语句实现表的建立/删除；索引的建立/删除以及表记录的插入/删除/查
找  



### 系统需求

1. 需求概述
   * 数据类型  
     只要求支持三种基本数据类型：`int`, `char(n)`, `float`，其中`char(n)`满足 $1 ≤ n ≤ 255$   
   * 表定义  
     一个表最多可以定义32个属性，各属性可以指定是否为 `unique`；支持`unique`属性的主键定义  
   * 索引的建立和删除  
     对于表的`主键`自动建立B+树索引，对于声明为`unique`的属性可以通过SQL语句由用户指定建立/删除 B+树索引（因此，所有的 B+树索引都是单属性单值的）  
   * 查找记录  
     可以通过指定用`and`连接的多个条件进行查询，支持等值查询和区间查询  
   * 插入和删除记录  
     支持每次一条记录的插入操作；支持每次一条或多条记录的删除操作。(where条件是范围时删除多条)  

2.  语法说明
   MiniSQL支持标准的SQL语句格式，每一条SQL语句以分号结尾，一条SQL语句可写在一行或多行。为简化编程，要求所有的关键字都为小写  

   * 创建表语句  
     `create table student2(
     	id int,
     	name char(12) unique,
     	score float,
     	primary key(id)
     ); ` 
   * 删除表语句  
     `drop table student2;`  
   * 创建索引语句  
     `create index stuidx on student2 ( name );`  
   * 删除索引语句  
     `drop index stuidx;`
   * 选择查询语句  
     `select * from student2 where score>95 and id<=1080100100; ` 
   * 插入记录语句插入记录语句  
     `insert into student2 values(1080197996,'name97996',100); ` 
   * 删除记录语句删除记录语句  
     `delete from student2 where score=98.5;`
   * 退出系统退出系统  
     `quit;`  
   * 执行脚本语句执行脚本语句  
     `execfile instruction0.txt;`  

   

### 模块

![模块](.\要求\模块.png)



### 分工

个人负责`Record Manager` & `Buffer Manager`  

