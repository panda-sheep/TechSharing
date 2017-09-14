<!-- 不涉及官方文档中的实验性内容 -->
# Spark 

## SparkSQL
流程

* 创建SparkSession
* 创建DataFrames
* 在DF上执行SQL语句
* 将结果DF通过writer方法写入结果表


## pyspark

* SparkConf()

    这个类通过键值对的形式来对Spark应用进行配置或者查看相关配置，这里的配置优先级，要高于Spark预先的配置

* SparkContext()
    
    Spark功能的入口，用于创建与Spark集群的连接，可以在集群上创建RDD（弹性分布式数据集）和broadcast量

    * addFile() 将文件分发到集群的所有节点
    * addPyFile() 分发并执行python脚本
    * cancelAllJobs() 
    * cancelJobGroup()
    * setJobGroup()    
    * runJob()
    * range()分布式实现的range()
* RDD()
    
    RDD相关操作

* StorageLevel()

    用于操作RDD的存储等级，在内存还是在磁盘等等

* Broadcast() 
    
    通过SparkContext.broadcast()方法创建的广播变量

* Accumlator()

    一个具有访问权限限制的变量类，支持int和float这种原始数据类型，worker tasks拥有使用+=运算符对Accumulator的写权限，而只有驱动程序能够读这个值。

* AccumlatorParam()

    一个对Accumlator的helper类，
    
    * addInPlace(value1, value2)
    * zero(value)

* MarshalSerializer/PickleSerializer
    两个对象序列化类
    
    * dumps()
    * loads()

* StatusTracker()
    
    用来监视job和stage

* Profiler()

    一个开发者API，关于profile的一些简单操作

* BasicProfiler()

    默认的Profiler，基于cProfile和Accumulator实现

## pyspark.sql 

* sql.SparkSession()

    SparkSQL的进入点，用于创建DataFrame，将DataFrame注册成表，在表上执行SQL，缓存表，读取parquet文件

    * sql(sqlQuery)
    
        执行语句然后返回结果DataFrame

    * table(tableName)

        将一个表转化为DataFrame

    * udf
        注册udf（用户自定义函数），以便在SQL语句中使用。

* sql.SQLContext()

    处理结构化数据的入口，在2.0版本中被SparkSession替代，但为了保证兼容性，目前还是保留了这个类

    * cacheTable(tableName)/uncacheTable(tableName)/clearCache()

        在内存中缓存表/删除缓存中的表/清除缓存
    
    * createDateFrame()

    * createExternalTable() 

        基于数据源中的数据集创建一个外部表

    * registerDataFrameAsTable(df, tableName)   

        把指定的DataFrame注册成临时表

    * registerFunction(name, f, returnType=StringType)

        把一改python函数注册成自定义函数，在SQL语句中使用

    * registerJavaFunction(name, javaClassName, returnType=None)

    * sql(sqlQuery)
        执行SQL，将结果以DataFrame形式返回

    * table(tableName)

    * tableNames(dbName=None)

        返回数据库中的表名列表

    * tables(dbName=None)

        返回数据库中的表名DataFrame

* sql.HiveContext()

* sql.UDFRegistration(sqlContext)

    * register(name, f, returnType=StringType)

        将Python函数注册为用户自定义函数

* sql.DataFrame(jdf, sql_ctx)

    DataFrame 相当于SparkSQL中的关系表，可以由SQLContext类来创建，自带方法十分丰富。 
* sql.Column(jc)

    DataFrame中的一列，创建Column的实例是一个语法糖

        # 1. Select a column out of a DataFrame

        df.colName
        df["colName"]

        # 2. Create from an expression
        df.colName + 1
        1 / df.colName

    也有非常丰富的方法，如排序，按位运算

* sql.Row

    DataFrame中的行
    
    可以通过row.key或者row[key]的方式，访问Row中的field

    可以通过key in row这个表达式的真假值来判断row中有没有相应的key

    * asDict(recursive=False)

        将Row转化为字典形式，支持递归转化

* sql.DataFrameNaFunctions(df)

    用于处理DataFrame中的缺失数据

    * drop(how='any', thresh=None, subset=None)

        返回一个没有null值的DataFrame

    * fill(value, subset=None)

        把null都替换成指定的value
    
    * replace(to_replace, value, subset=None)

        替换一些指定值

* sql.DataFrameStatFunctions(df)

    对DataFrame的统计功能，比如协方差

* sql.DataFrameReader(spark)

    用于从外部存储系统里读取DataFrame的接口，比如文件系统，KV存储等，通过spark.read()访问

    * csv()

        读取csv文件

    * format(source)

        指定数据源的格式

            >>> df = spark.read.format('json').load('python/test_support/sql/people.json')
            >>> df.dtypes
            [('age', 'bigint'), ('name', 'string')]

    * jdbc(url, table, column=None, lowerBound=None, upperBound=None, numPartitions=None, predicates=None, properties=None)

        读取数据库的表构建DataFrame    

        表的分区都是并行地读取。官方提醒不要在大集群上创建太多这种并行读取，否则Spark会把数据库弄崩溃。

    * json()

        读取json文件

    * load(path=None, format=None, schema=None, **options)    

        从数据源里载入数据
    
    * option(key, value)/options(**options)

        增加输入选项，目前好像只支持timeZone选项，用于指定时区来对时间戳进行解析

    * orc()

        读取orc文件

    * parquet()

        读取parquet文件

    * schema(schema)

        指定读取的schema

    * table(tableName)

    * text()

        读取txt文件

* sql.DataFrameWriter(df)

    将DataFrame写入到各种外部存储系统

    * insertInto(tableName, overwrite=False)

        将DataFrame插入到指定表里

    * jdbc(url, table, mode=None, properties=None)

    * json()

    * mode(saveMode)

        append, overwrite, error, ignore
    
    * option(key, value)

        timeZone
    
    * orc()

    * parquet()

    * partitionBy(*cols)

        输出时进行分区操作
    
    * save(path=None, format=None, mode=None, partitionBy=None, **options)

        将DataFrame保存到数据源里
    
    * saveAsTable(name, format=None, mode=None, partitionBy=None, **options)

        将DataFrame保存成指定的表

    * text

## pyspark.sql.types module

数据类型转换
* sql.types.DataType

    * fromInternal(obj)/toInternal(obj)

        将内部SQL对象转化成原生Python对象/反过来

    * json()
    * jsonvalue()
    * simpleString()
    * needConversion()

        用于指定某些对象是否需要转化

* sql.types.NullType
* sql.types.StringType
* sql.types.BinaryType
* sql.types.BoolenType
* sql.types.DateType

    * EPOCH_ORDINAL = 719163

    * fromInternal(v)

    * needConversion()

    * toInternal(d)

* sql.types.TimestampType

    * fromInternal(ts)

    * needConversion()

    * toInternal(dt)

* sql.types.DecimalType(precision=10, scale=0)

    指定定点精度的十进制数据类型，当数据范围超过六十四位有符号整形时可以使用

    * jsonValue()
    * simpleString()

* sql.types.DoubleType
* sql.types.FloatType
* sql.types.ByteType
    * simpleString()
* sql.types.IntegerType
    
    32位
    * simpleString()
* sql.types.LongType  

    64位
    * simpleString()

* sql.types.ShortType

    16位
    * simpleString()

* sql.types.ArrayType(elementType, containsNull=True)

    
    * fromInternal(obj)

    * fromJson(json)

    * jsonValue()

    * needConversion()

    * simpleString()

    * toInternal(obj)
   
* sql.types.MapType(keyType, valueType, valueContainsNull=True)

    * fromInternal(obj)

    * fromJson(json)

    * jsonValue()

    * needConversion()

    * simpleString()

    * toInternal(obj)

* sql.types.StructType(fields=None)

    这个数据类型代表一个Row，支持多种语法糖来访问field

    * add(field, data_type=None, nullable=True, metadata=None)

        增加一个新元素

    * fromInternal(obj)

    * fromJson(json)

    * jsonValue()

    * needConversion()

    * simpleString()

    * toInternal(obj)
* sql.types.StructField(name, dataType, nullable=True, metadata=None)
  
    StructType中的一个field

    * fromInternal(obj)

    * fromJson(json)

    * jsonValue()

    * needConversion()

    * simpleString()

    * toInternal(obj)

## pyspark.sql.functions module

各种内建函数，比如各种反三角函数，编解码，格式转换，不同类型对象创建

## pyspark.sql.streaming module

在2.0中新增的模块，以流的形式在后台持续执行语句，所有方法都是线程安全的，不过这个模块还没有稳定下来。应该是标准spark streaming的sql扩展。




# Difference with MySQL

* MySQL
    
        SELECT
            [ALL | DISTINCT | DISTINCTROW ]
                [HIGH_PRIORITY]
                [STRAIGHT_JOIN]
                [SQL_SMALL_RESULT] [SQL_BIG_RESULT] [SQL_BUFFER_RESULT]
                [SQL_CACHE | SQL_NO_CACHE] [SQL_CALC_FOUND_ROWS]
            select_expr [, select_expr ...]
            [FROM table_references
                [PARTITION partition_list]
            [WHERE where_condition]
            [GROUP BY {col_name | expr | position}
                [ASC | DESC], ... [WITH ROLLUP]]
            [HAVING where_condition]
            [ORDER BY {col_name | expr | position}
                [ASC | DESC], ...]
            [LIMIT {[offset,] row_count | row_count OFFSET offset}]
            [PROCEDURE procedure_name(argument_list)]
            [INTO OUTFILE 'file_name'
                    [CHARACTER SET charset_name]
                    export_options
                | INTO DUMPFILE 'file_name'
                | INTO var_name [, var_name]]
            [FOR UPDATE | LOCK IN SHARE MODE]]


## Supported Hive Features

Spark SQL supports the vast majority of Hive features, such as:

* Hive query statements, including:
    * SELECT
    
    * GROUP BY
    
    * ORDER BY
    
    * CLUSTER BY
    
    * SORT BY

* All Hive operators, including:
    * Relational operators (=, ⇔, ==, <>, <, >, >=, 
    <=, etc)
    * Arithmetic operators (+, -, *, /, %, etc)

    * Logical operators (AND, &&, OR, ||, etc)

    * Complex type constructors

    * Mathematical functions (sign, ln, cos, etc)

    * String functions (instr, length, printf, etc)

* User defined functions (UDF)

* User defined aggregation functions (UDAF)

* User defined serialization formats (SerDes)

* Window functions

* Joins

    * JOIN

    * {LEFT|RIGHT|FULL} OUTER JOIN

    * LEFT SEMI JOIN

    * CROSS JOIN

* Unions

* Sub-queries

    * SELECT col FROM ( SELECT a + b AS col from t1) t2
* Sampling
* Explain
* Partitioned tables including dynamic partition insertion
* View
* All Hive DDL Functions, including:
    * CREATE TABLE
    * CREATE TABLE AS SELECT
    * ALTER TABLE
* Most Hive Data types, including:
    * TINYINT
    * SMALLINT
    * INT
    * BIGINT
    * BOOLEAN
    * FLOAT
    * DOUBLE
    * STRING
    * BINARY
    * TIMESTAMP
    * DATE
    * ARRAY<>
    * MAP<>
    * STRUCT<>

## Hive SQL

    SELECT [ALL | DISTINCT] select_expr, select_expr, ...
    FROM table_reference
    [WHERE where_condition]
    [GROUP BY col_list [HAVING condition]]
    [   CLUSTER BY col_list
    | [DISTRIBUTE BY col_list] [SORT BY| ORDER BY col_list]
    ]
    [LIMIT number]
* 使用ALL和DISTINCT选项区分对重复记录的处理。默认是ALL，表示查询所有记录。DISTINCT表示去掉重复的记录

* Where 条件, 类似我们传统SQL的where 条件
* 目前支持 AND, OR, between, IN, NOT IN
* 不支持EXIST ,NOT EXIST
* ORDER BY 全局排序，只有一个Reduce任务
* SORT BY 只在本机做排序
* Limit 可以限制查询的记录数
        
        SELECT * FROM t1 LIMIT 5

* REGEX Column Specification

    SELECT 语句可以使用正则表达式做列选择，下面的语句查询除了 ds 和 hr 之外的所有列：

        SELECT `(ds|hr)?+.+` FROM test

* 基于Partition的查询
    * 一般 SELECT 查询会扫描整个表，使用 PARTITIONED BY 子句建表，查询就可以利用分区剪枝（input pruning）的特性
    * Hive 当前的实现是，只有分区断言出现在离 FROM 子句最近的那个WHERE 子句中，才会启用分区剪枝

* Join
        
        Syntax
        join_table: 
        table_reference JOIN table_factor [join_condition] 
        | table_reference {LEFT|RIGHT|FULL} [OUTER] JOIN table_reference join_condition 
        | table_reference LEFT SEMI JOIN table_reference join_condition 

        table_reference: 
            table_factor 
        | join_table 

        table_factor: 
            tbl_name [alias] 
        | table_subquery alias 
        | ( table_references ) 

        join_condition: 
            ON equality_expression ( AND equality_expression )* 

        equality_expression: 
            expression = expression
    
    * Hive 只支持等值连接（equality joins）、外连接（outer joins）和（left semi joins）。Hive 不支持所有非等值的连接，因为非等值连接非常难转化到 map/reduce 任务
 
    * LEFT，RIGHT和FULL OUTER关键字用于处理join中空记录的情况
    * LEFT SEMI JOIN 是 IN/EXISTS 子查询的一种更高效的实现
    * join 时，每次 map/reduce 任务的逻辑是这样的：reducer 会缓存 join 序列中除了最后一个表的所有表的记录，再通过最后一个表将结果序列化到文件系统
    * 实践中，应该把最大的那个表写在最后

    * SELECT a.* FROM a JOIN b ON (a.id = b.id)
    * SELECT a.* FROM a JOIN b 
        ON (a.id = b.id AND a.department = b.department)
    * 可以 join 多于 2 个表，例如
    SELECT a.val, b.val, c.val FROM a JOIN b 
        ON (a.key = b.key1) JOIN c ON (c.key = b.key2)
    
    * 如果join中多个表的 join key 是同一个，则 join 会被转化为单个 map/reduce 任务