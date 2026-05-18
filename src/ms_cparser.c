
static int find_end(const char *buf, int buflen)
{
    int i;
    for(i = 1; i < buflen; i++)
        if(buf[i] == '\n' && buf[i-1] == '\n')
            return i - 1;
    return -1;
}
