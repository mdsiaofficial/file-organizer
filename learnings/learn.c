#include <stdio.h>
#include <string.h>

int main()
{
    const char *str = "file.txt";
    const char *ext = strrchr(str, '.');

    if (ext)
    {
        printf("Extension found: %s\n", ext);
    }
    else
    {
        printf("No extension found.\n");
    }
    return 0;
}