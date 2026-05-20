int is_term(char c) {
    return (c == '\0' || c == ' ' || c == '\n');
}

int find_EOL(char** p)
{
    int c = 0;
    while(**p != '\n') {
        (*p)++;
        c++;
    }
    return c;
}

const char *decimal2a(unsigned int n)
{
    static char res[16];
    char *p = res + (sizeof(res)-1);

    if(n == 0)
        return "0";

    *p = 0;
    while(n > 0) {
        p--;
        *p = '0' + n % 10;
        n /= 10;
    }

    return p;
}
