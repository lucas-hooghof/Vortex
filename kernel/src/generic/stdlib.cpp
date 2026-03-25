#include <generic/stdlib.h>

char* itoa(int number, int base)
{
    if (base < 2 || base > 16)
        return nullptr;

    // Max length for 32-bit int in base 2 + sign + null
    int max_len = 35;
    char* buffer = (char*)malloc(max_len);
    if (!buffer)
        return nullptr;

    int i = 0;
    bool isNegative = false;

    // Handle 0 explicitly
    if (number == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return buffer;
    }

    // Handle negative (only for base 10)
    if (number < 0 && base == 10)
    {
        isNegative = true;
        number = -number;
    }

    // Convert digits (in reverse)
    while (number != 0)
    {
        int rem = number % base;

        if (rem < 10)
            buffer[i++] = '0' + rem;
        else
            buffer[i++] = 'A' + (rem - 10);

        number /= base;
    }

    if (isNegative)
        buffer[i++] = '-';

    buffer[i] = '\0';

    // Reverse string
    for (int j = 0; j < i / 2; j++)
    {
        char tmp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = tmp;
    }

    return buffer;
}