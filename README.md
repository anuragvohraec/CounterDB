# vDB

A Solution to durably store your likes/dislikes count (or any count of your choice).

Its takes O(1) time to read and put unsigned long values to DB, as it similar to Arrays (there are no keys but you use index to store counts).

Single header only C++ file for storing and retreiving numbers.

And one more thing it does this without increasing the file size, as its not append only DB. Which also means it can be slower on simultenous writes. Well if you task is just to store likes/dislikes counts, than its muts not be much of a problems. As reads are done more than writes for such cases.

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


