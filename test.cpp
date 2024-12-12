int main()
{
    int * q = new int;
    int i = 1;
    q = & i;

    return * q;
}