# Protocol Buffer

Protocol buffers are a flexible, efficient, automated mechanism for **serializing structured data** – think XML, but smaller, faster, and simpler. 

1. define the structure of data (*.proto* file)
2. generate source code
3. use it

You can even update your data structure without breaking deployed programs that are compiled against the "old" format.

## Example of *.proto* file

Each protocol buffer message is a small logical record of information, containing a series of name-value pairs. 

Here's a very basic example of a .proto file that defines a message containing information about a person:

    message Person {
    required string name = 1;
    required int32 id = 2;
    optional string email = 3;

    enum PhoneType {
        MOBILE = 0;
        HOME = 1;
        WORK = 2;
    }

    message PhoneNumber {
        required string number = 1;
        optional PhoneType type = 2 [default = HOME];
    }

    repeated PhoneNumber phone = 4;
    }

 Each message type has one or more uniquely numbered fields.
 
 Each field has a name and a value type, where value types can be ***numbers (integer or floating-point), booleans, strings, raw bytes, or even (as in the example above) other protocol buffer message types***, allowing you to structure your data hierarchically. 
 
 You can specify optional fields, required fields, and repeated fields. 

 ## Example of generated source code

 Once you've defined your messages, you run the ***protocol buffer compiler*** for your application's language on your .proto file to generate ***data access classes***. 
 
 These provide simple accessors for each field ***(like name() and set_name())*** as well as methods to serialize/parse the whole structure to/from raw bytes.
 
 So, for instance, if your chosen language is C++, running the compiler on the above example will generate a class called Person. You can then use this class in your application to populate, serialize, and retrieve Person protocol buffer messages. 
 
 You might then write some code like this:

    Person person;
    person.set_name("John Doe");
    person.set_id(1234);
    person.set_email("jdoe@example.com");
    fstream output("myfile", ios::out | ios::binary);
    person.SerializeToOstream(&output);

Then, later on, you could read your message back in:

You can add new fields to your message formats without breaking backwards-compatibility (向后兼容性).

Old binaries simply ignore the new field when parsing. So if you have a communications protocol that uses protocol buffers as its data format, you can extend your protocol without having to worry about breaking existing code.

## Why not just use XML?

Protocol buffers have many advantages over XML for serializing structured data.

* are simpler
* are 3 to 10 times smaller
* are 20 to 100 times faster
* are less ambiguous(含糊)
* generate data access classes that are easier to use programmatically

## Get started

https://github.com/golang/protobuf

Download source code and build it

## Protocol Buffer Basics: Go

* Define message formats in a .proto file.
* Use the protocol buffer compiler.
* Use the Go protocol buffer API to write and read messages.

#### Example code  &nbsp; &nbsp; &nbsp; addressbook.proto

https://github.com/google/protobuf/tree/master/examples

    syntax = "proto3";
    package tutorial;

    message Person {
        string name = 1;
        int32 id = 2;  // Unique ID number for this person.
        string email = 3;

        enum PhoneType {
            MOBILE = 0;
            HOME = 1;
            WORK = 2;
        }

        message PhoneNumber {
            string number = 1;
            PhoneType type = 2;
        }

        repeated PhoneNumber phones = 4;
    }

    // Our address book file is just one of these.
    message AddressBook {
        repeated Person people = 1;
    }

In the above example:

The **Person** message contains **PhoneNumber** messages.

The **AddressBook** message contains **Person** messages. 

* You can even define message types nested inside other messages – as you can see, the **PhoneNumber** type is defined inside **Person**.

* You can also define ***enum*** types if you want one of your fields to have one of a ***predefined*** list of values – here you want to specify that a phone number can be one of ***MOBILE, HOME,*** or ***WORK***.

* The **" = 1", " = 2"** markers on each element identify the unique **"tag"** that field uses in the binary encoding. 

* Tag numbers **1-15** require one less byte to encode than higher numbers, so as an ***optimization*** you can decide to use those tags for the ***commonly used or repeated elements***.

* Leaving tags **16** and higher for ***less-commonly used optional elements***. 

* Each element in a **repeated** field requires **re-encoding** the tag number, so repeated fields are particularly good candidates for this optimization.

* If a field value isn't set, a ***default value*** is used: 

    * zero for numeric types, 
    * the empty string for strings, 
    * false for bools. 
    * For embedded messages, the default value is always the **"default instance"** or **"prototype"** of the message, which has none of its fields set. 
    * Calling the accessor to get the value of a field which has not been explicitly set always returns that field's default value.

* If a field is repeated, the field may be repeated any number of times (including zero). The order of the repeated values will be preserved in the protocol buffer. Think of repeated fields as dynamically sized arrays.(不是很懂)

https://developers.google.com/protocol-buffers/docs/proto3


## Compiling your protocol buffers

Now that you have a **.proto**, the next thing you need to do is generate the classes you'll need to read and write AddressBook (and hence Person and PhoneNumber) messages. To do this, you need to run the protocol buffer compiler **protoc** on your **.proto**:

1. install the compiler https://developers.google.com/protocol-buffers/docs/downloads

2. install the Go protocol buffers plugin:

    go get -u github.com/golang/protobuf/protoc-gen-go
    
    The compiler plugin protoc-gen-go will be installed in $GOBIN, defaulting to $GOPATH/bin. It must be in your $PATH for the protocol compiler protoc to find it.

3. Now run the compiler, 

    * specifying the source directory (where your application's source code lives – the current directory is used if you don't provide a value)
    * the destination directory (where you want the generated code to go; often the same as $SRC_DIR)
    * the path to your .proto. 
    
    In this case, you...
    
        protoc -I=$SRC_DIR --go_out=$DST_DIR $SRC_DIR/addressbook.proto
    
    Because you want Go classes, you use the --go_out option – similar options are provided for other supported languages.

This generates **addressbook.pb.go** in your specified destination directory.

## The Protocol Buffer API

Generating **addressbook.pb.go** gives you the following useful types:

* An **AddressBook** structure with a **People** field.
* A **Person** structure with fields for **Name, Id, Email** and **Phones**.
* A **Person_PhoneNumber** structure, with fields for **Number** and **Type**.
* The type **Person_PhoneType** and a value defined for each value in the **Person.PhoneType** enum.

You can read more about the details of exactly what's generated in the ***Go Generated Code guide***(https://developers.google.com/protocol-buffers/docs/reference/go-generated), but for the most part you can treat these as perfectly ordinary Go types.

Here's an example of how you might create an instance of **Person**:

    p := pb.Person{
            Id:    1234,
            Name:  "John Doe",
            Email: "jdoe@example.com",
            Phones: []*pb.Person_PhoneNumber{
                    {Number: "555-4321", Type: pb.Person_HOME},
            },
    }

## Writing a Message

The whole purpose of using protocol buffers is to serialize your data so that it can be parsed elsewhere. 

In Go, you use the proto library's **Marshal** function to serialize your protocol buffer data. 

A pointer to a protocol buffer message's struct implements the proto.Message interface. 

Calling proto.Marshal returns the protocol buffer, encoded in its wire format. 

For example:

    book := &pb.AddressBook{}
    // ...

    // Write the new address book back to disk.
    out, err := proto.Marshal(book)
    if err != nil {
            log.Fatalln("Failed to encode address book:", err)
    }
    if err := ioutil.WriteFile(fname, out, 0644); err != nil {
            log.Fatalln("Failed to write address book:", err)
    }

## Reading a Message

To parse an encoded message, you use the proto library's **Unmarshal** function. Calling this parses the data in buf as a protocol buffer and places the result in pb. 

    // Read the existing address book.
    in, err := ioutil.ReadFile(fname)
    if err != nil {
            log.Fatalln("Error reading file:", err)
    }
    book := &pb.AddressBook{}
    if err := proto.Unmarshal(in, book); err != nil {
            log.Fatalln("Failed to parse address book:", err)
    }

##  Extending a Protocol Buffer

Sooner or later after you release the code that uses your protocol buffer, you will undoubtedly want to "improve" the protocol buffer's definition. 

If you want your new buffers to be backwards-compatible, and your old buffers to be forward-compatible – and you almost certainly do want this – then there are some rules you need to follow. 

In the new version of the protocol buffer:

* you must not change the tag numbers of any existing fields.
* you may delete fields.
* you may add new fields but you must use fresh tag numbers (i.e. tag numbers that were never used in this protocol buffer, not even by deleted fields).

If you follow these rules, old code will happily read new messages and simply ignore any new fields. 

To the old code, singular fields that were deleted will simply have their default value, and deleted repeated fields will be empty. New code will also transparently read old messages.

However, keep in mind that ***new fields will not be present in old messages***, so you will need to do something reasonable with the default value. 