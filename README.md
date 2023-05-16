# vDB
If you wanted to manage and persist unsigned long numbers, **say likes/dislikes/views count**, than it just does that.

To read/create counts you use index, which are unsigned long numbers. Consider it as an array of unsigned long numbers. The data though is persisted.
Any number of threads can read any index. Only one thread can write an index at a time. The rest will wait for there turn.

But why ?
* To Store likes , dislikes or View counts.
* To store any unsigned long number for any purpose, and retrive it with an index.

Usage example:

```c++
#include "vDB/vDB.hpp"

using namespace vDB;
    
int main(int argc, const char** argv) {
    //create DB
    auto db = new VDB("./db/test1.db",10);
    
    //insert value at index 1, and index 4
    db->update(1, 200000);
    db->update(4, 3400);
    
    
    //read the value after insertion
    std::cout << db->read(1) <<" "<< db->read(4) << std::endl;
    
    //read again
    auto t = db->read(4);
    //update the value: this will change on every restart
    db->update(4, t+400);
    
  
    std::cout << db->read(4) << std::endl;
    
    //by default value at an index is 0
    std::cout << db->read(9) << std::endl;
    
    //deleting db instance will not delet ethe file, it only closes the db fie properly
    delete db;
    return 0;
}
```


