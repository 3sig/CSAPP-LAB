
unsigned long a = 0;
void call1(unsigned long address);
void call2(char *s);
void func(unsigned long *arg){

	*arg = 0x6166373939623935;
	*(arg+1) = 0x0;
	unsigned long add = 0x000000005561db78;
	
	call1(add);


}


