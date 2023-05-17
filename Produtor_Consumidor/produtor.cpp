#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <semaphore.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

int Np, Nc, N;
int buffer[1000];
int bufferIndex = 0;
int totalConsumed = 0;
int maxConsumed = 100000;
int bufferHistory[1000000];
int historyIndex = 0;


// Semáforos
sem_t empty;
sem_t full;
sem_t mutex;

// Função que descobre se o inteiro é primo
bool isPrime(int n) {
    if (n <= 1) {
        return false;
    }
    for (int i = 2; i*i <= n; i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}


// Porção de código executada pelos produtores
void* producer(void* arg) {
    int id = *(int*)arg;
    delete (int*)arg;

    // Inicialização para gerador de números aleatórios
    std::mt19937 eng(static_cast<unsigned int>(std::time(nullptr)) + id);
    std::uniform_int_distribution<> distr(1, 10000000);

    while(totalConsumed < maxConsumed) {

        int randomNumber = distr(eng); // Produz um inteiro aleatório entre 1 e 10000000
        sem_wait(&empty); // Wait no semáforo empty (esperando um espaço vazio no buffer)
        sem_wait(&mutex); // Wait no semáforo mutex (esperando o buffer não estar sendo acessado por outras threads)

        // Verificação e liberação de threads presas no final da execução
        if (bufferIndex > (sizeof(buffer)/sizeof(buffer[0]))){
            sem_post(&full);
            sem_post(&empty);
            sem_post(&mutex);
            pthread_exit(nullptr);
            return(0);
        }
        // Adiciona o número aleatório ao buffer e registra ocupação do buffer no histórico
        buffer[bufferIndex] = randomNumber;
        bufferIndex++;
        bufferHistory[historyIndex] = bufferIndex;
        historyIndex++;

        sem_post(&mutex); // Signal no semáforo mutex (sinalizando que parou de usar as variáveis compartilhadas)
        sem_post(&full); // signal no semáforo full (indicando que um espaço do buffer foi preenchido)
    }

    // Termina a execução e dá signal nos semáforos para liberar possíveis threads presas
    printf("O PRODUTOR %d SE MATOU!!!\n", id);
    sem_post(&full);
    sem_post(&empty);
    sem_post(&mutex);
    pthread_exit(nullptr);
    return(0);
}

// Porção de código executada pelos consumidores
void* consumer(void* arg) {
    int id = *(int*)arg;
    delete (int*)arg;

    while(totalConsumed < maxConsumed) {  

        sem_wait(&full); // Wait no semáforo full (esperando um espaço ocupado no buffer)
        sem_wait(&mutex); // Wait no semáforo mutex (esperando o buffer não estar sendo acessado por outras threads)

        // Retira elemento do buffer, aumenta o contador de itens consumidos e registra ocupação do buffer no histórico
        int temp = buffer[bufferIndex-1];
        buffer[bufferIndex-1] = 0;
        bufferIndex--;
        totalConsumed++;
        bufferHistory[historyIndex] = bufferIndex;
        historyIndex++;

        sem_post(&mutex); // Signal no semáforo mutex (sinalizando que parou de usar as variáveis compartilhadas)
        sem_post(&empty); // signal no semáforo empty (indicando que um espaço do buffer foi esvaziado)

        // Verifica se o número retirado é primo ou não
        if (isPrime(temp)) { 
            printf("%d eh primo\n",temp);
        } else {
            printf("%d nao eh primo\n",temp);
        }
    }
    // Termina a execução e dá signal nos semáforos para liberar possíveis threads presas
    printf("O CONSUMIDOR %d SE MATOU!!!\n", id);
    sem_post(&full);
    sem_post(&empty);
    sem_post(&mutex); 
    pthread_exit(nullptr);
    return(0);
}

int main(int argc, char *argv[]) {
    // Verificação dos argumentos de execução
    if (argc != 4) {
        std::cout << "Uso correto: " << argv[0] << " Np Nc N" << std::endl;
        return 1;
    }

    // Obtenção dos parâmetros através dos argumentos de execução
    Np = std::stoi(argv[1]);
    Nc = std::stoi(argv[2]);
    N = std::stoi(argv[3]);

    // Inicialização dos semáforos
    sem_init(&empty, 0, N);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    
    // Vetores com as threads
    pthread_t producers[Np];
    pthread_t consumers[Nc];

    // Inicia o timer
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    // Cria os produtores
    for (int i = 0; i < Np; ++i) {
        int* id = new int(i);
        pthread_create(&producers[i], nullptr, producer, (void*)id);
    }

    // Cria os consumidores
    for (int i = 0; i < Nc; ++i) {
        int* id = new int(i);
        pthread_create(&consumers[i], nullptr, consumer, (void*)id);
    }

    // Espera os produtores terminarem de executar
    for (int i = 0; i < Np; ++i) {
        pthread_join(producers[i], nullptr);
    }
    
    // Espera os consumidores terminarem de executar
    for (int i = 0; i < Nc; ++i) {
        pthread_join(consumers[i], nullptr);
    }

    // Para o timer e calcula o tempo de execução
    clock_gettime(CLOCK_REALTIME, &end);
    double time_taken;
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    printf("Tempo de execução: %f\n", time_taken);
    
    // Destrói os semáforos usados
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    // Escreve em um arquivo txt o histórico de ocupação do buffer
    std::ofstream outputFile("array_elements.txt");
    if (outputFile.is_open()) {
        for (int i = 0; i < sizeof(bufferHistory)/sizeof(bufferHistory[0]); ++i) {
            outputFile << bufferHistory[i] << "\n";
        }
        outputFile.close();
        std::cout << "Histórico registrados." << std::endl;
    } else {
        std::cout << "Não foi possível abrir o arquivo de histórico." << std::endl;
    }

    std::ofstream outputTimeFile("times.txt", std::ios::app);
    if (outputTimeFile.is_open()) {
        outputTimeFile << time_taken << "\n";
        outputTimeFile.close();
        std::cout << "Tempo registrado." << std::endl;
    } else {
        std::cout << "Não foi possível abrir o arquivo de tempo." << std::endl;
    }

    return 0;
}
