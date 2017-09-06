# Spark 

## SparkSQL

## PySpark

* SparkConf()

    这个类通过键值对的形式来对Spark应用进行配置或者查看相关配置，这里的配置优先级，要高于Spark预先的配置

* SparkContext()
    
    Spark功能的入口，用于创建与Spark集群的连接，可以在集群上创建RDD（弹性分布式数据集）和broadcast变量

    * addFile() 将文件分发到集群的所有节点
    * addPyFile() 分发并执行python脚本
    * cancelAllJobs() 
    * cancelJobGroup()
    * setJobGroup()    
    * runJob()
    * range() 分布式实现的range()
* RDD()
    
    RDD相关操作

* StorageLevel()

    用于操作RDD的存储等级，在内存还是在磁盘等等

* Broadcast() 
    
    通过SparkContext.broadcast()方法创建的广播变量

* Accumlator()

    一个具有访问权限限制的变量类，支持int和float这种原始数据类型，worker tasks拥有使用+=运算符对Accumulator的写权限，而只有驱动程序能够读这个值。

* AccumlatorParam()

    一个对Accumlator的helper类，
    
    * addInPlace(value1, value2)
    * zero(value)


    


## Difference with MySQL