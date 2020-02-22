#include<stdio.h>
#include<stdlib.h>

typedef struct node{
  int number;
  struct node* next;
}Node;

Node* head = NULL;
void initialize();
void insert(int);
void delete(int);
void printlist();
void freelist();

void initialize(){
  Node* temp = malloc(sizeof(Node));
  temp->number = 0;
  temp->next = NULL;
  head = temp;
}

void insert(int num){
  if(head->number == 0){
    Node* temp = malloc(sizeof(Node));
    temp->number = num;
    temp->next = NULL;
    head->number++;
    head->next = temp;
    return;
  }
  Node* ptr = head;
  while(ptr->next != NULL){
    if(ptr->next->number == num){
      return;
    }
    if(ptr->next->number > num){
      break;
    }
    ptr = ptr->next;
  }
  Node* temp = malloc(sizeof(Node));
  temp->number = num;
  temp->next = ptr->next;
  ptr->next = temp;
  head->number++;
}

void delete(int num){
  Node* temp = head;
  while(temp->next != NULL){
    if(temp->next->number == num){
      Node* del = temp->next;
      temp->next = temp->next->next;
      head->number--;
      free(del);
      return;
    }
    temp = temp->next;
  }
}

void printlist(){
  Node* temp = head->next;
  if(temp == NULL){
    printf("0\n");
    printf("\n");
    return;
  }
  printf("%d\n", head->number);
  while(temp->next != NULL){
    printf("%d\t", temp->number);
    temp = temp->next;
  }
  printf("%d\n", temp->number);
}

void freelist(){
  Node * temp = head;
  while(temp != NULL){
    Node * temp2 = temp;
    temp = temp->next;
    free(temp2);
  }
}

int main(int argc, char** argv){
  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    printf("error\n");
    exit(0);
  }
  initialize();
  char act;
  int val;
  while(fscanf(fp, "%c\t%d\n", &act, &val) != EOF){
    if(act == 'i'){
      insert(val);
    }
    if(act == 'd'){
      delete(val);
    }
  }
  printlist();
  freelist();
  fclose(fp);
  return(0);
}
