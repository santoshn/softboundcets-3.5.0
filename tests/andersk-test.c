struct node { struct node *next, *prev; } *n;
struct head { struct node *first, *last; } h;
void fn() {
  h.last = n->prev;
  if (n->prev != (void *)&h)
    h.first = n->next;
  else
    n->prev->next = n->next;
  n->next = h.first;
}
int main() { fn(); }
