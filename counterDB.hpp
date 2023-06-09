#include <cstddef>
#include <iostream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <mutex>

#ifndef COUNTER_DB
#define COUNTER_DB

namespace counterDB {
    class CounterDB
    {
    public:
        CounterDB(std::string dbFilePath, unsigned long long maxExpectedIndex);
        ~CounterDB();

        /**
        Should be called only once, to issue and index to use counter for.
        Throws runtime_error if maxExpectedIndex have been issued already
        */
        unsigned long long issueAnIndex(){
            mtx.lock();
            auto t = data[0]+1;
            if(t>maxExpectedIndex){
                throw std::runtime_error("Cannot create new indices beyond maxExpectedIndex");
            }

            std::memcpy(data,&t,sizeof(unsigned long long));
            mtx.unlock();
            return t;
        }

        /*
        Number of indices issued so far.
        */
        unsigned long long maxIndexIssued(){
            return data[0];
        }

        
        /*
        Increment the value at the given index.
        Note: use only those index which you have issued once, in order to use it durably
        */
        unsigned long long increment(unsigned long long index, unsigned long long increment=1){
            if(index==0){
                throw std::runtime_error("Cannot modify zero index, its used to manage ther index");
            }
            if(index > maxExpectedIndex){
                throw std::runtime_error("Index can't be greater than maxExpectedIndex");
            }
            mtx.lock();
            auto t = data[index] + increment;
            std::memcpy(&data[index],&t,sizeof(unsigned long long));
            mtx.unlock();
            return t;
        }


        /**
        Reads value at the given index.
        throws runtime_error if the index is greater than maxExpectedIndex
        */
        unsigned long long read(unsigned long long index){
            if(index > maxExpectedIndex){
                throw std::runtime_error("Index can't be greater than maxExpectedIndex");
            }
            return data[index];
        }

        /*
        Sync data to underlying database. Though its done automaticaly. But calling it appropriate time can give strong durabilities. 
        Don't call it too frequent else it can impact performance.
        */
        void sync(){
            // Sync the memory-mapped file to disk
            if (msync(data, MAX_SIZE, MS_SYNC) == -1) {
                perror("Could not sync the file to disk");
            }
        }
        
    
    private:
        std::string dbFilePath;
        int fd;
        size_t MAX_SIZE=-1;
        unsigned long long* data=NULL;
        std::mutex mtx;
        unsigned long long maxExpectedIndex=-1;
    };

    inline CounterDB::CounterDB(std::string dbFilePath, unsigned long long maxExpectedIndex):maxExpectedIndex(maxExpectedIndex),dbFilePath(dbFilePath)
    {
        // Open the file
        fd = open(&dbFilePath[0], O_RDWR | O_CREAT, (mode_t)0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        // To use a file as memory file we need to extends its size first to the 
        //maximum size of this database
        MAX_SIZE = maxExpectedIndex * sizeof(unsigned long long);
        {
            //1. SO here we seek file to almost last position
            off_t offset = lseek(fd, MAX_SIZE - 1, SEEK_SET);
            if (offset == -1) {
                close(fd);
                perror("Error calling lseek() to 'stretch' the file");
                exit(EXIT_FAILURE);
            }

            //2. and then we write just a byte : which will make the file of that size
            ssize_t result = write(fd, "", 1);
            if (result != 1) {
                close(fd);
                perror("Error writing last byte of the file");
                exit(EXIT_FAILURE);
            }
        }

        // Map the file to memory
        data = static_cast<unsigned long long*>(mmap(0, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (data == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }

    };

    inline CounterDB::~CounterDB()
    {
        // Unmap the file from memory
        if (munmap(data, MAX_SIZE) == -1) {
            perror("Error un-mmapping the file");
        }
        // Close the file
        close(fd);
    };
}

#endif // !COUNTER_DB