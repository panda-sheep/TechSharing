 # Kubernetes 

目前，是仅次于Linux的第二大开源项目。

通常，我们可以将k8s看作Docker的上层架构，就像Java和J2EE的关系：J2EE是以Java为基础的企业级软件架构，k8s以Docker为基础打造了云计算时代的分布式系统架构。

基于容器技术，实现资源管理的自动化，跨多个数据中心的资源利用率的最大化。

## 服务

在k8s中，Service是分布式集群架构的核心：

* 拥有唯一制定的名字（如mysql-server）
* 拥有一个虚拟IP和端口号
* 能够提供某种远程服务能力
* 被映射到了提供这种服务能力的一组容器应用上

Service的服务进程基于Socket通信方式对外提供服务，或者实现了某个具体业务的一个特定的TCP Server进程。

k8s对服务有内建的负载均衡和故障恢复机制，服务的一些属性（如IP），也不会发生变化。

## Pod

容器提供了隔离功能，我们将提供服务的一组进程放入容器中进行隔离，然后再放入Pod中。为建立Pod和服务之间的关系，k8s提供了标签功能：

* 我们可以给所有运行MySQL的Pod贴上*name=mysql*标签，运行PHP的Pod贴上*name=php*标签
* 之后为具体的服务定义标签选择器，比如MySQL服务的标签选择器的条件可以设为*name=mysql*，这样MySQL服务就与*name=mysql*标签下的一组Pod形成了映射

Pod运行在节点环境中，节点可以是物理机也可以是虚拟机。通常一个节点中，可以运行几百个Pod。

Pod中运行着一个Pause容器，和若干业务容器。业务容器共享Pause容器的网络栈和Volume挂载卷，从而使业务容器间通信与数据交换更为高效，所以我们最好将一组紧密相关的服务放到同一个Pod中。

## 节点

主从结构，一个Master节点，一群工作节点。

**Master**，整个集群的资源管理，Pod调度，弹性伸缩，安全控制，系统监控和纠错：
* kube-apiserver: 提供了HTTP Rest接口的关键服务进程，k8s中所有资源的增删改查的唯一入口，也是集群控制的入口进程。
* kube-controller-manager：k8s中所有资源对象的自动化控制中心
* kube-scheduler：负责Pod的调度
* etcd：k8s中所有资源对象的数据保存在etcd中

**Node**（工作节点），运行真正的应用程序，以Pod为最小运行单元，负责Pod的创建、启动、监控、重启、销毁，以及软件模式的负载均衡器：
* Pods
* kubelet：负责容器的创建、启停，与Master密切协作，实现集群管理的基本功能
* kube-proxy：实现k8s上服务的通信与负载均衡机制
* Docker Engine: 负责容器的创建和管理工作

### Volume (存储卷)

Pod中多个容器访问的共享目录，可以在RC的模板定义中指定挂载目录

### PV(Persistent Volume)

Volume是Pod内的，PV是k8s集群中的一块网络存储。

### Namespace

用于实现多租户的资源隔离。



## 服务扩容与服务升级 

在k8s中，服务扩容只需要为服务所关联的Pod创建RC（Replication Controller）。

### RC(Replication Controller)

RC的定义文件中包括三个关键信息：

* 目标Pod的定义
* 目标Pod需要运行的副本数量（Replicas）
* 要监控的目标Pod的标签

k8s根据标签统计出Pod数量，少于RC定义的副本数量时，则根据目标Pod的定义创建新的Pod，并自动调度到合适的Node上运行。

主要功能：

* 通过定义一个RC实现Pod的创建过程
* 通过改变RC中副本数量，实现对Pod的扩容缩容
* 通过改变RC中模板，实现Pod的滚动升级

### RS(Replica Set) 和 Deployment

RS是RC的直接升级版，支持集合形式的label selector，而RC是等式形式的label selector。

Deployment则通过构造RS的形式，实现比RC更好的部署功能：

* 能够查看部署的进度
* 可以回滚Deployment的版本
* 可以暂停Deployment，进行修改

### Horizontal Pod Autoscaler

横向Pod自动扩容。

两种标准计算负载：

* CPU Utilization Percentage——目标Pod所有副本自身CPU利用率平均值
* 应用自定义的度量标准，如TPS或QPS

不单单是扩容，在高峰后也能自动缩容

### StatefulSet

是RC的一个变种，负责处理如MySQL，Zookeeper这类有状态，需要持久化存储的服务。

与PV卷捆绑，从而存储Pod的状态数据。


## 服务管理

### 配置管理

#### ConfigMap

* 生成容器内的环境变量
* 设置容器启动命令的启动参数
* 以Volume的形式挂载为容器内部的文件或目录

#### Downward API

用于在容器内获得Pod的信息，如IP，处于哪个Namespace下。

### Pod调度

1. Deployment/RC 全自动调度
2. NodeSelector 定向调度
    * 通过标签选择器，将Pod调度到指定的Node上
3. NodeAffinity Node 亲和性调度
    * 用于替换NodeSelector的调度方式
        * Required: 必须满足指定的规则才可以调度Pod到Node上
        * Preferred:强调优先满足规则，软限制，可以设定不同规则的权重
4. PodAffinity Pod亲和与互斥调度策略
5. Taints 和 Tolerations 污点和容忍
    * Taints是让Node拒绝某些Pod运行，需与Toleration配合，让Pod避开那些不适合的Node
6. Daemonset 在每个Node上调度一个Pod
    * 用于管理在集群内每个节点上都运行，并且只运行一个Pod副本的应用
7. Job 批处理调度
    * Job Template Expansion模式：一个Job对象对应一个待处理的Work item，适合Work item数量少，每个Work item处理数据量大的场景。
    * Queue with Pod Per Work item：一个Job对象对应一个Work item队列，Job对象启动与Work item数量相当的Pod
    * Queue with Variable Pod Count：与上面的模式类似，但Pod与Work item不一一对应
8. Cronjob 定时任务
9. 自定义调度器

## 开发

### Restful API
### Java 客户端

## 运维

### Node 管理
#### 隔离与恢复
* 可以将Node纳入调度范围，也可以将Node脱离调度范围。
#### 扩容
* 自动注册机制，将新的Node加入到集群中
### 更新Label
### Namespace 集群环境共享与隔离
### 资源管理
#### 计算资源管理
* CPU和内存
* 有监控和调度机制
#### LimitRange 资源的配置范围管理
* 对资源申请范围的约束和管理
#### Resource QoS 资源的服务质量管理
* 资源可靠性分级，对可靠性需求高的Pod优先申请可靠资源
#### 资源的配额管理
### 资源紧张时的Pod驱逐机制
1. 驱逐策略
    * kubelet会停止一个或多个Pod来防止计算资源被耗尽
2. 驱逐信号
    * kubelet根据不同的信号触发不同的行为
3. 驱逐阈值
4. 驱逐监控频率
### Pod Disruption Budget 主动驱逐保护
* 节点维护或升级
* 对应用的自动缩容操作
### 高可用部署方案
1. etcd高可用部署
    * etcd在整个集群中处于中心数据库的地位
2. k8s Master 高可用部署
    * Master是总控中心
### 集群监控
1. cAdvisor
2. Heapster+Infuxdb+Grafana 监控平台
### 集群日志管理
推荐 Fluentd+Elasticsearch+Kibana
### Web UI 管理集群
kubernetes-dashboard
### Helm 应用包管理工具

##

yum install python-rhsm-certificates.x86_64
yum install python-rhsm.x86_64

[认证](http://blog.csdn.net/jinzhencs/article/details/51435020)

http://bbotte.com/kvm-xen/let-kubernetes-k8s-run-in-docker/