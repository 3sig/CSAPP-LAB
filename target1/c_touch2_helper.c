void call(unsigned i);
void call2(unsigned long *i);
void func(){

	unsigned cookie = 0x59b997fa;
	unsigned long  address = 0x00000000004017ec;
	call(cookie);
	call2(&address);
	return;
}
