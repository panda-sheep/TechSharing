# [TiDB的k8s相关配置文件](https://github.com/pingcap/tidb-on-k8s)
## pd.yaml
```yaml
apiVersion: v1
kind: Service
metadata:
  name: pd
spec:
  type: NodePort
  ports:
  - name: client
    port: 2379
    protocol: TCP
    targetPort: 2379
    nodePort: 30080
  selector:
    app: pd-cluster

---

apiVersion: v1
kind: Service
metadata:
  labels:
    peer_no: pd-peer1
  name: pd1
spec:
  ports:
  - name: peer
    port: 2380
    protocol: TCP
    targetPort: 2380
  selector:
    peer_no: pd-peer1

---

apiVersion: v1
kind: Service
metadata:
  labels:
    peer_no: pd-peer2
  name: pd2
spec:
  ports:
  - name: peer
    port: 2380
    protocol: TCP
    targetPort: 2380
  selector:
    peer_no: pd-peer2

---

apiVersion: v1
kind: Service
metadata:
  labels:
    peer_no: pd-peer3
  name: pd3
spec:
  ports:
  - name: peer
    port: 2380
    protocol: TCP
    targetPort: 2380
  selector:
    peer_no: pd-peer3

---

apiVersion: v1
kind: Pod
metadata:
  labels:
    app: pd-cluster
    peer_no: pd-peer1
  name: pd-pod1
spec:
  containers:
  - image: pingcap/pd:latest
    name: pd-container1
    env:
    - name: MY_POD_IP
      valueFrom:
        fieldRef:
          fieldPath: status.podIP
    args:
    - --name=pd1
    - --cluster-id=1
    - --client-urls=http://0.0.0.0:2379
    - --peer-urls=http://0.0.0.0:2380
    - --advertise-client-urls=http://$(MY_POD_IP):2379
    - --advertise-peer-urls=http://pd1:2380
    - --initial-cluster
    - pd1=http://pd1:2380,pd2=http://pd2:2380,pd3=http://pd3:2380
    ports:
    - containerPort: 2379
      name: etcd-server
      protocol: TCP
    - containerPort: 2380
      name: peer
      protocol: TCP
    volumeMounts:
    - name: timestamp
      mountPath: /etc/localtime
  volumes:
  - name: timestamp
    hostPath:
      path: /etc/localtime
  restartPolicy: OnFailure

---

apiVersion: v1
kind: Pod
metadata:
  labels:
    app: pd-cluster
    peer_no: pd-peer2
  name: pd-pod2
spec:
  containers:
  - image: pingcap/pd:latest
    name: pd-container2
    env:
    - name: MY_POD_IP
      valueFrom:
        fieldRef:
          fieldPath: status.podIP
    args:
    - --name=pd2
    - --cluster-id=1
    - --client-urls=http://0.0.0.0:2379
    - --peer-urls=http://0.0.0.0:2380
    - --advertise-client-urls=http://$(MY_POD_IP):2379
    - --advertise-peer-urls=http://pd2:2380
    - --initial-cluster
    - pd1=http://pd1:2380,pd2=http://pd2:2380,pd3=http://pd3:2380
    ports:
    - containerPort: 2379
      name: etcd-server
      protocol: TCP
    - containerPort: 2380
      name: peer
      protocol: TCP
    volumeMounts:
    - name: timestamp
      mountPath: /etc/localtime
  volumes:
  - name: timestamp
    hostPath:
      path: /etc/localtime
  restartPolicy: OnFailure


---

apiVersion: v1
kind: Pod
metadata:
  labels:
    app: pd-cluster
    peer_no: pd-peer3
  name: pd-pod3
spec:
  containers:
  - image: pingcap/pd:latest
    name: pd-container3
    env:
    - name: MY_POD_IP
      valueFrom:
        fieldRef:
          fieldPath: status.podIP
    args:
    - --name=pd3
    - --cluster-id=1
    - --client-urls=http://0.0.0.0:2379
    - --peer-urls=http://0.0.0.0:2380
    - --advertise-client-urls=http://$(MY_POD_IP):2379
    - --advertise-peer-urls=http://pd3:2380
    - --initial-cluster
    - pd1=http://pd1:2380,pd2=http://pd2:2380,pd3=http://pd3:2380
    ports:
    - containerPort: 2379
      name: etcd-server
      protocol: TCP
    - containerPort: 2380
      name: peer
      protocol: TCP
    volumeMounts:
    - name: timestamp
      mountPath: /etc/localtime
  volumes:
  - name: timestamp
    hostPath:
      path: /etc/localtime
  restartPolicy: OnFailure
```
## tidb.yaml
```yaml
apiVersion: extensions/v1beta1
kind: Deployment
metadata:
  name: tidb
spec:
  replicas: 3
  template:
    metadata:
      labels:
        app: tidb-cluster
    spec:
      containers:
      - name: tidb
        image: pingcap/tidb:latest
        ports:
        - containerPort: 4000
        - containerPort: 10080
        args: ["-L=info", "--store=tikv", "--path=pd:2379?cluster=1", "-P=4000"]
        volumeMounts:
        - name: timestamp
          mountPath: /etc/localtime
      volumes:
      - name: timestamp
        hostPath:
          path: /etc/localtime

---

apiVersion: v1
kind: Service
metadata:
  name: tidb
spec:
  type: NodePort
  ports:
  - name: mysql
    port: 4000
    targetPort: 4000
    nodePort: 30004
  - name: status
    port: 10080
    targetPort: 10080
    nodePort: 30005
  selector:
    app: tidb-cluster
```
## tikv.yaml
```yaml
apiVersion: extensions/v1beta1
kind: Deployment
metadata:
  name: tikv
spec:
  replicas: 3
  template:
    metadata:
      labels:
        run: tikv
    spec:
      containers:
      - name: tikv
        image: pingcap/tikv:latest
        ports:
        - containerPort: 20160
        env:
        - name: MY_POD_IP
          valueFrom:
            fieldRef:
              fieldPath: status.podIP
        args: ["-S", "raftkv",  "--store=/data", "--addr=0.0.0.0:20160", "--advertise-addr=$(MY_POD_IP):20160", "--pd=pd:2379", "--cluster-id=1"]
        volumeMounts:
        - name: timestamp
          mountPath: /etc/localtime
      volumes:
      - name: timestamp
        hostPath:
          path: /etc/localtime
```

# [TiDB docker-compose](https://github.com/pingcap/tidb-docker-compose)