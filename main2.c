#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define total_crates 12
#define total_fruits 37

int treeFullFlag = 0;
int NoOfCrates = 0;
int NoOfFruits = 0;
int crate[total_crates];
int crate_index = 0;
int tree[total_fruits];
int tree_index = 0;

void *picker(void *arg);
void *loader(void *arg);

pthread_mutex_t lock_on_crates = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_on_tree = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_on_crates = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_on_tree = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_on_loader = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_tree_empty = PTHREAD_COND_INITIALIZER;

int main() {
    pthread_t pickers[3], loader_thread;

    // Create picker threads
    for (long i = 0; i < 3; i++) {
        pthread_create(&pickers[i], NULL, picker, (void *)(i + 1));
    }

    // Create loader thread
    pthread_create(&loader_thread, NULL, loader, NULL);

    // Wait for all threads to finish
    for (int i = 0; i < 3; i++) {
        pthread_join(pickers[i], NULL);
    }
    pthread_join(loader_thread, NULL);

    printf("\nAll fruits have been picked and loaded. Total crates filled: %d\n", NoOfCrates);
    return 0;
}

void *picker(void *arg) {
    long picker_id = (long)arg;
    while (1) {
        pthread_mutex_lock(&lock_on_tree);

        // Check if tree is empty
        if (tree_index >= total_fruits) {
            if (crate_index > 0) {
                printf("\n[Picker %ld] Tree is empty. Waiting for all pickers to reach this point...\n", picker_id);
                treeFullFlag++;
                if (treeFullFlag == 3) {
                    printf("[Picker %ld] All pickers reached. Signaling loader for final crate.\n", picker_id);
                    pthread_cond_signal(&cond_on_loader);
                    pthread_mutex_unlock(&lock_on_tree);
                    break;
                }
                pthread_mutex_unlock(&lock_on_tree);
                break;
            }
        }

        int fruitIndex = ++tree_index;
        pthread_mutex_unlock(&lock_on_tree);

        pthread_mutex_lock(&lock_on_crates);
        if (crate_index < total_crates) {
            crate_index++;
            printf("[Picker %ld] Picked a fruit (Fruit #%d). Placed in Crate #%d (Current Crate Count: %d)\n", 
                picker_id, fruitIndex, NoOfCrates + 1, crate_index);
            pthread_mutex_unlock(&lock_on_crates);
        } else {
            while (crate_index >= total_crates) {
                printf("\n[Picker %ld] Crate is full. Signaling loader to replace crate.\n", picker_id);
                pthread_cond_signal(&cond_on_loader);
                pthread_cond_wait(&cond_on_crates, &lock_on_crates);
                printf("[Picker %ld] New crate arrived. Resuming fruit placement.\n", picker_id);
            }
            crate_index++;
            printf("[Picker %ld] Picked a fruit (Fruit #%d). Placed in Crate #%d (Current Crate Count: %d)\n", 
                picker_id, fruitIndex, NoOfCrates + 1, crate_index);
            pthread_mutex_unlock(&lock_on_crates);
        }

        usleep(10000); // Simulate work
    }
}

void *loader(void *arg) {
    while (1) {
        printf("\n[Loader] Waiting to be signaled by pickers...\n");
        pthread_mutex_lock(&lock_on_crates);
        printf("[Loader] Lock acquired on crates.\n");

        if (tree_index >= total_fruits) {
            printf("\n[Loader] Tree is empty. Loading final partial crate with %d fruits. Exiting...\n", crate_index);
            pthread_mutex_unlock(&lock_on_crates);
            break;
        } else {
            printf("[Loader] Crate is not yet full and tree still has fruits. Waiting...\n");
            while (tree_index < total_fruits && crate_index < total_crates) {
                printf("[Loader] Waiting for crate to be full or tree to become empty...\n");
                pthread_cond_wait(&cond_on_loader, &lock_on_crates);
                printf("[Loader] Activated by picker signal.\n");
            }

            if (crate_index > 0) {
                printf("\n[Loader] Crate filled with %d fruits. Loading into truck.\n", crate_index);
                crate_index = 0;
                NoOfCrates++;
                printf("[Loader] New crate placed. Total crates loaded: %d\n", NoOfCrates);
                pthread_cond_broadcast(&cond_on_crates);
                pthread_mutex_unlock(&lock_on_crates);
                printf("[Loader] New empty crate available. Continuing...\n");
                continue;
            }
        }

        usleep(10000); // Simulate work
        printf("[Loader] Simulating loading delay.\n");
    }
}
