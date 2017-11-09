# metadata

## Keys

Cockroach keys are arbitrary byte arrays. Keys come in two flavors: 

1. system keys 
2. table data keys

System keys are used by Cockroach for internal data structures and metadata. 

Table data keys contain SQL table data (as well as index data). 

System and table data keys are prefixed in such a way that all system keys sort before any table data keys.

System keys come in several subtypes:

* **Global keys** store cluster-wide data such as the "meta1" and "meta2" keys as well as various other system-wide keys such as the node and store ID allocators.

* **Store local keys** are used for unreplicated store metadata (e.g. the StoreIdent structure). "Unreplicated" indicates that these values are not replicated across multiple stores because the data they hold is tied to the lifetime of the store they are present on.

* **Range local keys** store range metadata that is associated with a global key. Range local keys have a special prefix followed by a global key and a special suffix. For example, transaction records are range local keys which look like: 

        \x01k<global-key>txn-<txnID>.
    
* **Replicated Range ID local keys** store range metadata that is present on all of the replicas for a range. These keys are updated via Raft operations. Examples include the range lease state and abort cache entries.
    
* **Unreplicated Range ID local keys** store range metadata that is local to a replica. The primary examples of such keys are the Raft state and Raft log.

Table data keys are used to store all SQL data. Table data keys contain internal structure as described in the section on [mapping data between the SQL model and KV](https://github.com/cockroachdb/cockroach/blob/master/docs/design.md#data-mapping-between-the-sql-model-and-kv).

## Logical Map Content

Logically, the map contains a series of reserved system key/value pairs preceding the actual user data (which is managed by the SQL subsystem).

`\x02<key1>`: Range metadata for range ending `\x03<key1>`. This a "meta1" key.

...

`\x02<keyN>`: Range metadata for range ending `\x03<keyN>`. This a "meta1" key.

`\x03<key1>`: Range metadata for range ending `<key1>`. This a "meta2" key.

...

`\x03<keyN>`: Range metadata for range ending `<keyN>`. This a "meta2" key.

`\x04{desc,node,range,store}-idegen`: ID generation oracles for various component types.

`\x04status-node-<varint encoded Store ID>`: Store runtime metadata.

`\x04tsd<key>`: Time-series data key.

`<key>`: A user key. In practice, these keys are managed by the SQL subsystem, which employs its own key anatomy.

## Range Metadata

1. 为了解决特大数据量的问题，range metadata是分布式的

The default approximate size of a range is 64M (2^26 B). 

In order to support 1P (2^50 B) of logical data, metadata is needed for roughly 2^(50 - 26) = 2^24 ranges.

A reasonable upper bound on range metadata size is roughly 256 bytes (3*12 bytes for the triplicated node locations and 220 bytes for the range key itself). 

2^24 ranges * 2^8 B would require roughly 4G (2^32 B) to store--too much to duplicate between machines. 

Our conclusion is that range metadata must be distributed for large installations.
 
---

2. 两级 range metadata

To keep key lookups relatively fast in the presence of distributed metadata, we store all the top-level metadata in a single range (the first range).

These top-level metadata keys are known as meta1 keys, and are prefixed such that they sort to the beginning of the key space. 

Given the metadata size of 256 bytes given above, a single 64M range would support 64M/256B = 2^18 ranges, which gives a total storage of 64M * 2^18 = 16T. 

To support the 1P quoted above, we need two levels of indirection, where the first level addresses the second, and the second addresses user data. 

With two levels of indirection, we can address 2^(18 + 18) = 2^36 ranges; each range addresses 2^26 B, and altogether we address 2^(36+26) B = 2^62 B = 4E of user data.

---

For a given user-addressable `key1`, the associated *meta1* record is found at the successor(继承者) key to `key1` in the *meta1* space. 

Since the *meta1* space is sparse(稀疏), the successor key is defined as the next key which is present. 

The *meta1* record identifies the range containing the *meta2* record, which is found using the same process. 

The *meta2* record identifies the range containing `key1`, which is again found the same way (see examples below).

Concretely, metadata keys are prefixed by `\x02` (meta1) and `\x03` (meta2); 
the prefixes `\x02` and `\x03` provide for the desired sorting behaviour. Thus, `key1`'s meta1 record will reside at the successor key to `\x02<key1>`.

---

Note: we append the **end key** of each range to meta{1,2} records because the RocksDB iterator only supports a Seek() interface which acts as a Ceil(). 

Using the **start key** of the range would cause Seek() to find the key after the meta indexing record we’re looking for, which would result in having to back the iterator up, an option which is both less efficient and not available in all cases.

---

3. 示例

The following example shows the directory structure for a map with three ranges worth of data. 

Ellipses indicate additional key/value pairs to fill an entire range of data. 

For clarity, the examples use `meta1` and `meta2` to refer to the prefixes `\x02` and `\x03`. 

Except for the fact that splitting ranges requires updates to the range metadata with knowledge of the metadata layout, the range metadata itself requires no special treatment or bootstrapping.

**Range 0** (located on servers dcrama1:8000, dcrama2:8000, dcrama3:8000)

* `meta1\xff`: dcrama1:8000, dcrama2:8000, dcrama3:8000
* `meta2<lastkey0>`: dcrama1:8000, dcrama2:8000, dcrama3:8000
* `meta2<lastkey1>`: dcrama4:8000, dcrama5:8000, dcrama6:8000
* `meta2\xff`: dcrama7:8000, dcrama8:8000, dcrama9:8000
* `...`
* `<lastkey0>`: `<lastvalue0>`

**Range 1** (located on servers dcrama4:8000, dcrama5:8000, dcrama6:8000)

* `...`
* `<lastkey1>`: `<lastvalue1>`

**Range 2** (located on servers dcrama7:8000, dcrama8:8000, dcrama9:8000)

* `...`
* `<lastkey2>`: `<lastvalue2>`

---
Consider a simpler example of a map containing less than a single range of data. 

In this case, all range metadata and all data are located in the same range:

**Range 0** (located on servers dcrama1:8000, dcrama2:8000, dcrama3:8000)

* `meta1\xff`: dcrama1:8000, dcrama2:8000, dcrama3:8000
* `meta2\xff`: dcrama1:8000, dcrama2:8000, dcrama3:8000
* `<key0>`: `<value0>`
* ...

---

Finally, a map large enough to need both levels of indirection would look like (note that instead of showing range replicas, this example is simplified to just show range indexes):

**Range 0**

* `meta1<lastkeyN-1>`: Range 0
* `meta1\xff`: Range 1
* `meta2<lastkey1>`: Range 1
* `meta2<lastkey2>`: Range 2
* `meta2<lastkey3>`: Range 3
* `...`
* `meta2<lastkeyN-1>`: Range 262143

Range 1

* `meta2<lastkeyN>`: Range 262144
* `meta2<lastkeyN+1>`: Range 262145
* `...`
* `meta2\xff`: Range 500,000
* `...`
* `<lastkey1>`: `<lastvalue1>`

Range 2

* `...`
* `<lastkey2>`: `<lastvalue2>`

Range 3

* `...`
* `<lastkey3>`: `<lastvalue3>`

Range 262144

* `...`
* `<lastkeyN>`: `<lastvalueN>`

Range 262145

* `...`
* `<lastkeyN+1>`: `<lastvalueN+1>`

Note that the choice of range 262144 is just an approximation. 

The actual number of ranges addressable via a single metadata range is dependent on the size of the keys. 

If efforts are made to keep key sizes small, the total number of addressable ranges would increase and vice versa.

---

From the examples above it’s clear that key location lookups require at most three reads to get the value for `<key>`:

* lower bound of `meta1<key>`
* lower bound of `meta2<key>`
* `<key>`

For small maps, the entire lookup is satisfied in a single RPC to Range 0. 

Maps containing less than 16T of data would require two lookups. 

Clients cache both levels of range metadata, and we expect that data locality for individual clients will be high.

Clients may end up with stale cache entries. 

If on a lookup, the range consulted does not match the client’s expectations, the client evicts the stale entries and possibly does a new lookup.

## Splitting / Merging Ranges

Once the new replicas are fully up to date, the range metadata is updated and old, source replica(s) deleted if applicable.

A node with spare capacity is chosen in the same datacenter and a special-case split is done which simply duplicates the data 1:1 and resets the range configuration metadata.

