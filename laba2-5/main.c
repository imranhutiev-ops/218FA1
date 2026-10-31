#include "functions.h"
#include <stdio.h>

int main(void) {
    char out_buf[2048];

    overfprintf(stdout, "=== Demo overfprintf ===\n");
    overfprintf(stdout, "Roman: 1994 = %Ro, 42 = %Ro, 9 = %Ro\n", 1994, 42, 9);
    overfprintf(stdout, "Zeckendorf: 11 = %Zr, 17 = %Zr\n", 11u, 17u);
    overfprintf(stdout, "Base conversion: 255 in base 16 lower = %Cv, upper = %CV\n", 255, 16, 255, 16);
    overfprintf(stdout, "Parse to base 10: 'ff' base 16 = %to, 'FF' base 16 = %TO\n", "ff", 16, "FF", 16);

    int val_i = 1;
    overfprintf(stdout, "Dump int (1): %mi\n", val_i);

    unsigned int val_ui = 255;
    overfprintf(stdout, "Dump uint (255): %mu\n", val_ui);

    float val_f = 1.0f;
    overfprintf(stdout, "Dump float (1.0): %mf\n", val_f);

    double val_d = 1.0;
    overfprintf(stdout, "Dump double (1.0): %md\n", val_d);

    overfprintf(stdout, "Standard specs: string = '%s', int = %d, float = %.2f\n", "test string", 123, 45.67);

    oversprintf(out_buf, "=== Demo oversprintf ===\nRoman: %Ro\nZeckendorf: %Zr\nBase 16: %Cv / %CV\n", 2026, 42u, 1024, 16, 1024, 16);
    printf("\n%s", out_buf);

    return 0;
}