#include <stdio.h>
#include <stdlib.h>
// NODE STRUCTURE
struct Node {
    int key;
    int value;
    struct Node *prev;
    struct Node *next;
};
// NODE FUNCTION
struct Node* createNode(int key, int value){
    struct Node* newNode = ( struct Node*)malloc(sizeof(struct Node));
    newNode->key = key;
    newNode->value = value;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}
// ADD NODE TO HEAD FUNCTION
struct Node* addNodeToHead(struct Node* head, struct Node* node){
    node->next = head;      // new node points to current head
    node->prev = NULL;      // new node has no prev. node

    if(head != NULL){
        head->prev = node;     // old head points back to new node
    } 
    return node;              // new node become the new head
}
// REMOVE A NODE FUNCTION
struct Node* removeNode(struct Node* head, struct Node* node,struct Node** tail ){
    if(node->prev != NULL){
        node->prev->next = node->next;        // bypass node in prev. node
    } else{
        head = node->next;     //node was headed
    }
    if(node->next != NULL){
        node->next->prev = node->prev;   //bypass node in next node
    } else{
        *tail = node->prev;       //removing tail
    }
    node->prev = node->next = NULL;       // disconnect node
    return head;             // updated node will get return
}
// MOVE NODE TO HEAD
struct Node* moveToHead(struct Node* head, struct Node* node, struct Node** tail){
   // if node is already head, do nothing
    if (node == head) return head;

    // if node is tail, update tail to previous node
    if (node == *tail) {
        *tail = node->prev;
    }

    // Remove the node safely
    head = removeNode(head, node, tail);

    // Add to head again
    head = addNodeToHead(head, node);

    // If list had only one node earlier, fix tail pointer
    if (*tail == NULL) *tail = node;

    return head;
}
// Put() function (PUT NODE IN CACHE)
struct Node* put(struct Node* head, struct Node** tail, int key, int value, int capacity, int* size){
   struct Node* temp = head;
   // check if key already exists
   while(temp != NULL){
    if(temp->key == key){
        temp->value = value;
        head = moveToHead(head, temp, tail);
        return head;
    }
    temp = temp->next;
   }
   
   
    // if cache full, remove tail
    if(*size>= capacity && *tail != NULL){
        struct Node* oldTail = *tail;
        head = removeNode(head, oldTail, tail);
        free(oldTail);
        (*size)--;
    }
    //create and add new node to head
    struct Node* newNode = createNode(key, value);
    head = addNodeToHead(head, newNode);
    if(*tail == NULL) *tail = newNode;    // if tail is null(empty list before), set tail
    (*size)++;
    return head;
}
// GET() FUNCTION 
int get(struct Node** head, struct Node** tail, int key){
    struct Node* temp = *head;
    //search for the key
    while(temp != NULL){
        if(temp->key == key){
            //move node to head since it's accessed
            *head = moveToHead(*head, temp, tail);
            return temp->value;
        }
        temp = temp->next;
    }
    return -1;     // key not found
}
// DISPLAY CACHE
void displayCache(struct Node* head){
    struct Node* temp = head;
    printf(" Cache ( most recent -> least recent):\n");
    while(temp != NULL) {
        printf("[%d:%d]", temp->key, temp->value );
        temp = temp->next;
    }
    printf("\n");
}

//  main function 
int main() {
    int capacity , size = 0;

    struct Node* head = NULL;       
    struct Node* tail = NULL; 
    int choice, key, value;

    printf("Enter   LRU Cache Capacity: ");
    scanf("%d", &capacity);

    while(1) {
        printf("\n---MENU ---\n");
        printf("1. Put (Insert / Update )\n");
        printf("2. Get (Access key)\n");
        printf("3. Display Cache\n");
        printf("4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: 
                printf("Enter key and value: ");
                scanf("%d %d", &key, &value);
                head = put(head, &tail, key, value, capacity, &size );
                printf("Key %d added / updated successfully! \n", key);
                break;

            case 2: 
                printf("Enter key to get value: ");
                scanf("%d", &key );
                value = get(&head, &tail, key);
                if(value != -1)
                printf("Value for key %d = %d\n", key, value);
            else
                printf("Key not found in cache!\n");
            break;

            case 3:
                displayCache(head);
                break;

            case 4:
                printf("Exiting program... Thank you!\n");
                return 0;

            default:
                printf("Invalid choice! Please try again.\n");
        }
    }

    return 0;
}
