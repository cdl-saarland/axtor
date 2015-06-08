int P(void);
int Q(void);
void A(void);
void B(void);

int main(int argc, char ** argv) {
  if (P() && Q())
    A();
  else
    B();

  return 0;
}
