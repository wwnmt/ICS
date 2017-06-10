#include"trap.h"
#define N 10

static int T[] = {1,2,3,4,5,6,7,8,9,10};
void test(int *a){
	int i;
	for (i = 1; i < N; i++){
		if (a[i-1] > a[i]){
			int temp = a[i];
			int j = i;
			while (j > 0 && a[i-1] >temp){
				a[i] = a[i-1];
				j--;
			}
			a[j] = temp;
		}
	}
	for (i = 0; i < N; i++)
		nemu_assert(a[i] == T[i]);
}


int main(){
	int L[]={1,2,3,4,5,6,7,8,9,10};
			
	test(L);
	HIT_GOOD_TRAP;
	return 0;
}
