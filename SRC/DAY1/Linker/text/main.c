//global, notype, undefined
unsigned long long sum(unsigned long long *a, unsigned long long n);
//global, object, .data
unsigned long long array[2] = {0x12340000, 0xabcd};
//global, object, .data
unsigned long long bias = 0xf00000000;

//global, function, .test
void main()
{
    unsigned long long var = sum(array, 2);
}