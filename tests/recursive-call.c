#include<stdio.h>
#include<stdlib.h>

struct node{

  int a;
  int b;
  struct node * left, *right;
};

size_t ind = 0;

struct node* create_node(int a, int b){

  struct node * new_node = (struct node*) malloc(sizeof(struct node));


  new_node->a = ind++;
  new_node->b = new_node->a;

  return new_node;
}


struct node * root = NULL;

struct node * build_tree ( int level){
  
  if (level == 0) return NULL;
  struct node *temp_root = NULL;
  
  temp_root = create_node(level, level);

  temp_root->left = build_tree(level-1);

  temp_root->right = build_tree(level-1);  
  
  return temp_root;
}

void traverse(struct node * temp){

  if(temp == NULL) return;

  traverse(temp->left);
  traverse(temp->right);

  printf("[program]root: %d\n", temp->a);

}

int main(int argc, char** argv){

  int i = 0;

  root = build_tree(2);
  traverse(root);
  return 0;
}
