#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>
#include <atomic>
#include <unistd.h>
using namespace std;


// Cria classe de Spin Lock, para implementar a exclusao mutua com busy wait

class SpinLock {
public:
    SpinLock() {
        flag_.clear();
    }

    void acquire() {
        while (flag_.test_and_set(std::memory_order_acquire));
    }

    void release() {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_;
};


// Função que soma os inteiros de uma parte de um array

void somarParte(vector<char> vetor, int inicio, int fim, atomic<int>& resultado, SpinLock& lock) {

    int soma = 0;
    int check_nice = nice(-20);

    for (int i = inicio; i <= fim; i++) {
        soma += static_cast<int>(vetor[i]);
    }

    lock.acquire();
    resultado += soma;
    lock.release();
}


int main(int argc, char *argv[]) {

    // Define variaveis
    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    double total_time = 0.0;

    // 10 iteracoees para pegar tempo medio de execucao
    for (int i = 0; i < 10; i++) {

        // Define lock e vetor
        SpinLock lock;
        std::vector<char> vetor(n);

        // Gera uma semente aleatória para a função rand()
        srand(time(nullptr));

        // Preenche o vetor com valores aleatórios entre -100 e 100
        for (int i = 0; i < n; i++) {
            vetor[i] = static_cast<char>(rand() % 201 - 100); // gera um valor entre -100 e 100
        }

        // Divide o vetor em K partes e cria uma thread para cada parte, chamando a funcao somarParte
        int elementosPorParte = n / k;
        int resto = n % k;
        int inicio = 0;
        int fim = elementosPorParte - 1;
        vector<thread> threads;
        atomic<int> resultadoTotal(0);

        // Rever essa parte
        auto start_time = chrono::high_resolution_clock::now();
        for (int i = 0; i < k; i++) {
            if (i < resto) {
                fim++;
            }
            threads.push_back(thread(somarParte, vetor, inicio, fim, ref(resultadoTotal), ref(lock)));
            inicio = fim + 1;
            fim = inicio + elementosPorParte - 1;
        }
        
        // Aguarda todas as threads terminarem
        for (auto& t : threads) {
            t.join();
        }
        auto end_time = chrono::high_resolution_clock::now();
        double elapsed_time = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() / 1000.0;
        total_time += elapsed_time;

        // Soma os elementos do vetor em apenas uma thread e imprime o resultado (para validar o resultado anterior)
        atomic<int> resultado_single_thread(0);
        somarParte(vetor, 0, n-1, resultado_single_thread, lock);

        // Testa corretude de soma
        if (resultado_single_thread != resultadoTotal) {
            cout << "Single-threaded and multi-threaded results are different!: " << endl;
            cout << "Single-threaded :" << resultado_single_thread << endl;
            cout << "Multi-threaded :" << resultadoTotal << endl;
        }

        sleep(0.1);
    }
    
    // Imprime tempo medio de execucao
    cout << "Mean runtime: " << total_time/10 << endl;
    
    return 0;
}
