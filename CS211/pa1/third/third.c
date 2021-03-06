#include<stdio.h>
#include<stdlib.h>

typedef struct node{
  int key;
  int value;
  struct node* next;
}Node;

typedef struct item{
  Node* head;
}Item;

Item* htable = NULL;

void initialize();
void insert(int);
void search(int);
void freetable();

void initialize(){
  htable = malloc(1000 * sizeof(Item));
  for(int i = 0; i < 1000; i++){
    htable[i].head = NULL;
  }
}

void insert(int n){
  Node* temp = malloc(sizeof(Node));
  temp->key = abs(n/1000);
  temp->value = n;
  temp->next = NULL;
  if(htable[temp->key%1000].head == NULL){
    htable[temp->key%1000].head = temp;
    printf("inserted\n");
    return;
  }
  Node* ptr = htable[temp->key%1000].head;
  while(ptr->next != NULL){
    if(temp->value == ptr->value){
      printf("duplicate\n");
      return;
    }
    ptr = ptr->next;
  }
  if(temp->value == ptr->value){
    printf("duplicate\n");
    return;
  }
  ptr->next = temp;
  printf("inserted\n");
}

void search(int n){
  Node* ptr = htable[(n/1000)%1000].head;
  while(ptr != NULL){
    if(ptr->value == n){
      printf("present\n");
      return;
    }
    ptr = ptr->next;
  }
  printf("absent\n");
}

void freetable(){
  for(int i = 0; i < 1000; i++){
    Node* ptr = htable[i].head;
    while(ptr != NULL){
      Node* temp = ptr;
      ptr = ptr->next;
      free(temp);
    }
  }
  free(htable);
}

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  initialize();
  char act;
  int val;
  while(fscanf(fp, "%c\t%d\n", &act, &val) != EOF){
    if(act == 'i'){
      insert(val);
    }
    if(act == 's'){
      search(val);
    }
  }
  freetable();
  fclose(fp);
  return(0);
}
