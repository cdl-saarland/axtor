int A(int a);
void S(int x);

int main(int argc, char ** argv) {
  int a = A(0); 
  int b = A(1); 

  // bitwise
  S(a & b);
  S(a | b);
  S(a ^ b);
  
  // arithmetic
  S(a + b);
  S(a - b);
  S(a * b);
  S(a / b);
  S(a % b);
  
  return 0;
}
