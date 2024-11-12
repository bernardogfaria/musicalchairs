#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <semaphore.h>
#include <random>
#include <algorithm>

std::vector<int> jogadores;
std::vector<int> eliminados;
std::mutex mutex;
sem_t semaforo;

// Função para inicializar jogadores
void inicializar_jogadores(int n) {
    jogadores.clear();
    for (int i = 1; i <= n; ++i) {
        jogadores.push_back(i);
    }
}

// Função que simula o tempo de tentativa para ocupar uma cadeira
void tentar_ocupar_cadeira(int id) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(0, 1000);

    std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));

    if (sem_trywait(&semaforo) == 0) {
        std::cout << "Jogador P" << id << " conseguiu uma cadeira!" << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << "Jogador P" << id << " não conseguiu uma cadeira e foi eliminado!" << std::endl;
        eliminados.push_back(id);
    }
}

// Remove os jogadores eliminados da lista de jogadores ativos
void remover_eliminados() {
    jogadores.erase(
        std::remove_if(jogadores.begin(), jogadores.end(),
            [](int id) {
                return std::find(eliminados.begin(), eliminados.end(), id) != eliminados.end();
            }),
        jogadores.end()
    );
}

// Inicia uma rodada do jogo
void iniciar_rodada(int cadeiras) {
    eliminados.clear();
    sem_init(&semaforo, 0, cadeiras);

    std::cout << "\nIniciando rodada com " << cadeiras << " cadeira(s)." << std::endl;
    std::cout << "> A música parou! Os jogadores estão tentando se sentar..." << std::endl;

    std::vector<std::thread> threads;
    for (int jogador : jogadores) {
        threads.emplace_back(tentar_ocupar_cadeira, jogador);
    }

    for (auto& t : threads) {
        t.join();
    }

    remover_eliminados();
    sem_destroy(&semaforo);
}

// Verifica se o jogo terminou
bool jogo_terminou() {
    return jogadores.size() == 1;
}

// Retorna o ID do jogador vencedor
int jogador_vencedor() {
    return jogadores.front();
}

int main() {
    int n = 4; // Número de jogadores
    inicializar_jogadores(n);

    int cadeiras = n - 1;
    while (cadeiras > 0 && !jogo_terminou()) {
        iniciar_rodada(cadeiras);
        cadeiras--; // Remove uma cadeira após cada rodada
    }

    if (jogo_terminou()) {
        std::cout << "\nJogador vencedor: P" << jogador_vencedor() << std::endl;
    }

    std::cout << "Fim do jogo!" << std::endl;
    return 0;
}