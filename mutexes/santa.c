#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_REINDEER 9
#define DELIVERY_ROUNDS 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_cond = PTHREAD_COND_INITIALIZER;

int reindeer_count = 0;
int deliveries_made = 0;
int finished = 0;

/*
 * Zachowania reniferów:
 *
 * Są na wakacjach w ciepłych krajach losowy okres czasu (5-10s)
 * Wracaja na biegun północny (Komunikat: Renifer: czeka _ reniferów na Mikołaja, ID), jeśli wracający renifer jest dziewiątym reniferem to wybudza Mikołaja (Komunikat: Renifer: wybudzam Mikołaja, ID).
 * Dostarczają wraz z Mikołajem zabawki grzecznym dzieciom (i studentom którzy nie spóźniają się z dostarczaniem zestawów) przez (2-4s).
 * Lecą na wakacje.*/

void *reindeer(void *id) {
    int reindeer_id = *((int *)id);
    free(id);

    while (1) {
        if (finished) break;

        sleep((rand() % 6) + 5);

        pthread_mutex_lock(&mutex);
        reindeer_count++;

        printf("Renifer ID= %d: Na Mikołaja czeka już %d reniferów.\n", reindeer_id, reindeer_count);

        if (reindeer_count == NUM_REINDEER) {
            printf("Renifer ID= %d: wybudzam Mikołaja.\n", reindeer_id);
            pthread_cond_signal(&santa_cond);
        }

        while (reindeer_count > 0 && !finished) {
            pthread_cond_wait(&reindeer_cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


/*
 * Zachowania Mikołaja:
 *
 * Śpi.
 * Kiedy zostaje wybudzony (Komunikat: Mikołaj: budzę się) to dostarcza wraz z reniferami zabawki (Komunikat: Mikołaj: dostarczam zabawki) (2-4s).
 * Wraca do snu (Komunikat: Mikołaj: zasypiam).*/

void *santa(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (reindeer_count < NUM_REINDEER && deliveries_made < DELIVERY_ROUNDS) {
            pthread_cond_wait(&santa_cond, &mutex);
        }

        if (deliveries_made >= DELIVERY_ROUNDS) {
            finished = 1;
            pthread_cond_broadcast(&reindeer_cond);
            pthread_mutex_unlock(&mutex);
            break;
        }

        printf("Mikołaj: budzę się.\n");
        printf("Mikołaj: dostarczam zabawki.\n");
        sleep((rand() % 3) + 2);
        deliveries_made++;

        reindeer_count = 0;
        printf("Mikołaj: zasypiam.\n");
        pthread_cond_broadcast(&reindeer_cond);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t santa_thread;
    pthread_t reindeer_threads[NUM_REINDEER];

    srand(time(NULL));
    pthread_create(&santa_thread, NULL, santa, NULL);

    for (int i = 0; i < NUM_REINDEER; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&reindeer_threads[i], NULL, reindeer, id);
    }

    pthread_join(santa_thread, NULL);

    for (int i = 0; i < NUM_REINDEER; i++) {
        pthread_join(reindeer_threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&santa_cond);
    pthread_cond_destroy(&reindeer_cond);

    return 0;
}
