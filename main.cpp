include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

std::mutex fileMutex;
size_t lastFileSize = 0;  // Track last known size of the file

// Custom memory allocation tracking
void* operator new(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    std::cout << "[Memory Alloc] " << size << " bytes at " << ptr
              << " (File: " << file << ", Line: " << line << ")\n";
    return ptr;
}

void operator delete(void* ptr) noexcept {
    std::cout << "[Memory Free] at " << ptr << "\n";
    free(ptr);
}

// Overloaded `operator new[]` for array allocations
void* operator new[](size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    std::cout << "[Memory Alloc (Array)] " << size << " bytes at " << ptr
              << " (File: " << file << ", Line: " << line << ")\n";
    return ptr;
}

void operator delete[](void* ptr) noexcept {
    std::cout << "[Memory Free (Array)] at " << ptr << "\n";
    free(ptr);
}

// Macro for tracking memory allocation
#define NEW new(__FILE__, __LINE__)

// Function to monitor a specific file
void monitorFile(const std::string& filePath) {
    while (true) {
        std::lock_guard<std::mutex> lock(fileMutex);
        
        if (fs::exists(filePath)) {
            size_t fileSize = fs::file_size(filePath);
            
            if (fileSize != lastFileSize) {
                std::cout << "[File Modified] " << filePath 
                          << " | Size: " << fileSize << " bytes\n";
                lastFileSize = fileSize;
            }
        } else {
            std::cout << "[File Deleted] " << filePath << "\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_to_monitor>\n";
        return 1;
    }

    std::string filePath = argv[1];

    if (!fs::exists(filePath)) {
        std::cerr << "[Error] File does not exist: " << filePath << "\n";
        return 1;
    }

    std::cout << "[Monitoring] " << filePath << "\n";

    // Start monitoring in a separate thread
    std::thread fileMonitorThread(monitorFile, filePath);
    fileMonitorThread.detach();

    // Simulate memory allocation tracking
    int numElements = 10;
    int* arr = NEW int[numElements];  // ✅ Allocating memory with tracking

    // Simulate writing to the file
    std::ofstream file(filePath, std::ios::app);
    file << "New data added to file.\n";
    file.close();

    std::cout << "[Data Written] " << filePath << "\n";

    // Free allocated memory
    delete[] arr;  // ✅ Properly deallocate array memory

    return 0;
}
