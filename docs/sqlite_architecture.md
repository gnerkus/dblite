# Notes

### Questions about DBs
What format is data saved in? (in memory and on disk)
When does it move from memory to disk?
Why can there only be one primary key per table?
How does rolling back a transaction work?
How are indexes formatted?
When and how does a full table scan happen?
What format is a prepared statement saved in?

## SQLite internals
Tokenizer -> Parser -> Code Generator -> Virtual Machine -> B-Tree -> Pager -> OS Interface

### SQLite architecture: https://www.sqlite.org/arch.html
SQLite works by compiling SQL text into bytecode, then running the bytecode using a virtual machine.

`sqlite3_prepare_v2()` acts as a compiler for converting SQL text into bytecode.
`sqlite3_stmt` container for single bytecode program that implements a single sQL statement
`sqlite3_step()` interface passes a bytecode program into the virtual machine, and runs the program until it either completes, or forms a row of result to be returned, or hits a fatal error, or is interrupted.

### Architecture diagram
*SQL Compiler*: `tokenizer` -> `Parser` -> `Code Generator` -> SQL Command processor
*Core*: `Interface` -> `SQL Command Processor` -> `Virtual Machine` -> B-Tree
                       `SQL Command Processor` -> Tokenizer
 *Backend*: `B-Tree` -> `Pager` -> `OS Interface`
 *Accessories*: `Utilities`, `Test Code`

 A query goes through a chain of components in order to retrieve or modify data. The front-end consists of the:

tokenizer
parser
code generator
The input to the front-end is a SQL query. the output is sqlite virtual machine bytecode (essentially a compiled program that can operate on the database).

The back-end consists of the:

virtual machine
B-tree
pager
os interface
The virtual machine takes bytecode generated by the front-end as instructions. It can then perform operations on one or more tables or indexes, each of which is stored in a data structure called a B-tree. The VM is essentially a big switch statement on the type of bytecode instruction.

Each B-tree consists of many nodes. Each node is one page in length. The B-tree can retrieve a page from disk or save it back to disk by issuing commands to the pager.

The pager receives commands to read or write pages of data. It is responsible for reading/writing at appropriate offsets in the database file. It also keeps a cache of recently-accessed pages in memory, and determines when those pages need to be written back to disk.

The os interface is the layer that differs depending on which operating system sqlite was compiled for. In this tutorial, I???m not going to support multiple platforms.

