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

static int iswhitespace(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int str2integer(char* str, long long* llval)
{
    int s = 0;
    long long m = 0;
    const char *p = str;
    while(*p && iswhitespace(*p))
        p++;
    if(*p == '-' || *p == '+') {
        s = (*p == '-');
        p++;
    }
    while(*p && !iswhitespace(*p)) {
        if(*p < '0' || *p > '9')
            return 0;
        m = m * 10 + (*p - '0');
        p++;
    }
    while(*p && iswhitespace(*p))
        p++;
    if(!*p) {
        *llval = s ? -m : m;
        return 1;
    }
    return 0;
}
