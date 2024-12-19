#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

typedef struct Queue {
    int* data;              // Vetor que armazena os elementos da fila
    int front;              // Índice do primeiro elemento da fila
    int rear;               // Índice do último elemento da fila
    int size;               // Tamanho atual da fila
    int capacity;           // Capacidade máxima da fila
} Queue;

// Funções para manipulação da fila
void init_queue(Queue* queue, int capacity) {
    queue->data = (int*)malloc(sizeof(int) * capacity);
    queue->front = 0;
    queue->rear = 0;
    queue->size = 0;
    queue->capacity = capacity;
}

int is_empty(Queue* queue) {
    return queue->size == 0;
}

int is_full(Queue* queue) {
    return queue->size == queue->capacity;
}

void enqueue(Queue* queue, int value) {
    if (is_full(queue)) {
        printf("Fila cheia! Não é possível adicionar mais padawans.\n");
        return;
    }
    queue->data[queue->rear] = value;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->size++;
}

int dequeue(Queue* queue) {
    if (is_empty(queue)) {
        printf("Fila vazia! Não há padawans para remover.\n");
        return -1; // Retorna um valor inválido
    }
    int value = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return value;
}

void free_queue(Queue* queue) {
    free(queue->data);
}

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Constantes
#define MAX_PADAWANS 10
#define MAX_PUBLIC 7
#define MAX_SALAO 5
#define MAX_PUBLIC_SALAO 5

// Semáforos
sem_t sem_padawans;  // Controle de acesso ao salão para Padawans
sem_t sem_public;    // Controle de acesso ao salão para espectadores
sem_t sem_yoda;      // Sincronização entre Yoda e os Padawans
sem_t sem_tests;    // Sincronização para o inicio do teste
sem_t sem_results; // Sincronização para os resultados dos Padawans

pthread_mutex_t room_mutex; // Proteção para variáveis compartilhadas
pthread_mutex_t finished_test_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger a condição
pthread_mutex_t start_test_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger a condição


pthread_cond_t start_test_cond = PTHREAD_COND_INITIALIZER;     // Variável condicional
pthread_cond_t finished_test_cond = PTHREAD_COND_INITIALIZER;     // Variável condicional
int finished_test = 1;                               // ID liberado
int start_test_id = -1;

// Contadores globais
int padawans_in_room = 0;
int spectators_in_room = 0;
int processed_padawans = 0;

int padawans_ids[MAX_PADAWANS];
int testing_padawans[MAX_PADAWANS];
Queue padawan_queue;

// Funções de Yoda
void* yoda_actions(void* arg) {
    printf("Yoda está se preparando para as avaliações...\n");
    int in_room_padawan_ids[MAX_SALAO];
    while (processed_padawans < MAX_PADAWANS) {
        // Aguardando que o salão encha
        printf("Yoda está aguardando o salão encher para iniciar os testes...\n");
        sem_wait(&sem_yoda);

        // Inicia os testes na ordem da fila
        printf("Yoda inicia os testes!\n");

        pthread_mutex_lock(&finished_test_mutex);   // Protege o acesso à variável `released_id`

        finished_test = 0;             // Define o ID liberado

        pthread_cond_broadcast(&finished_test_cond);   // Sinaliza a variável condicional para acordar as threads
        pthread_mutex_unlock(&finished_test_mutex); // Libera o mutex

        for (int i = 0; i < MAX_SALAO; i++) {
            pthread_mutex_lock(&queue_mutex);
            if (!is_empty(&padawan_queue)) {
                int padawan_id = dequeue(&padawan_queue); // Obtém o primeiro Padawan na fila
                in_room_padawan_ids[i] = padawan_id;                
                pthread_mutex_lock(&start_test_mutex);   // Protege o acesso à variável `released_id`
                testing_padawans[padawan_id - 1] = 0;             // Define o ID liberado
                pthread_cond_broadcast(&start_test_cond);   // Sinaliza a variável condicional para acordar as threads
                pthread_mutex_unlock(&start_test_mutex); // Libera o mutex
            }
            pthread_mutex_unlock(&queue_mutex);
        }

        // Simula o tempo de teste
        sleep(2);

        // Processa os resultados
        pthread_mutex_lock(&room_mutex);
        for (int i = 0; i < padawans_in_room; i++) {
            int aprovado = rand() % 2; // Simula aprovação/reprovação
            printf("Yoda anuncia: Padawan_%d %s.\n", in_room_padawan_ids[i], (aprovado ? "aprovado! Cortando a trança" : "reprovado. Apenas cumprimenta Yoda"));
            processed_padawans++;
        }
        
        for (int i = 0; i < MAX_SALAO; i++) {
            sem_post(&sem_padawans); // Libera vagas para novos Padawans
        }
        for (int i = 0; i < MAX_PUBLIC_SALAO; i++) {
            sem_post(&sem_public); // Libera vagas para novos espectadores
        }

        printf("Yoda permite a entrada de novos participantes.\n");

        padawans_in_room = 0;
        pthread_mutex_unlock(&room_mutex);


        pthread_mutex_lock(&finished_test_mutex);   // Protege o acesso à variável `released_id`

        finished_test = 1;             // Define o ID liberado

        pthread_cond_broadcast(&finished_test_cond);   // Sinaliza a variável condicional para acordar as threads
        pthread_mutex_unlock(&finished_test_mutex);
        
        printf("Yoda aguarda para a próxima sessão...\n");
        sleep(3); // Simula o intervalo
        printf("Yoda permite a entrada dos novos padawans...\n");

    }

    // Finaliza os testes
    printf("Yoda faz um discurso final e encerra as avaliações.\n");
    return NULL;
}


// Funções dos Padawans
void* padawan_actions(void* arg) {
    int id = *(int*)arg;
    printf("Padawan_%d chega ao salão e aguarda entrada...\n", id);
    
    // Adiciona o Padawan à fila
    pthread_mutex_lock(&queue_mutex);
    enqueue(&padawan_queue, id);
    pthread_mutex_unlock(&queue_mutex);

    sem_wait(&sem_padawans); // Aguarda permissão para entrar
    while(finished_test == 0) {
        pthread_mutex_lock(&finished_test_mutex);  // Protege o acesso à variável `released_id`
        pthread_cond_wait(&finished_test_cond, &finished_test_mutex);
        pthread_mutex_unlock(&finished_test_mutex);  // Libera o mutex
    }

    pthread_mutex_lock(&room_mutex);
    printf("Padawan_%d entra no salão e cumprimenta formalmente os Mestres Avaliadores.\n", id);


    padawans_in_room++;
    if (padawans_in_room == MAX_SALAO) {
        // Notifica Yoda que o salão está cheio
        sem_post(&sem_yoda);
    }
    pthread_mutex_unlock(&room_mutex);

    while(testing_padawans[id-1] == 1){
        pthread_mutex_lock(&start_test_mutex);  // Protege o acesso à variável `released_id`
        pthread_cond_wait(&start_test_cond, &start_test_mutex);
        pthread_mutex_unlock(&start_test_mutex);  // Libera o mutex
    }
    // Aguarda o corte da trança ou cumprimento final
    printf("Padawan_%d faz sua avaliação e aguarda resultado da avaliação.\n", id);
    while(finished_test == 0) {
        pthread_mutex_lock(&finished_test_mutex);  // Protege o acesso à variável `released_id`
        pthread_cond_wait(&finished_test_cond, &finished_test_mutex);
        pthread_mutex_unlock(&finished_test_mutex);  // Libera o mutex
    }
    printf("Padawan_%d sai do salão.\n", id);


    free(arg);
    return NULL;
}

// Funções dos espectadores
void* public_actions(void* arg) {
    int id = *(int*)arg;
    printf("Espectador_%d chega ao salão e aguarda entrada...\n", id);

    sem_wait(&sem_public); // Aguarda permissão para entrar

    pthread_mutex_lock(&room_mutex);
    spectators_in_room++;
    printf("Espectador_%d entra no salão. Total de espectadores no salão: %d.\n", id, spectators_in_room);
    pthread_mutex_unlock(&room_mutex);

    sleep((rand() % 3) + 1); // Simula tempo assistindo

    pthread_mutex_lock(&room_mutex);
    spectators_in_room--;
    printf("Espectador_%d sai do salão. Total de espectadores no salão: %d.\n", id, spectators_in_room);
    pthread_mutex_unlock(&room_mutex);
    free(arg);
    return NULL;
}

int main() {
    pthread_t yoda;
    pthread_t padawans[MAX_PADAWANS];
    pthread_t public_threads[MAX_PUBLIC];
    srand(time(NULL));

    // Inicializa semáforos e mutex
    sem_init(&sem_padawans, 0, MAX_SALAO); // Máximo de Padawans no salão
    sem_init(&sem_public, 0, MAX_PUBLIC_SALAO); // Máximo de espectadores
    sem_init(&sem_yoda, 0, 0);            // Sincronização com Yoda
    init_queue(&padawan_queue, MAX_PADAWANS);

    pthread_mutex_init(&room_mutex, NULL);

    // Cria thread de Yoda
    pthread_create(&yoda, NULL, yoda_actions, NULL);

    // Cria threads do público
    for (int i = 0; i < MAX_PUBLIC; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&public_threads[i], NULL, public_actions, id);
    }

    // Cria threads de Padawans
    for (int i = 0; i < MAX_PADAWANS; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        padawans_ids[i] = *id;
        testing_padawans[i] = 1;
        pthread_create(&padawans[i], NULL, padawan_actions, id);
    }

    // Aguarda finalização
    pthread_join(yoda, NULL); // Aguarda Yoda terminar
    for (int i = 0; i < MAX_PADAWANS; i++) pthread_join(padawans[i], NULL);
    for (int i = 0; i < MAX_PUBLIC; i++) pthread_join(public_threads[i], NULL);

    // Destrói semáforos e mutex
    sem_destroy(&sem_padawans);
    sem_destroy(&sem_public);
    sem_destroy(&sem_yoda);
    sem_destroy(&sem_tests);
    sem_destroy(&sem_results);
    pthread_mutex_destroy(&room_mutex);

    pthread_cond_destroy(&finished_test_cond);
    pthread_mutex_destroy(&finished_test_mutex);

    return 0;
}
