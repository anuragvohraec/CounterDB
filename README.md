# CounterDB

A Solution to durably store your counts. For example number of likes/dislikes on a post.

Its time complexity is O(1). 

Single header only C++ file for storing and retrieving numbers.

And one more thing it does this without increasing the file size, as its not append only DB. Which also means it can be slower on simultaneous writes to an index. Well if your task is just to store likes/dislikes counts, than it must not be much of a problem. As reads are done more than writes for such cases.

Usage example:

```c++
#include "vDB/vDB.hpp"

using namespace vDB;
    
int main(int argc, const char** argv) {
    //create DB file : and maximum number of counter supported
    auto db = new VDB("./db/test1.db",10);

    //To check how many index
    auto c = db->maxIndexIssued();
    unsigned long t = 1;
    if(c==0){
        t = db->issueAnIndex();
    }
    
    db->increment(t);

    std::cout << "Count: " << db->read(t)<< std::endl;

    //this should not be called frequently, but its should be DO called if you want stronger durability.
    db->sync();

    delete db;
    return 0;
}
```


# Notes:
1. To get maximum speed: Use a cache layer and pass bigger increments in one go using `unsigned long increment(unsigned long index, unsigned long increment=1)`.
2. An index can count upto maximum value of `unsigned long`. So if you wanted higher counts than that, than you must use more than one index and sum them later using some custom logic.