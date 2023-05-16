#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <semaphore.h>

// Shared data
std::vector<int> sharedVector;
int N;

// Semaphores
sem_t emptySlots;
sem_t filledSlots;

void producer(int id)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000000);

    for (int i = 0; i < N; i++)
    {
        sem_wait(&emptySlots); // Wait for an empty slot
        sharedVector[i] = dis(gen); // Insert random integer
        std::cout << "Producer " << id << " produced: " << sharedVector[i] << std::endl;
        sem_post(&filledSlots); // Signal a filled slot

        // Sleep for a random time
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
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

    int np = 3; // Number of producer threads (can be changed)
    std::vector<std::thread> producers;

    for (int i = 0; i < np; i++)
        producers.emplace_back(producer, i);

    for (auto &thread : producers)
        thread.join();

    sem_destroy(&emptySlots);
    sem_destroy(&filledSlots);

    return 0;
}
