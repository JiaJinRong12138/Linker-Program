//global, object, common
unsigned long long bias;
//global, function, .text
unsigned long long sum (unsigned long long *a, unsigned long long n)
{
    unsigned long long i, s = 0;
    for(i = 0; i < n; ++ i)
    {
         s += a[i];
    }
    return s + bias;
}
