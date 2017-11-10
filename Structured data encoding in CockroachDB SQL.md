# Structured data encoding in CockroachDB SQL

Tables (primary indexes)
---
SQL tables consist of a rectangular array of data and some metadata. 

数据和元数据

The metadata include a unique table ID; a nonempty list of primary key columns, each with an ascending/descending designation; and some information about each column. 

唯一表ID，主键列的列表，每个列的信息

Each column has a numeric ID that is unique within the table, a SQL type, and a column family ID. 

每个列有自己的ID，数据类型，族ID

A column family is a maximal subset of columns with the same column family ID.

列族是拥有同样族ID的列的最大子集 

For more details, see [pkg/sql/sqlbase/structured.proto]().

Each row of a table gives rise to one or more KV pairs, one per column family as needed (see subsection NULL below). 

每一行会有一到多个键值对，每个列族会需要一个

CRDB stores primary key data in KV keys and other data in KV values so that it can use the KV layer to prevent duplicate primary keys. 

主键数据在Key中存储，其他数据在value中。这样可以在KV层防止主键重复（还是多个主键？）

For encoding, see [pkg/sql/rowwriter.go](). For decoding, see  [pkg/sql/sqlbase/rowfetcher.go]().

### Key encoding

KV keys consist of several fields:

* The table ID 表ID
* The ID of the primary index (see section Indexes below) 主索引ID
* The primary key of the row, one field per primary key column in list order每一行的主键
* The column family ID.族ID
* When the previous field is nonzero (non-sentinel), its length in bytes.以字节计的长度

CRDB encodes these fields individually and concatenates(链接) the resulting bytes. 

对每部份分别编码然后链接

The decoder can determine the field boundaries because the field encoding is prefix-free.

因为无前缀，所以可以知道边界（？）

Encoded fields start with a byte that indicates the type of the field. 

首部一位用于表明类型。

For primary key fields, this type has a one-to-many relationship with the SQL datum type. 

对于主键，这个类型与SQL数据类型有一对多的关系

The SQL types STRING and BYTES, for example, share an encoding. 

举个例子，字符串类型和byte型用一样的编码

The relationship will become many-to-many when CRDB introduces a new DECIMAL encoding, since the old decoder will be retained for backward compatibility.

这种关系会变成多对多的。CRDB引进了新的十进制（不知道现在有没有）编码，届时，老的解码器保留向后兼容性。

The format of the remaining bytes depends on the field type. 

每个feild的类型决定了存储的字节的格式

The details (in [pkg/util/encoding/encoding.go]()) are irrelevant(无关) here except that, for primary key fields, these bytes have the following order property. 

除了主键以外的细节不用太关心，主键按照下面的规则编码。

Consider a particular primary key column and let enc be the mathematical function that maps SQL data in that column to bytes.

考虑到一个特定的主键列，令enc称为一个数学函数，来将SQL数据映射成byte

* If the column has an ascending designation, then for data x and y, enc(x) ≤ enc(y) if and only if x ≤ y.
* If the column has a descending designation, then for data x and y, enc(x) ≤ enc(y) if and only if x ≥ y.

In conjunction(结合) with prefix freedom, the order property ensures that the SQL layer and the KV layer sort primary keys the same way.

结合无前缀，这个规则令SQL层和KV层对主键排序是一种方式。

For more details on primary key encoding, see `EncodeTableKey` ([pkg/sql/sqlbase/table.go]()). See also EncDatum ([pkg/sql/sqlbase/encoded_datum.go]()).

### Value encoding

KV values consist of

* A four-byte checksum covering the whole KV pair 四字节的整个键值对的校验和
* A one-byte value type (see the enumeration ValueType in [pkg/roachpb/data.proto]())一字节的值类型
* Data from where the row specified in the KV key intersects the specified column family, excluding primary key data.KV键中指定的行与指定列族相交的数据，不包括主键数据。

The value type defaults to `TUPLE`, which indicates the following encoding.

默认是元祖类型，按下面的编码方式编码

(For other values, see subsection Single-column column families below.) 

For each column in the column family sorted by column ID, encode the column ID difference and the datum(资料) encoding type (unrelated to the value type!) jointly, followed by the datum itself. 

对于按列id排序的列族中的每个列，联合编码列ID差和数据编码类型，然后由数据本身进行编码。

The column ID difference is the column ID minus the previous column ID if this column is not the first, else the column ID. 

如果这个列不是第一列，列id差是列id减去前面的列id，否则是列ID。

The joint（共同的） encoding is commonly one byte, which displays conveniently in hexadecimal as the column ID difference followed by the datum encoding type.

联合编码通常是一个字节，它以十六进制格式方便地显示为列id差，然后是数据编码类型。

The Go function that performs the joint encoding is `encodeValueTag` ([pkg/util/encoding/encoding.go]()), which emits an unsigned integer with a variable-length encoding. 

最后出来的是一个用变长编码的无符号整形

The low four bits of the integer contain the datum encoding type. 

整形的低四位包含数据的编码类型

The rest contain the column ID difference.

剩下的是列ID差

As an alternative for datum encoding types greater than 14, `encodeValueTag` sets the low four bits to `SentinelType` (15) and emits the actual datum encoding type next.

至于数据编码类型大于14的……
### Sentinel（哨兵） KV pairs

The column family with ID 0 is special because it contains the primary key columns. 

列族ID为0的包含了主键列

The KV pairs arising from this column family are called sentinel KV pairs. CRDB emits(发出) sentinel KV pairs regardless of whether the KV value has other data, to guarantee that primary keys appear in at least one KV pair.

0列族中的键值对称为哨兵键值对。CRDB产生哨兵对会无视value中有没有数据，来保证主键至少在一个键值对中出现 

(Even if there are other column families, their KV pairs may be suppressed; see subsection NULL below.)

### Single-column column families

Before column families (i.e., in format version 1), non-sentinel KV keys had a column ID where the column family ID is now. 

Non-sentinel KV values contained exactly one datum, whose encoding was indicated by the one-byte value type (see `MarshalColumnValue` in [pkg/sql/sqlbase/table.go]()).

非哨兵键值对中，value只包含一个数据，其编码由一字节的类型位指定

Unlike the `TUPLE` encoding, this encoding did not need to be prefix-free, which was a boon for strings.

与元组编码不同，这个编码不需要无前缀，这对字符串十分友好。

On upgrading to format version 2 or higher, CRDB puts each existing column in a column family whose ID is the same as the column ID. This allows backward-compatible encoding and decoding. 

更新到格式版本2或者更高……

The encoder uses the old format for single-column column families when the ID of that column equals the DefaultColumnID of the column family ([pkg/sql/sqlbase/structured.proto]()).

### NULL

SQL NULL has no explicit encoding in tables (primary indexes). 

NULL没有特别的编码

Instead, CRDB encodes each row as if the columns where that row is null did not exist. 

如果是NULL，就当不存在

If all of the columns in a column family are null, then the corresponding KV pair is suppressed. 

如果列族中的所有列都为NULL，则相应的KV对被压缩（？）。

The motivation for this design is that adding a column does not require existing data to be re-encoded.

为了不再浪费时间重编码

### Example dump

The commands below create a table and insert some data. An annotated KV dump follows.
``` SQL
CREATE TABLE accounts (
  id INT PRIMARY KEY,
  owner STRING,
  balance DECIMAL,
  FAMILY f0 (id, balance),
  FAMILY f1 (owner)
);

INSERT INTO accounts VALUES
  (1, 'Alice', 10000.50),
  (2, 'Bob', 25000.00),
  (3, 'Carol', NULL),
  (4, NULL, 9400.10),
  (5, NULL, NULL);
```

Here is the relevant output from `cockroach debug rocksdb scan --value_hex`, with annotations.

    /Table/51/1/1/0/1489427290.811792567,0 : 0xB244BD870A3505348D0F4272
           ^- ^ ^ ^                            ^-------^-^^^-----------
           |  | | |                            |       | |||
           Table ID (accounts)                 Checksum| |||
              | | |                                    | |||
              Index ID                                 Value type (TUPLE)
                | |                                      |||
                Primary key (id = 1)                     Column ID difference
                  |                                       ||
                  Column family ID (f0)                   Datum encoding type (Decimal)
                                                           |
                                                           Datum encoding (10000.50)

    /Table/51/1/1/1/1/1489427290.811792567,0 : 0x30C8FBD403416C696365
           ^- ^ ^ ^ ^                            ^-------^-^---------
           |  | | | |                            |       | |
           Table ID (accounts)                   Checksum| |
              | | | |                                    | |
              Index ID                                   Value type (BYTES)
                | | |                                      |
                Primary key (id = 1)                       Datum encoding ('Alice')
                  | |
                  Column family ID (f1)
                    |
                    Column family ID encoding length

    /Table/51/1/2/0/1489427290.811792567,0 : 0x2C8E35730A3505348D2625A0
                ^                                          ^-----------
                2                                          25000.00

    /Table/51/1/2/1/1/1489427290.811792567,0 : 0xE911770C03426F62
                ^                                          ^-----
                2                                          'Bob'

    /Table/51/1/3/0/1489427290.811792567,0 : 0xCF8B38950A
                ^
                3

    /Table/51/1/3/1/1/1489427290.811792567,0 : 0x538EE3D6034361726F6C
                ^                                          ^---------
                3                                          'Carol'

    /Table/51/1/4/0/1489427290.811792567,0 : 0x247286F30A3505348C0E57EA
                ^                                          ^-----------
                4                                          9400.10

    /Table/51/1/5/0/1489427290.811792567,0 : 0xCB0644270A
                ^
                5

### Composite encoding

There exist decimal numbers and collated strings that are equal but not identical, e.g., 1.0 and 1.000. 

存在十进制数和经过排序的字符串，它们相等但不相同。

This is problematic(有问题) because in primary keys, 1.0 and 1.000 must have the same encoding, which precludes lossless decoding. 

这里有个问题是，在主键中1.0和1.000是一样的编码，形成了有损的编码

Worse, the encoding of collated strings in primary keys is defined by the [Unicode Collation Algorithm](http://unicode.org/reports/tr10/), which may not even have [an efficient partial inverse](https://stackoverflow.com/questions/23609457/invert-unicode-string-collation-keys).

When collated strings and [(soon) decimals](https://github.com/cockroachdb/cockroach/issues/13384#issuecomment-277120394) appear in primary keys, they have composite encoding. 

解决办法应该就是这个混合编码

For collated strings, this means encoding data as both a key and value, with the latter appearing in the sentinel KV value (naturally, since the column belongs to the column family with ID 0).

对于已整理的字符串，这意味着将数据编码为键和值，后者将显示在哨兵KV值中（当然，列属于ID为0的列族）。

Example schema and data:
```SQL
CREATE TABLE owners (
  owner STRING COLLATE en PRIMARY KEY
);

INSERT INTO owners VALUES
  ('Bob' COLLATE en),
  ('Ted' COLLATE en);
```

Example dump:

    /Table/51/1/"\x16\x05\x17q\x16\x05\x00\x00\x00 \x00 \x00 \x00\x00\b\x02\x02"/0/1489502864.477790157,0 : 0xDC5FDAE10A1603426F62
                ^---------------------------------------------------------------                                          ^-------
                Collation key for 'Bob'                                                                                   'Bob'

    /Table/51/1/"\x18\x16\x16L\x161\x00\x00\x00 \x00 \x00 \x00\x00\b\x02\x02"/0/1489502864.477790157,0 : 0x8B30B9290A1603546564
                ^------------------------------------------------------------                                          ^-------
                Collation key for 'Ted'                                                                                'Ted'

Indexes (secondary indexes)
---

To unify(统一) the handling of SQL tables and indexes, CRDB stores the authoritative table data in what is termed the primary index. 

统一SQL表和索引，CRDB在所谓一级索引中存储authoritative table数据

SQL indexes are secondary indexes. All indexes have an ID that is unique within their table.

SQL索引是二级索引，每个索引在他们的表中都有唯一的ID

The user-specified metadata for secondary indexes include a nonempty list of indexed columns, each with an ascending/descending designation, and a disjoint list of stored columns. 

二级索引中用户指定的metadata包括一个非空的列表，包含索引化的列，和一个不相交的列表包含存储的列。

The first list determines how the index is sorted, and columns from both lists can be read directly from the index.

第一个列表定义了索引如何排序，两个列表中的列都可以通过索引直接读取。

Users also specify whether a secondary index should be unique. 

用户也可以指定二级索引是否唯一

Unique secondary indexes constrain the table data not to have two rows where, for each indexed column, the data therein are non-null and equal.

唯一的二级索引约束表数据不包含这样的两行：对于每个索引列，其中的数据是非null的。

### Key encoding

The main encoding function for secondary indexes is `EncodeSecondaryIndex` in [pkg/sql/sqlbase/table.go]().

Each row gives rise to one KV pair per secondary index, whose KV key has fields mirroring the primary index encoding:

* The table ID
* The index ID
* Data from where the row intersects the indexed columns 行与索引列交叉的数据。
* If the index is non-unique or the row has a NULL in an indexed column, data from where the row intersects the non-indexed primary key (implicit) columns 如果索引不是唯一的，或者在索引列中有一个NULL，则该行的数据与非索引主键（隐式）列相交。
* If the index is non-unique or the row has a NULL in an indexed column, and the index uses the old format for stored columns, data from where the row intersects the stored columns如果索引不是唯一的，或者索引列中的行有null，索引使用存储列的旧格式，则行与存储列相交的数据。
* Zero (instead of the column family ID; all secondary KV pairs are sentinels).

Unique indexes relegate(归类) the data in extra columns to KV values so that the KV layer detects constraint violations. 

唯一索引额外列中的数据归类到value中，以便KV层检测约束冲突

The special case for an indexed NULL arises from the fact that NULL does not equal itself, hence rows with an indexed NULL cannot be involved in a violation. 

有索引的NULL的特殊情况：因为null本身不等于它本身，所以有索引的null的行不能被包含在一个违规中（？）。

They need a unique KV key nonetheless, as do rows in non-unique indexes, which is achieved by including the non-indexed primary key data. 

但是，它们需要一个唯一的kV键，就像在非唯一索引中的行一样，这是通过包含非索引主键数据来实现的。

For the sake of simplicity, data in stored columns are also included.

为了简单起见，存储列中的数据也包括在内。

### Value encoding

KV values for secondary indexes have value type BYTES and consist of:

* If the index is unique, data from where the row intersects the non-indexed primary key (implicit) columns, encoded as in the KV key
如果索引是唯一的，则该行中的数据与非索引主键（隐式）列相交，编码为key中的列。
* If the index is unique, and the index uses the old format for stored columns, data from where the row intersects the stored columns, encoded as in the KV key如果索引是唯一的，索引使用存储的列的旧格式，则从该行的数据与存储的列相交，编码为key中的数据。
* If needed, TUPLE-encoded bytes for non-null composite and stored column data (new format).

All of these fields are optional, so the BYTES value may be empty. 

Note that, in a unique index, rows with a NULL in an indexed column have their implicit column data stored in both the KV key and the KV value. (Ditto for stored column data in the old format.)注意，在一个惟一的索引中，索引列中带有null的行有隐式列数据存储在KV键和KV值中。（在旧格式存储列数据同上）

### Example dump

Example schema and data:
```SQL
CREATE TABLE accounts (
  id INT PRIMARY KEY,
  owner STRING,
  balance DECIMAL,
  UNIQUE INDEX i2 (owner) STORING (balance),
  INDEX i3 (owner) STORING (balance)
);

INSERT INTO accounts VALUES
  (1, 'Alice', 10000.50),
  (2, 'Bob', 25000.00),
  (3, 'Carol', NULL),
  (4, NULL, 9400.10),
  (5, NULL, NULL);
```

Index ID 1 is the primary index.

    /Table/51/1/1/0/1489504989.617188491,0 : 0x4AAC12300A2605416C6963651505348D0F4272
    /Table/51/1/2/0/1489504989.617188491,0 : 0x148941AD0A2603426F621505348D2625A0
    /Table/51/1/3/0/1489504989.617188491,0 : 0xB1D0B5390A26054361726F6C
    /Table/51/1/4/0/1489504989.617188491,0 : 0x247286F30A3505348C0E57EA
    /Table/51/1/5/0/1489504989.617188491,0 : 0xCB0644270A

#### Old STORING format

Index ID 2 is the unique secondary index `i2`.

    /Table/51/2/NULL/4/9400.1/0/1489504989.617188491,0 : 0x01CF9BB0038C2BBD011400
                ^--- ^ ^-----                                      ^-^-^---------
      Indexed column | Stored column                           BYTES 4 9400.1
                     Implicit column

    /Table/51/2/NULL/5/NULL/0/1489504989.617188491,0 : 0xE86B1271038D00
                ^--- ^ ^---                                      ^-^-^-
      Indexed column | Stored column                         BYTES 5 NULL
                     Implicit column

    /Table/51/2/"Alice"/0/1489504989.617188491,0 : 0x285AC6F303892C0301016400
                ^------                                      ^-^-^-----------
         Indexed column                                  BYTES 1 10000.5

    /Table/51/2/"Bob"/0/1489504989.617188491,0 : 0x23514F1F038A2C056400
                ^----                                      ^-^-^-------
       Indexed column                                  BYTES 2 2.5E+4

    /Table/51/2/"Carol"/0/1489504989.617188491,0 : 0xE98BFEE6038B00
                ^------                                      ^-^-^-
         Indexed column                                  BYTES 3 NULL

Index ID 3 is the non-unique secondary index `i3`.

    /Table/51/3/NULL/4/9400.1/0/1489504989.617188491,0 : 0xEEFAED0403
                ^--- ^ ^-----                                      ^-
      Indexed column | Stored column                           BYTES
                     Implicit column

    /Table/51/3/NULL/5/NULL/0/1489504989.617188491,0 : 0xBE090D2003
                ^--- ^ ^---                                      ^-
      Indexed column | Stored column                         BYTES
                     Implicit column

    /Table/51/3/"Alice"/1/10000.5/0/1489504989.617188491,0 : 0x7B4964C303
                ^------ ^ ^------                                      ^-
         Indexed column | Stored column                            BYTES
                        Implicit column

    /Table/51/3/"Bob"/2/2.5E+4/0/1489504989.617188491,0 : 0xDF24708303
                ^---- ^ ^-----                                      ^-
       Indexed column | Stored column                           BYTES
                      Implicit column

    /Table/51/3/"Carol"/3/NULL/0/1489504989.617188491,0 : 0x96CA34AD03
                ^------ ^ ^---                                      ^-
         Indexed column | Stored column                         BYTES
                        Implicit column

#### New STORING format

Index ID 2 is the unique secondary index `i2`.

    /Table/51/2/NULL/4/0/1492010940.897101344,0 : 0x7F2009CC038C3505348C0E57EA
                ^--- ^                                      ^-^-^-------------
      Indexed column Implicit column                    BYTES 4 9400.10

    /Table/51/2/NULL/5/0/1492010940.897101344,0 : 0x48047B1A038D
                ^--- ^                                      ^-^-
      Indexed column Implicit column                    BYTES 5

    /Table/51/2/"Alice"/0/1492010940.897101344,0 : 0x24090BCE03893505348D0F4272
                ^------                                      ^-^-^-------------
         Indexed column                                  BYTES 1 10000.50

    /Table/51/2/"Bob"/0/1492010940.897101344,0 : 0x54353EB9038A3505348D2625A0
                ^----                                      ^-^-^-------------
       Indexed column                                  BYTES 2 25000.00

    /Table/51/2/"Carol"/0/1492010940.897101344,0 : 0xE731A320038B
                ^------                                      ^-^-
         Indexed column                                  BYTES 3

Index ID 3 is the non-unique secondary index `i3`.

    /Table/51/3/NULL/4/0/1492010940.897101344,0 : 0x17C357B0033505348C0E57EA
                ^--- ^                                      ^-^-------------
      Indexed column Implicit column                    BYTES 9400.10

    /Table/51/3/NULL/5/0/1492010940.897101344,0 : 0x844708BC03
                ^--- ^                                      ^-
      Indexed column Implicit column                    BYTES

    /Table/51/3/"Alice"/1/0/1492010940.897101344,0 : 0x3AD2E728033505348D0F4272
                ^------ ^                                      ^-^-------------
         Indexed column Implicit column                    BYTES 10000.50

    /Table/51/3/"Bob"/2/0/1492010940.897101344,0 : 0x7F1225A4033505348D2625A0
                ^---- ^                                      ^-^-------------
       Indexed column Implicit column                    BYTES 25000.00

    /Table/51/3/"Carol"/3/0/1492010940.897101344,0 : 0x45C61B8403
                ^------ ^                                      ^-
         Indexed column Implicit column                    BYTES

### Composite encoding

Secondary indexes use key encoding for all indexed columns, implicit
columns, and stored columns in the old format. 

二级索引使用旧版本中所有索引列、隐式列和存储列的键编码。

Every datum whose key encoding does not suffice for decoding (collated strings, floating-point and decimal negative zero, decimals with trailing zeros) is encoded again, in the same `TUPLE` that contains stored column data in the new
format.

Example schema and data:
```SQL
    CREATE TABLE owners (
      id INT PRIMARY KEY,
      owner STRING COLLATE en,
      INDEX i2 (owner)
    );

    INSERT INTO owners VALUES
      (1, 'Ted' COLLATE en),
      (2, 'Bob' COLLATE en),
      (3, NULL);
```
Index ID 1 is the primary index.

    /Table/51/1/1/0/1492008659.730236666,0 : 0x6CA87E2B0A2603546564
    /Table/51/1/2/0/1492008659.730236666,0 : 0xE900EBB50A2603426F62
    /Table/51/1/3/0/1492008659.730236666,0 : 0xCF8B38950A

Index ID 2 is the secondary index `i2`.

    /Table/51/2/NULL/3/0/1492008659.730236666,0 : 0xBDAA5DBE03
                ^---                                        ^-
                Indexed column                          BYTES

    /Table/51/2/"\x16\x05\x17q\x16\x05\x00\x00\x00 \x00 \x00 \x00\x00\b\x02\x02"/2/0/1492008659.730236666,0 : 0x4A8239F6032603426F62
                ^---------------------------------------------------------------                                        ^-^---------
                Indexed column: Collation key for 'Bob'                                                             BYTES 'Bob'

    /Table/51/2/"\x18\x16\x16L\x161\x00\x00\x00 \x00 \x00 \x00\x00\b\x02\x02"/1/0/1492008659.730236666,0 : 0x747DA39A032603546564
                ^------------------------------------------------------------                                        ^-^---------
                Indexed column: Collation key for 'Ted'                                                          BYTES 'Ted'

Interleaving（交错）
------------

By default, indexes (in CRDB terminology, so both primary and secondary)
occupy disjoint KV key spans. 
默认情况下，索引占用不相交的KV键跨度。

Users can request that an index be interleaved with another index, which improves the efficiency of joining them.

用户可以要求索引与另一个索引进行交叉，从而提高了加入索引的效率。

One index, the parent, must have a primary key that, ignoring columnnames, is a prefix (not necessarily proper) of the other index, the child. 

一个索引（父）必须有一个主键，忽略列名，是另一个索引(子)的前缀（不一定是正确的）。

The parent, which currently must be a primary index, has its usual encoding.

父级，现在必须是主索引，它有通常的编码。 

To encode a KV key in the child, encode it as if it were in the parent but with an interleaving sentinel (`EncodeNotNullDescending` in [pkg/util/encoding/encoding.go]()) where the column family ID would be. 

Append the non-interleaved child encoding but without the parent columns. The sentinel informs the decoder that therow does not belong to the parent table.

Note that the parent may itself be interleaved. In general, theinterleaving relationships constitute an [arborescence].

Example schema and data:
```SQL
    CREATE TABLE owners (
      owner_id INT PRIMARY KEY,
      owner STRING
    );

    CREATE TABLE accounts (
      owner_id INT,
      account_id INT,
      balance DECIMAL,
      PRIMARY KEY (owner_id, account_id)
    ) INTERLEAVE IN PARENT owners (owner_id);

    INSERT INTO owners VALUES (19, 'Alice');
    INSERT INTO accounts VALUES (19, 83, 10000.50);
```
Example dump:

    /Table/51/1/19/0/1489433137.133889094,0 : 0xDBCE04550A2605416C696365
           ^- ^ ^- ^                            ^-------^-^^^-----------
           |  | |  |                            |       | |||
           Table ID (owners)                    Checksum| |||
              | |  |                                    | |||
              Index ID                                  Value type (TUPLE)
                |  |                                      |||
                Primary key (owner_id = 19)               Column ID difference
                   |                                       ||
                   Column family ID                        Datum encoding type (Bytes)
                                                            |
                                                            Datum encoding ('Alice')

    /Table/51/1/19/#/52/1/83/0/1489433137.137447008,0 : 0x691956790A3505348D0F4272
           ^- ^ ^- ^ ^- ^ ^- ^                            ^-------^-^^^-----------
           |  | |  | |  | |  |                            |       | |||
           Table ID (owners) |                            Checksum| |||
              | |  | |  | |  |                                    | |||
              Index ID  | |  |                                    Value type (TUPLE)
                |  | |  | |  |                                      |||
                Primary key (owner_id = 19)                         Column ID difference
                   | |  | |  |                                       ||
                   Interleaving sentinel                             Datum encoding type (Decimal)
                     |  | |  |                                        |
                     Table ID (accounts)                              Datum encoding (10000.50)
                        | |  |
                        Index ID
                          |  |
                          Primary key (account_id = 83)
                             |
                             Column family ID