#include <stdio.h>

void f1 (void) { printf("%i\n", 1); }
void f2 (void) { printf("%i\n", 2); }
void f3 (void) { printf("%i\n", 3); }
void f4 (void) { printf("%i\n", 4); }
void f5 (void) { printf("%i\n", 5); }
void f6 (void) { printf("%i\n", 6); }
void f7 (void) { printf("%i\n", 7); }
void f8 (void) { printf("%i\n", 8); }
void f9 (void) { printf("%i\n", 9); }

typedef void(*void_fp)(void);

// There is a basic check that excludes all functions that aren't used anywhere
// This ensures that check can't work in this example
const void_fp fp_all[] = {f1, f2 ,f3, f4, f5 ,f6, f7, f8, f9};

void func(int i, int j)
{
  const void_fp fp_tbl[] = {fp_all[i*2], fp_all[j+1]};
  // Illegal:
  //fp_tbl[1] = f4;
  const void_fp fp = fp_tbl[1];
  fp();
}

int main()
{
  for(int i=0;i<3;i++)
  {
    func(i,0);
  }

  return 0;
}
