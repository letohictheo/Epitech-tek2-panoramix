#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define NUM_THREADS 5

sem_t sem;
sem_t dsem;
pthread_mutex_t lock;


typedef struct village_s {
    long villagers;
    long pot_size;
    long nb_fights;
    long nb_refills;
} village_t;

village_t *village(void)
{
    static village_t *village;

    if (!village)
        village = calloc(1, sizeof(village_t));
    return village;
}

void *druid(void *threadid) {
   printf("Druid : I'm ready... but sleepy...\n");
   sem_post(&sem);
   while (village()->nb_refills) {
      sem_wait(&dsem);
      pthread_mutex_lock(&lock);
      printf("Druid : Ah! Yes, I'm awake! working on it! Beware I can only make %d more refills after this one\n", village()->nb_refills);
      village()->pot_size = 2;
      village()->nb_refills--;
      pthread_mutex_unlock(&lock);
      sem_post(&sem);
   }
   pthread_exit(NULL);
}

void *villager(void *threadid) {
   int a = *(int *)threadid;
   int left = village()->pot_size;
   int fight = village()->nb_fights;
   sem_wait(&sem);
   printf("Villager %d : Going into battle!\n", a);
   sem_post(&sem);
   sleep(1);
   for (int i = 0; i != fight + 1; fight--) {
      sem_wait(&sem);
      if (village()->pot_size) {
         printf("Villager %d : I need a drink I see %d serving left!\n", a, village()->pot_size);
         pthread_mutex_lock(&lock);
         village()->pot_size--;
         pthread_mutex_unlock(&lock);
      } else if (village()->nb_refills) {
         printf("Villager %d : I need a drink I see %d serving left!\n", a, village()->pot_size);
         printf("Villager %d : hey pano wake up we need more potion!\n", a);
         sem_post(&dsem);
         sem_wait(&sem);
      } else {
         sem_post(&sem);
         printf("Villager %d: I'm going to sleep now.\n", a);
         pthread_exit(NULL);
      }
      sem_post(&sem);
   }
   printf("Villager %d: I'm going to sleep now.\n", a);
   pthread_exit(NULL);
}

int main () {
   sem_init(&sem, 0, 0);
   sem_init(&dsem, 0, 0);
   village()->villagers = 3;
   village()->nb_fights = 3;
   village()->nb_refills = 4;
   village()->pot_size = 2;
   if (pthread_mutex_init(&lock, NULL) != 0) {
      printf("\n mutex init failed\n");
      return 84;
   }
   pthread_t *threads = malloc(sizeof(pthread_t) * (village()->villagers + 1));
   threads[village()->villagers] = NULL;
   pthread_t tdruid;
   int rc;
   int i;
   int j = 0;
   rc = pthread_create(&tdruid, NULL, druid, NULL);
   for(i = 0; i < village()->villagers; i++) {
      int *a = malloc(sizeof(int));
      *a = i;
      rc = pthread_create(&threads[i], NULL, villager, a);
   }
   for(i = 0; i < village()->villagers; i++) {
      rc = pthread_join(threads[i], NULL);
   }
   pthread_cancel(tdruid);
   pthread_join(tdruid, NULL);
}