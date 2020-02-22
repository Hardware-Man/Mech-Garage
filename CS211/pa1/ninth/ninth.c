#include<stdio.h>
#include<stdlib.h>

typedef struct treenode{
  int value;
  int height;
  struct treenode* left;
  struct treenode* right;
}Treenode;

Treenode* root;

void initialize_tree(int);
void add_leaf(int);
void search_tree(int);
Treenode* delete_leaf(Treenode*, int);
void reduce_height(Treenode*);
void free_tree(Treenode*);

void initialize_tree(int n){
  root = malloc(sizeof(Treenode));
  root->value = n;
  root->height = 1;
  root->left = NULL;
  root->right = NULL;
  printf("inserted %d\n", root->height);
}

void add_leaf(int n){
  Treenode* ptr = root;
  while(ptr != NULL){
    if(ptr->value == n){
      printf("duplicate\n");
      return;
    }
    if(ptr->value > n){
      if(ptr->left == NULL){
	Treenode* temp = malloc(sizeof(Treenode));
	temp->value = n;
	temp->height = (ptr->height)+1;
	temp->left = NULL;
	temp->right = NULL;
	ptr->left = temp;
	printf("inserted %d\n", temp->height);
	return;
      }
      else{
	ptr = ptr->left;
      }
    }
    else{
      if(ptr->right == NULL){
	Treenode* temp = malloc(sizeof(Treenode));
	temp->value = n;
	temp->height = (ptr->height)+1;
	temp->left = NULL;
	temp->right = NULL;
	ptr->right = temp;
	printf("inserted %d\n", temp->height);
	return;
      }
      else{
	ptr = ptr->right;
      }
    }
  }
}

void search_tree(int n){
  Treenode* ptr = root;
  while(ptr != NULL){
    if(ptr->value == n){
      printf("present %d\n", ptr->height);
      return;
    }
    else if(ptr->value > n){
      ptr = ptr->left;
    }
    else{
      ptr = ptr->right;
    }
  }
  printf("absent\n");
}

Treenode* delete_leaf(Treenode* curr, int n){
  if(curr == NULL){
    printf("fail\n");
    return curr;
  }
  if(curr->value == n){
    if(curr->left == NULL){
      if(curr->right == NULL){
	free(curr);
	printf("success\n");
	return NULL;
      }
      Treenode* temp = curr;
      curr = curr->right;
      reduce_height(curr);
      free(temp);
      printf("success\n");
      return curr;
    }
    if(curr->right == NULL){
      Treenode* temp = curr;
      curr = curr->left;
      reduce_height(curr);
      free(temp);
      printf("success\n");
      return curr;
    }
    Treenode* leastright = curr->right;
    if(leastright->left != NULL){
      Treenode* prev = curr->right;
      while(leastright->left != NULL){
	leastright = leastright->left;
      }
      while(prev->left != leastright){
	prev = prev->left;
      }
      prev->left = leastright->right;
      if(prev->left != NULL){
	reduce_height(prev->left);
      }
      curr->value = leastright->value;
      free(leastright);
      printf("success\n");
      return curr;
    }
    curr->right = leastright->right;
    if(curr->right != NULL){
      reduce_height(curr->right);
    }
    curr->value = leastright->value;
    free(leastright);
    printf("success\n");
    return curr;
  }
  else if(curr->value > n){
    curr->left = delete_leaf(curr->left, n);
  }
  else{
    curr->right = delete_leaf(curr->right, n);
  }
  return curr;
}

void reduce_height(Treenode* curr){
  if(curr->left != NULL){
    reduce_height(curr->left);
  }
  if(curr->right != NULL){
    reduce_height(curr->right);
  }
  curr->height = (curr->height)-1;
}

void free_tree(Treenode* curr){
  if(curr->left != NULL){
    free_tree(curr->left);
  }
  if(curr->right != NULL){
    free_tree(curr->right);
  }
  free(curr);
}

int main(int argc, char** argv){
  if(argc != 2) return 0;
  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    printf("error\n");
    exit(0);
  }
  char act;
  int val;
  while(fscanf(fp, "%c\t%d\n", &act, &val) != EOF){
    if(act == 'i'){
      if(root == NULL) initialize_tree(val);
      else add_leaf(val);
    }
    else if(act == 's'){
      search_tree(val);
    }
    else{
      root = delete_leaf(root, val);
    }
  }
  if(root != NULL) free_tree(root);
  fclose(fp);
  return(0);
}
