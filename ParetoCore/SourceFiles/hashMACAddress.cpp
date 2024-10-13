unsigned long hashString(char *str,int tmp)
{
    unsigned long hash = 7;
    int c;
	int offset = 0;
	str = str+offset+tmp;
    while (c = *str++)
        hash = ((hash << 2) + hash) + c; 
 
    return hash;
}