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

#ifndef VDB

namespace vDB {
    class VDB
    {
    public:
        VDB(std::string dbFilePath, unsigned long maxExpectedIndex);
        ~VDB();

        void update(unsigned long index, unsigned long value){
            mtx.lock();
            // Write data to the file
            std::memcpy(&data[index],&value,sizeof(unsigned long));
            mtx.unlock();
        }

        unsigned long read(unsigned long index){
            if(index > maxExpectedIndex){
                throw std::runtime_error("Index can't be greater than maxExpectedIndex");
            }
            return data[index];
        }

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
        unsigned long* data=NULL;
        std::mutex mtx;
        unsigned long maxExpectedIndex=-1;
    };

    inline VDB::VDB(std::string dbFilePath, unsigned long maxExpectedIndex):maxExpectedIndex(maxExpectedIndex),dbFilePath(dbFilePath)
    {
        // Open the file
        fd = open(&dbFilePath[0], O_RDWR | O_CREAT, (mode_t)0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        // To use a file as memory file we need to extends its size first to the 
        //maximum size of this database
        MAX_SIZE = maxExpectedIndex * sizeof(unsigned long);
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
        data = static_cast<unsigned long*>(mmap(0, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (data == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }

    };

    inline VDB::~VDB()
    {
        // Unmap the file from memory
        if (munmap(data, MAX_SIZE) == -1) {
            perror("Error un-mmapping the file");
        }
        // Close the file
        close(fd);
    };
}

#endif // !VDB
