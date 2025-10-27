#include <stdio.h>
#include <stdlib.h>

// ======================== COLOR DEFINITIONS ========================
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"

// ======================== STRUCTURES ========================
struct Node {
    int key, value, priority;
    struct Node *prev, *next;
};

#define MAX_EVICTED 10
#define HASH_SIZE 10

struct Node* hashTable[HASH_SIZE];
struct Node* evictedList[MAX_EVICTED];
int evictedCount = 0;
int cacheHits = 0, cacheMisses = 0, evictions = 0;

// ======================== UTILITY FUNCTIONS ========================
int hashFunc(int key) { return key % HASH_SIZE; }

void putInHash(int key, struct Node* node) {
    hashTable[hashFunc(key)] = node;
}

struct Node* getFromHash(int key) {
    int idx = hashFunc(key);
    struct Node* node = hashTable[idx];
    return (node && node->key == key) ? node : NULL;
}

void removeFromHash(int key) {
    int idx = hashFunc(key);
    if (hashTable[idx] && hashTable[idx]->key == key)
        hashTable[idx] = NULL;
}

struct Node* createNode(int key, int value, int priority) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->key = key;
    newNode->value = value;
    newNode->priority = priority;
    newNode->prev = newNode->next = NULL;
    return newNode;
}

struct Node* addNodeToHead(struct Node* head, struct Node* node) {
    node->next = head;
    node->prev = NULL;
    if (head) head->prev = node;
    return node;
}

struct Node* removeNode(struct Node* head, struct Node* node, struct Node** tail) {
    if (node->prev) node->prev->next = node->next;
    else head = node->next;
    if (node->next) node->next->prev = node->prev;
    else *tail = node->prev;
    node->prev = node->next = NULL;
    return head;
}

struct Node* moveToHead(struct Node* head, struct Node* node, struct Node** tail) {
    if (node == head) return head;
    head = removeNode(head, node, tail);
    head = addNodeToHead(head, node);
    return head;
}

struct Node* findLowestPriority(struct Node* tail) {
    struct Node* minNode = tail;
    int minPriority = tail->priority;
    while (tail) {
        if (tail->priority > minPriority)
            minNode = tail, minPriority = tail->priority;
        tail = tail->prev;
    }
    return minNode;
}

void addToEvicted(struct Node* node) {
    if (evictedCount < MAX_EVICTED)
        evictedList[evictedCount++] = node;
    else {
        for (int i = 1; i < MAX_EVICTED; i++)
            evictedList[i - 1] = evictedList[i];
        evictedList[MAX_EVICTED - 1] = node;
    }
}

// ======================== CACHE OPERATIONS ========================
struct Node* put(struct Node* head, struct Node** tail, int key, int value, int priority, int capacity, int* size) {
    struct Node* existing = getFromHash(key);

    if (existing) {
        existing->value = value;
        existing->priority = priority;
        head = moveToHead(head, existing, tail);
        printf(GREEN "✅ Updated key %d with new value %d (Priority %d)\n" RESET, key, value, priority);
        return head;
    }

    if (*size >= capacity && *tail) {
        struct Node* toRemove = findLowestPriority(*tail);
        printf(RED "⚠️  Cache full! Evicting [%d:%d] (Priority %d)\n" RESET,
               toRemove->key, toRemove->value, toRemove->priority);
        head = removeNode(head, toRemove, tail);
        removeFromHash(toRemove->key);
        addToEvicted(toRemove);
        free(toRemove);
        (*size)--; evictions++;
    }

    struct Node* newNode = createNode(key, value, priority);
    head = addNodeToHead(head, newNode);
    putInHash(key, newNode);
    if (*tail == NULL) *tail = newNode;
    (*size)++;

    printf(CYAN "🆕 Inserted key %d -> value %d (Priority %d)\n" RESET, key, value, priority);
    return head;
}

int get(struct Node** head, struct Node** tail, int key) {
    struct Node* node = getFromHash(key);
    if (node) {
        *head = moveToHead(*head, node, tail);
        cacheHits++;
        printf(GREEN "✅ Cache Hit! Key %d found. Value = %d\n" RESET, key, node->value);
        return node->value;
    }
    cacheMisses++;
    printf(YELLOW "❌ Cache Miss! Key %d not found.\n" RESET, key);
    return -1;
}

// ======================== NEW FEATURE: SEARCH ========================
void searchCache(struct Node* head, int key) {
    struct Node* temp = head;
    while (temp) {
        if (temp->key == key) {
            printf(GREEN "\n🔍 Search Result:\n" RESET);
            printf(MAGENTA "+-------------------------------+\n" RESET);
            printf("| Key | Value | Priority | Pos. |\n");
            printf(MAGENTA "+-------------------------------+\n" RESET);

            if (temp == head)
                printf(GREEN "| %-3d | %-5d | %-8d | HEAD |\n" RESET, temp->key, temp->value, temp->priority);
            else if (temp->next == NULL)
                printf(YELLOW "| %-3d | %-5d | %-8d | TAIL |\n" RESET, temp->key, temp->value, temp->priority);
            else
                printf("| %-3d | %-5d | %-8d | MID  |\n", temp->key, temp->value, temp->priority);

            printf(MAGENTA "+-------------------------------+\n" RESET);
            return;
        }
        temp = temp->next;
    }
    printf(RED "❌ Key %d not found in cache.\n" RESET, key);
}

// ======================== DISPLAY FUNCTIONS ========================
void displayCache(struct Node* head, struct Node* tail) {
    printf(BLUE "\n================ CACHE TABLE ================\n" RESET);
    if (!head) {
        printf(RED "Cache is empty!\n" RESET);
        return;
    }

    printf(MAGENTA "+-------------------------------------------+\n" RESET);
    printf(MAGENTA "|  Key  |  Value  | Priority | Position     |\n" RESET);
    printf(MAGENTA "+-------------------------------------------+\n" RESET);

    struct Node* temp = head;
    while (temp) {
        if (temp == head)
            printf(GREEN "|  %-4d |  %-6d |   %-7d| HEAD         |\n" RESET,
                   temp->key, temp->value, temp->priority);
        else if (temp == tail)
            printf(YELLOW "|  %-4d |  %-6d |   %-7d| TAIL         |\n" RESET,
                   temp->key, temp->value, temp->priority);
        else
            printf("|  %-4d |  %-6d |   %-7d| -            |\n",
                   temp->key, temp->value, temp->priority);
        temp = temp->next;
    }
    printf(MAGENTA "+-------------------------------------------+\n" RESET);

    printf(BLUE "\nCache Visualization:\n" RESET);
    printf(GREEN "HEAD → " RESET);
    temp = head;
    while (temp) {
        printf("[Key:%d]", temp->key);
        if (temp->next) printf(" → ");
        temp = temp->next;
    }
    printf(GREEN " → TAIL\n" RESET);
}

void displayStats() {
    printf(CYAN "\n================ CACHE STATISTICS ================\n" RESET);
    printf(GREEN "Cache Hits: %d\n" RESET, cacheHits);
    printf(YELLOW "Cache Misses: %d\n" RESET, cacheMisses);
    printf(RED "Total Evictions: %d\n" RESET, evictions);
    printf(MAGENTA "Evicted History Count: %d\n" RESET, evictedCount);
}

void displayEvictedHistory() {
    printf(CYAN "\n================ EVICTED CACHE HISTORY ================\n" RESET);
    if (evictedCount == 0) {
        printf(RED "No evicted cache entries yet.\n" RESET);
        return;
    }
    for (int i = 0; i < evictedCount; i++)
        printf("[%d:%d (P%d)] ", evictedList[i]->key, evictedList[i]->value, evictedList[i]->priority);
    printf("\n");
}

void clearCache(struct Node** head, struct Node** tail, int* size) {
    struct Node* temp = *head;
    while (temp) {
        struct Node* next = temp->next;
        free(temp);
        temp = next;
    }
    *head = *tail = NULL;
    *size = 0;
    evictedCount = cacheHits = cacheMisses = evictions = 0;
    for (int i = 0; i < HASH_SIZE; i++) hashTable[i] = NULL;
    printf(RED "🧹 Cache cleared successfully!\n" RESET);
}

// ======================== FILE HANDLING ========================
void saveCache(struct Node* head) {
    FILE* fp = fopen("cache_data.txt", "w");
    while (head) {
        fprintf(fp, "%d %d %d\n", head->key, head->value, head->priority);
        head = head->next;
    }
    fclose(fp);
    printf(GREEN "💾 Cache saved to file successfully!\n" RESET);
}

struct Node* loadCache(struct Node* head, struct Node** tail, int capacity, int* size) {
    FILE* fp = fopen("cache_data.txt", "r");
    if (!fp) return NULL;
    int key, value, priority;
    while (fscanf(fp, "%d %d %d", &key, &value, &priority) == 3)
        head = put(head, tail, key, value, priority, capacity, size);
    fclose(fp);
    printf(GREEN "📂 Cache loaded from file successfully!\n" RESET);
    return head;
}

// ======================== MAIN PROGRAM ========================
int main() {
    int capacity, size = 0;
    struct Node* head = NULL;
    struct Node* tail = NULL;
    int choice, key, value, priority;

    printf("Enter cache capacity: ");
    scanf("%d", &capacity);
    head = loadCache(head, &tail, capacity, &size);

    while (1) {
        printf(BLUE "\n=====================================================\n" RESET);
        printf(CYAN "          LRU CACHE MANAGEMENT SYSTEM\n" RESET);
        printf(BLUE "=====================================================\n" RESET);
        printf("1. Insert / Update Data\n");
        printf("2. Get Value by Key (Update Order)\n");
        printf("3. Display Cache Table\n");
        printf("4. Show Cache Statistics\n");
        printf("5. Show Evicted History\n");
        printf("6. Clear Entire Cache\n");
        printf("7. Save Cache to File\n");
        printf("8. Search Key (Without Updating Order)\n");
        printf("9. Exit Program\n");
        printf("-----------------------------------------------------\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter key, value, priority (1=High,2=Med,3=Low): ");
                scanf("%d %d %d", &key, &value, &priority);
                head = put(head, &tail, key, value, priority, capacity, &size);
                break;
            case 2:
                printf("Enter key: ");
                scanf("%d", &key);
                get(&head, &tail, key);
                break;
            case 3:
                displayCache(head, tail);
                break;
            case 4:
                displayStats();
                break;
            case 5:
                displayEvictedHistory();
                break;
            case 6:
                clearCache(&head, &tail, &size);
                break;
            case 7:
                saveCache(head);
                break;
            case 8:
                printf("Enter key to search: ");
                scanf("%d", &key);
                searchCache(head, key);
                break;
            case 9:
                saveCache(head);
                printf(GREEN "Exiting program... Goodbye!\n" RESET);
                return 0;
            default:
                printf(RED "Invalid choice! Try again.\n" RESET);
        }
    }
}
