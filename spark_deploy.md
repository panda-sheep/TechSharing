# Spark 部署

## 0. 准备

jre: 1.8 (1.7和1.9都不可以)

Spark安装包: 内含mysql jdbc, 由我们提供

dbscale-spark-agent安装包: 由我们提供

## 1. 安装

1. 配置jre, 确保 `java -version`的显示结果为 1.8

2. 在所有Spark服务器上解压Spark安装包, 假设路径为`SPARK_HOME`

3. 选择一台Spark服务器解压dbscale-spark-agent安装包, 假设安装路径为`DSA_HOME`

## 2. 启动

1. 选择一台Spark服务器做为master节点, 执行:

```
SPARK_HOME/sbin/start-master.sh
```

假设master节点的host为`spark1`, 打开浏览器, 登录`spark1:8080`, 可以看到集群状态界面.

2. 在所有Spark服务器上执行:

```
SPARK_HOME/sbin/start-slave.sh spark://spark1:7077
```

如果对spark服务器的资源有使用限制, 例如只能使用10个cpu, 200g内存, 可以使用如下命令

```
SPARK_HOME/sbin/start-slave.sh spark://spark1:7077 -c 10 -m 200g
```

通常情况, 建议为Spark分配当前机器内存的75%.

刷新`spark1:8080`页面, 可以看到成功加入到spark集群中的节点列表

3. 在安装了dbscale-spark-agent的机器上执行

```
DSA_HOME/dsaServer.sh start
```

## 3. 关闭

1. 在安装了dbscale-spark-agent的机器上执行的

```
DSA_HOME/dsaServer.sh stop
```

2. 在所有Spark服务器上执行:

```
SPARK_HOME/sbin/stop-slave.sh
```

3. 在Spark Master上执行:
```
SPARK_HOME/sbin/stop-master.sh
```

