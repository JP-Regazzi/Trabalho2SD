#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <semaphore.h>

// Shared data
std::vector<int> sharedVector;
int N;

// Semaphores
sem_t emptySlots;
sem_t filledSlots;

void consumer(int id)
{
    for (int i = 0; i < N; i++)
    {
        sem_wait(&filledSlots); // Wait for a filled slot
        std::cout << "Consumer " << id << " consumed: " << sharedVector[i] << std::endl;
        sharedVector[i] = 0; // Free memory by setting the consumed slot to 0
        sem_post(&emptySlots); // Signal an empty slot

        // Sleep for a random time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " N" << std::endl;
        return 1;
    }

    N = std::stoi(argv[1]);
    sharedVector.resize(N);

    sem_init(&emptySlots, 0, N); // Initialize emptySlots semaphore to N
    sem_init(&filledSlots, 0, 0); // Initialize filledSlots semaphore to 0

    int nc = 2; // Number of consumer threads (can be changed)
    std::vector<std::thread> consumers;

    for (int i = 0; i < nc; i++)
        consumers.emplace_back(consumer, i);

    for (auto &thread : consumers)
        thread.join();

    sem_destroy(&emptySlots);
    sem_destroy(&filledSlots);

    return 0;
}
