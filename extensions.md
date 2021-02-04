# Extensions



## SQL Support

Branch `sql` contains the work-in-progress implementation of adding SQL support to WeakIsolationMockDB. We use sql-parser [[Github]](https://github.com/hyrise/sql-parser) library for parsing SQL queries.

#### **Implementation**

SQL-store is implemented on top of kv-store (kv-store/include/sql-store.h). Apart from column wise data, there is metadata stored for each table. In particular, the following metadata in key-value form is stored:

* "*table_name*:columns", "col1,col2,col3,...."
* "*table_name*:primary_keys", "1,2,3,...."



Below examples show the steps involved in executing different queries:

**SELECT * FROM table;**

 - Read "table:columns"
 - Read "table:primary_keys"
 - Form column keys:
    - Assuming col1 is the primary key with value 1, 2...
    - ["table:1:col2", "table:1:col3", "table:2:col2", "table:2:col3", ...] 
 - Read column keys



**SELECT c1, c2, ... cn from table where predicate;**

 - Read "table:columns"

 - Read "table:primary_keys"

 - Form column keys from given columns, ["table:1:col2", "table:1:col3"]

 - Read column keys

 - Filter rows based on predicate

   *Note that this implementation is inefficient as it reads the whole table irrespective of predicate.*



**CREATE TABLE table (column1 type, column2 type, column3 type, .....)**

 - Write column metadata
    - write ("table:columns", "col1,col2,col3, ...")



**INSERT INTO table values (val1, val2)**

 - Append to list of primary keys
	- read ("table:primary_keys")
	- write ("table:primary_keys")
 - Write individual column
    - write("table:1:col1", val1), write("table:1:col2", val2), ...

