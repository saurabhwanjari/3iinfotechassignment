#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>

using namespace std;

std::mutex mtx;
std::condition_variable cv; // Conditional variable for synchronization
bool ready = false; // Flag indicating readiness for thread execution

// Function to read data from file
void readFromFile(const string& filename, int id) {
    // Wait until signaled to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return ready; }); // Wait until ready flag is true
    }

    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            mtx.lock(); // Lock before printing
            cout << "Thread " << id << " read data: " << line << endl;
            mtx.unlock(); // Unlock after printing
        }
        file.close();
    } else {
        cout << "Unable to open file: " << filename << endl;
    }
}

// Simulate data processing function
void processData(int id, const string& filename) {
    // Wait until signaled to start
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return ready; }); // Wait until ready flag is true
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate processing time
        int data = distrib(gen);
        
        mtx.lock(); // Lock before accessing file
        ofstream file(filename, ios::app);
        if (file.is_open()) {
            file << data << endl;
            file.close();
        } else {
            cout << "Unable to open file: " << filename << endl;
        }
        mtx.unlock(); // Unlock after accessing file
    }
}

int main() {
    const vector<string> inputFiles = {"input1.txt", "input2.txt"};

    // Create threads to read from input files
    vector<thread> threads; // Creating a vector of threads
    for (size_t i = 0; i < inputFiles.size(); ++i) {
        // Create reader thread
        threads.emplace_back(readFromFile, inputFiles[i], i + 1);
        // Create worker thread
        threads.emplace_back(processData, i + 1, inputFiles[i]);
    }
3i Infotech
    // Signal all threads to start
    {
        std::lock_guard<std::mutex> lock(mtx); // Lock the mutex
        ready = true; // Set the ready flag to true
        cv.notify_all(); // Notify all threads waiting on the conditional variable
    }

    // Join all threads with the main thread
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
