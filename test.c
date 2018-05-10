int add(int a, int b);
int sub(int a, int b);
int mul(int a, int b);
int X(int a, int b);
int Y(int a, int b);
int Z(int a, int b);
int U(int a, int b);
int V(int a, int b);
int W(int a, int b);



void f1();
void f2();

void (*fp1)();
void (*fp2)();

int X(int a, int b)
{
	Y(a,b);
	Z(a,b);

	return (a*b);
}

int Y(int a, int b)
{
	return (a/b);
}

int Z(int a, int b)
{
	return (a/b);
}

void f1()
{
	int x = 1;
}

void f2()
{
	int x = 1;
}


int add(int a, int b)
{
	int c;

	if(c)
		add(a-1, b);

	sub(a, b);

	return (a+b);
}

int sub(int a, int b)
{
	add(a,b);

	mul(a,b);

	X(a, b);
	U(a, b);
	V(a, b);

	return (a-b);
}

int mul(int a, int b)
{
	int i, sum=a;

	for(i=0; i < b; i++)
	{
//		sum=add(sum,1);
	}

	X(a, b);

	return sum;
}

int main()
{
	int x, y, z;

	x = 1;
	y = 2;

	z = add(x, y);
	z = sub(x, y);
	z = mul(x, y);

	fp1 = f1;

	fp1();

	return 0;
}
