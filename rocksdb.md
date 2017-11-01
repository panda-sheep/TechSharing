# rocksdb
定义
---
The RocksDB library provides a persistent key value store. Keys and values are arbitrary byte arrays. The keys are ordered within the key value store according to a user-specified comparator function.

The library is maintained by the Facebook Database Engineering Team, and is based on LevelDB, by Sanjay Ghemawat and Jeff Dean at Google.

不是个完整的数据库系统，而是个类库，用C++编写的，提供了C++和java两种API，crdb使用了cgo的方式来调用C++API

对在闪存（SSD）上的存储进行了优化，此外可以通过不同的配置来调整到不同的生产环境中，包括纯内存，闪存，硬盘，HDFS

从leveldb借用了核心代码，并借鉴了HBase的核心思想

目标
---
### 性能：
* fast storage and server workloads

* 它应该利用Flash或RAM子系统提供的高读/写速率的全部潜力。

* 它应该支持高效的点查找以及范围扫描。

* 它应该是可配置的，以支持高随机读，高更新或两者的组合。

* 其架构应支持读取放大，写入放大和空间放大的简单调整。

### 生产支持：
* RocksDB的设计必须具有对生产环境中部署和调试的，工具与实用程序的内置支持。

* 大多数主要参数应该是完全可调的，以便不同应用在不同的硬件上使用。

### 兼容性：

* 此软件的较新版本应向后兼容，因此升级到较新版本的RocksDB时，现有应用程序不需要更改。

### 高级架构:
* RocksDB是一个嵌入式键值存储，其中键和值是任意字节流。
* RocksDB按排序顺序组织所有数据，常用操作是 ***Get(key), Put(key), Delete(key) and Scan(key)***

RocksDB的三个基本结构是memtable，sstfile和logfile

* **memtable**: 是一个内存中的数据结构 - 新的写入被插入到memtable中，并且可选地写入logfile。

* **logfile**: 是一个顺序写入的存储文件

* **sstfile**: 当memtable填满时，它被刷新到存储器上的sstfile，并且可以安全地删除相应的logfile。sstfile中的数据被排序以便于轻松查找key。
    
    * [BlockBasedTable](https://github.com/facebook/rocksdb/wiki/Rocksdb-BlockBasedTable-Format) Format is the default SST table format in RocksDB
```
<beginning_of_file>
[data block 1]
[data block 2]
...
[data block N]
[meta block 1: filter block]           
[meta block 2: stats block]           
[meta block 3: compression dictionary block]  
...
[meta block K: future extended block]
[metaindex block]
[index block]
[Footer]                    
<end_of_file>
```
### 特性:

* **Gets, Iterators and Snapshots**

键和值被视为纯字节流。

键或值的大小没有限制。

***Get*** 获取一个键值对。

***MultiGet*** 获取多个键值对。

数据库中所有数据在逻辑上是一个排好的顺序。可以通过指定特定的比较规则，来制定这个顺序。

***Iterator*** 可以在数据库上做一个 ***RangeScan***，可以指定一个特定的key，然后从这个点开始扫描，也可以对key进行反向迭代。当创建 ***Iterator*** 时，会创建数据库的**consistent-point-in-time**视图，之后所用通过这个 ***Iterator*** 返回的key都来自这个视图。

***Snapshot*** 创建一个 **point-in-time** 视图，***Get*** 和 ***Iterator*** 可以从一个特定的 ***Snapshot*** 来读取值。

某种意义上，***Iterator*** 和 ***Snapshot*** 都提供了 **point-in-time** 视图，但是实现上是不同的。短生命周期的Scan最好使用 ***Iterator*** ，长生命周期的Scan最好使用 ***Snapshot***。

***Iterator*** 对与数据库的该时间点视图相对应的所有底层文件保留引用计数，在 ***Iterator*** 释放之前，这些文件不会被删除。

而 ***Snapshot*** 不会阻止文件的删除，而是保证了不会删除在 ***Snapshot*** 中可见的key。

***Snapshot*** 在数据库重启之后会消失，RocksDB库的重新加载（通过一个服务的重启），会释放所有之前存在的 ***Snapshot***。

* **Prefix Iterators**











简单例子
---

### 启动:

```c++
#include <assert>
#include "rocksdb/db.h"

rocksdb::DB* db;
rocksdb::Options options;
options.create_if_missing = true;
rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
assert(status.ok());
...
```
### 关闭

```c++
/* open the db as described above */
/* do something with db */
delete db;
```

### 读写
```c++
std::string value;
rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key1, &value);
if (s.ok()) s = db->Put(rocksdb::WriteOptions(), key2, value);
if (s.ok()) s = db->Delete(rocksdb::WriteOptions(), key1);
```

