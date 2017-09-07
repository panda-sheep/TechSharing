<!-- 不涉及官方文档中的实验性内容 -->
# Spark 

## SparkSQL

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

    DataFrame中的一列           
## Difference with MySQL