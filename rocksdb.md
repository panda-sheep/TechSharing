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

大多数LSM引擎不支持强力的 ***RangScan*** API，由于他们需要查看每个数据文件。但大多数应用不会做一个纯随机的针对指定key的范围扫描，而是针对key的前缀进行扫描。

在RocksDB中，应用可以设置一个 ***prefix_extractor*** 来指定key的前缀。 RocksDB uses this to store blooms for every key-prefix（不知道怎么翻译）。一个指定了前缀的迭代器，可以不去那些没有相应前缀的文件中去扫描。

* **Updates**

***Put*** 向数据库中插入一个键值对，如果key在数据库中已经存在，之前的value会被覆盖。

***Write*** 可以原子地插入多个键值对，数据库可以保证，这些键值对要么一起被插入数据库，要么都不会被插入数据库，和 ***Put*** 一样，会覆盖原本存在的key的value。

* **Persistency**

RocksDB有一个事务日志，所有的 ***Put*** 都会保存在内存缓冲里（**memtable**），同时可选择地写入到事务日志中。

每个 ***Put*** 有一组标志位(通过**WriteOptions**来设置)，来确定 ***Put*** 是否会写入到事务日志中，也可以指定在 ***Put*** 声明 commit 之前，是否对事务日志发出sync调用。

在内部，RocksDB使用一个批量提交机制，来将事务批量地发给事务日志，所以可能用一个sync调用来提交多个事务。

* **Fault Tolerance**

RocksDB用校验和来检测存储中的数据是否损坏。每一个块（一般大小是4k到128k）都有自己的校验和。一个块一旦被写入存储，就不会再被更改。

RocksDB动态检测硬件支持校验和计算，并在可用时利用该支持。

* **Multi-Threaded Compactions**

一些文件被定期读入并合并形成较大的文件 - 这被称为压缩（**Compactions**）。

在删除一个key的多份拷贝的时候会需要压缩，比如覆盖一个已存在的key的时候。压缩也用来处理键的删除。经过适当设置，是可以通过多线程完成这个过程。

LSM数据库的总体写入吞吐量直接取决于压缩的速度，尤其是在SSD和内存上存储的时候。与单线程压缩相比，当数据库在SSD上时，多线程压缩持续的写入速率可能会增加多达10倍。

整个数据库都存储在一组**sstfile**上。当一个**memtable**满了，其内容写入Level-0（**L0**）的文件中。当RocksDB向**L0** flush的时候，RocksDB会删除**memtable**中的副本，并覆盖key。

RocksDB支持两种不同的压缩方式。

**Universal Style Compaction**: 所有文件都存储成**L0**，并按时间排序。压缩拾取几个按时间顺序相邻的文件，并将它们并入新的**L0**文件。所有文件都可以有重叠的key。

**Level Style Compaction**: 在数据库中分为多级存储。最新的数据存储为**L0**， 最老的数据是**Lmax**。L0文件可能有重叠的key，但其他级别不会有这种情况。一个压缩过程将拾取一个**Ln**文件，和它所有在**Ln+1**中的重叠文件，然后将它们替换成一个新的**Ln+1**文件。

**Level Style Compaction**比**Universal Style Compaction**有较低的写放大，但是有
更高的空间放大。

**MANIFEST**:这个文件在数据库中记录数据库的状态。压缩过程中的文件增删都会持久化地记录在这个文件中。事务在同步到**MANIFEST**时，通过一个批量提交算法来缓冲重复的消耗。


* **Avoiding Stalls**
后台压缩线程也用于将**memtable**中的内容刷到存储中。如果所有的后台压缩线程，都在做长时间的压缩，一个突然的写爆发将**memtable**写满，就会阻塞新的写操作。这个情况可以通过设置RocksDB来避免，比如专门设置一小组线程负责将**memtable**中的内容刷到存储中。

* **Compaction Filter**

一些应用有可能在压缩的时候想对key进行处理。比如一个支持time-to-live（TTL）的数据库，会删除那些到期的key。这可以通过应用定义的压缩过滤器来实现。如果应用向持续地删除那些超过某一时限的数据，可以使用压缩过滤器删除那些到期的记录。压缩过滤器可以使，更改某个key的value或者删除一个key这样的操作，成为压缩过程的一部分。比如一个应用可以持续地在压缩过程中进行数据清洗。

* **ReadOnly Mode**

只读模式由于无锁提供了更高的读性能。

* **Database Debug Logs**

名为*LOG* *的文件是RocksDB的详细日志文件，可以通过设置另其定期滚动。

* **Data Compression**

RocksDB支持snappy, zlib, bzip2, lz4和lz4_hc。支持对不同级别的数据使用不同的压缩算法。

* **Transaction Logs**

RocksDB通过将事务存储到日志文件来防护系统崩溃造成的损失。

* **Full Backups, Incremental Backups and Replication**

RocksDB是一个LSM数据库引擎，所以数据文件一旦被创建，就不会被重写。所以它很容易提取对应于数据库内容的时间点快照的文件名列表。

全量备份

**DisableFileDeletions** API 指示RocksDB不删除数据文件，压缩过程中不会删除那些数据库不在需要的文件。

**GetLiveFiles/GetSortedWalFiles** 一个备份应用可以使用这两个API，检索数据库中的实时文件列表并将其复制到备份位置。

**EnableFileDeletions** 备份完成后，应用要调用这个API，数据库就可以自由地回收所有不再需要的文件。

增量备份和复制

**GetUpdatesSince** 这个API的功能，和MySQL 做Repilcation的时候获取binlog的末端位置类似，获取的是事务日志的尾部，可以持续地获取事务日志中的事务，并在远程的复制和备份中应用。

**PutLogData** 一个Repilcation系统一般会在每个**Put**上注释一些metadata，以用来检测检测复制流水线中的循环。metadata只会在事务日志中存储，不会写到数据文件中。可以通过**GetUpdatesSince** API来检索这些metadata。

**GetSortedWalFiles** 事务日志在数据库目录中被创建，当日志文件不再被需要的时候，会被移动到*archive*目录，这个API可以获取全部事务文件的列表。


* **Support for Multiple Embedded Databases in the same process**

一个普通的RocksDB应用场景：应用程序将其数据集固有地分割成逻辑分区或分片。这种技术有利于应用程序负载平衡和快速恢复故障。这意味着单个服务器进程应该能够同时操作多个RocksDB数据库。

**Env** 这个环境对象可以实现这个功能。一个线程池与一个**Env**相关联。如果应用想要让多个数据库共享某个线程池，可以使相应的**Env**对那些数据库开放。

* **Block Cache -- Compressed and Uncompressed Data**

RocksDB使用LRU（最近最少使用）缓存那些被读取的块。

块缓存被分为两个独立的部分：1. 未压缩 2. 压缩

如果一个压缩块缓存被配置，数据库就会智能地避免向操作系统缓冲区中缓存数据。

* **Table Cache**

表缓存是一个结构用以缓存打开的**sstfile**描述，一个应用可以指定表缓存的最大尺寸。

* **External Compaction Algorithms**

LSM数据库的性能，很大程度上依赖于压缩算法和其实现。RocksDB除了自带的两种压缩算法，还提供了相应的API，来支持大量的来自社区开发者的算法。

*Options.disable_auto_compaction* 设置之后可以禁用原生的压缩算法。

**GetLiveFilesMetaData** 允许外部组件查看每个数据文件，然后决定数据文件的合并与压缩。

**DeleteFile** 允许外部组件对数据文件的删除操作。

* **Non-Blocking Database Access**

有些应用程序的架构是这样一种方式，即只有当数据检索调用是非阻塞的时候才从数据库中检索数据。RocksDB将数据库的一部分缓存在块高速缓存中，这些应用程序只有在该块缓存中找到数据时才检索数据。如果此调用在块高速缓存中找不到数据，则RocksDB会向应用程序返回相应的错误代码。

* **Stackable DB**

RocksDB有一个内置的包装机制，可以在数据库内核层之上添加一些功能。

**StackableDB** 比如TTL功能就是通过这个API来实现。这个API使得代码更模块化，更加干净。


* **Backupable DB**

这个特性是通过上面的API来实现的，使得RocksDB的备份变得十分简单。

[How to backup RocksDB](https://github.com/facebook/rocksdb/wiki/How-to-backup-RocksDB%3F)


* **Memtables**:
    
    * Pluggable Memtables:

    memtable的默认实现是跳表，跳表是一个排序集，当工作负载通过范围扫描交错写入时，这是一个必要的结构。有些应用程序不会交错写入和扫描，但是一些应用程序根本不进行范围扫描。对于这些应用程序，排序集可能无法提供最佳性能。因此提供了一个API来让开发者自己实现memtable。
    
    原生自带了三种memtable实现:
    
    1. 跳表
    
    2. 向量（vector）: 适用于将数据批量加载到数据库中，每个写入在向量的末尾插入一个新元素，当将memtable中的内容刷入存储时，向量中的元素将被排序并写入L0中的文件。
    
    3. 前缀哈希：强大的前缀扫描

    * Memtable Pipelining

    RocksDB支持指定任意数量的memtable，一个memtable写满后就不再改变，通过后台线程刷入存储，同时新的写入会被分配到新的memtable上。如果一个memtable刷入完成前，新的memtable刷入请求发出，就会形成*flush pipeline*。这个流水线机制提高RocksDB的写性能，尤其是在那些较慢的存储设备上。

    * Memtable Compaction
    当一个memtable被刷新到存储时，一个在线压缩过程将从输出流中删除重复的记录。类似地，如果较早的put被以后删除隐藏，那么put完全不会写入输出文件。该功能大大减少了存储和写入放大的数据大小。当将RocksDB用作生产者 - 消费者队列时，这是一个重要功能，特别是队列中的元素的生命周期非常短暂的情况下。

* **Merge Operator**

RocksDB原生支持三种记录，**put delete merge**。当压缩进程遇到合并记录时，它调用一个称为合并操作的应用程序指定的方法。合并可以将多个Put和Merge记录合并成一个。

This powerful feature allows applications that typically do read-modify-writes to avoid the reads altogether.（不太会翻译） 

它允许应用程序将操作意图记录为合并记录，并且RocksDB压缩过程将该意图延迟地应用于原始值。

[Merge Operator](https://github.com/facebook/rocksdb/wiki/Merge-Operator)


工具
---

**sst_dump** dumps all the keys-values in a sst file

**ldb** tool can put, get, scan the contents of a database. ldb can also dump contents of the MANIFEST, it can also be used to change the number of configured levels of the database. It can be used to manually compact a database.

测试
---

**make check** command runs all unit tests. 该单元测试触发了RocksDB的特定功能，不能用于大规模测试数据的正确性。

**db_stress** 用于验证数据的正确性。


性能
---
**db_bench** 进行基准测试

Performance results of a few typical workloads using Flash storage are described [here](https://github.com/facebook/rocksdb/wiki/Performance-Benchmarks). 

You can also find RocksDB performance results for in-memory workload [here](https://github.com/facebook/rocksdb/wiki/RocksDB-In-Memory-Workload-Performance-Benchmarks).



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

