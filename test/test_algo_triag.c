#include <stdio.h>
#include <stdbool.h>

int main()
{
    int data = 0, bounded_data = 0, offset = 40, range = 4015;
    for (int i = 0; i <= 4096 * 4; i++)
    {
        data -= 40;
        bounded_data = (data % range) * ((data / range + 1) % 2) + (range - data % range) * ((data / range) % 2) + offset;
        data += 40;
        printf("%7d %4d\n", data, bounded_data);
        data++;
    }
}