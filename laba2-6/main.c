#include "functions.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        FILE *input_stream = NULL;
        int needs_closing = 0;

        if (argv[1][0] == '-' && argv[1][1] == '\0') {
            input_stream = stdin;
        } else {
            input_stream = fopen(argv[1], "r");
            if (!input_stream) {
                fprintf(stderr, "Error: Could not open input file '%s'.\n", argv[1]);
                return 1;
            }
            needs_closing = 1;
        }

        int r_val = 0;
        unsigned int z_val = 0;
        int cv_val = 0;
        int CV_val = 0;
        int d_val = 0;
        char s_val[64];
        float f_val = 0.0f;

        int matched_count = overfscanf(input_stream, "%Ro %Zr %Cv %CV %d %s %f",
                                       &r_val,
                                       &z_val,
                                       &cv_val, 16,
                                       &CV_val, 16,
                                       &d_val,
                                       s_val,
                                       &f_val);

        if (needs_closing) {
            fclose(input_stream);
        }

        printf("Successfully parsed fields: %d\n", matched_count);
        printf("Roman value: %d\n", r_val);
        printf("Zeckendorf value: %u\n", z_val);
        printf("Base 16 lower value: %d\n", cv_val);
        printf("Base 16 upper value: %d\n", CV_val);
        printf("Standard integer: %d\n", d_val);
        printf("Standard string: %s\n", s_val);
        printf("Standard float: %.2f\n", f_val);

        return (matched_count > 0) ? 0 : 1;
    }

    printf("=== Demonstration of oversscanf ===\n");

    const char *sample_data = "MCMXCIV 001011 ff FF 42 hello 3.14";
    int r_val = 0;
    unsigned int z_val = 0;
    int cv_val = 0;
    int CV_val = 0;
    int d_val = 0;
    char s_val[64];
    float f_val = 0.0f;

    int matched_count = oversscanf(sample_data, "%Ro %Zr %Cv %CV %d %s %f",
                                    &r_val,
                                    &z_val,
                                    &cv_val, 16,
                                    &CV_val, 16,
                                    &d_val,
                                    s_val,
                                    &f_val);

    printf("Successfully parsed fields: %d\n", matched_count);
    printf("Roman (MCMXCIV): %d\n", r_val);
    printf("Zeckendorf (001011): %u\n", z_val);
    printf("Base 16 lower ('ff'): %d\n", cv_val);
    printf("Base 16 upper ('FF'): %d\n", CV_val);
    printf("Standard integer: %d\n", d_val);
    printf("Standard string: %s\n", s_val);
    printf("Standard float: %.2f\n", f_val);

    return 0;
}