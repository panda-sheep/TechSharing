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

#### Example code

