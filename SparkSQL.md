<!-- 不涉及官方文档中的实验性内容 -->
# Spark 

## SparkSQL

## pyspark

* SparkConf()

    这个类通过键值对的形式来对Spark应用进行配置或者查看相关配置，这里的配置优先级，要高于Spark预先的配置

* SparkContext()
    
    Spark功能的入口，用于创建与Spark集群的连接，可以在集群上创建RDD（弹性分布式数据集）和broadcast量

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

* Profiler()

    一个开发者API，关于profile的一些简单操作

* BasicProfiler()

    默认的Profiler，基于cProfile和Accumulator实现

## pyspark.sql 

* sql.SparkSession()

    SparkSQL的进入点，用于创建DataFrame，将DataFrame注册成表，在表上执行SQL，缓存表，读取parquet文件

    * sql(sqlQuery)
    
    执行语句然后返回结果DataFrame

    * table(tableName)

    将一个表转化为DataFrame




## Difference with MySQL