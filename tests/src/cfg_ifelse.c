int P(void);
void A(void);
void B(void);

int main(int argc, char ** argv) {
  if (P())
    A();
  else
    B();

  return 0;
}
