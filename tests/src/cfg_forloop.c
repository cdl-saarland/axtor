int P(void);
void A(int);

int main(int argc, char ** argv) {
  int limit = P();
  for (int i =0; i < limit; ++i)
    A(i);
  
return 0;
}
