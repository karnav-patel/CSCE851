#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    unsigned char buffer[4096];
    int exit_code = EXIT_SUCCESS;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [file ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int index = 1; index < argc; ++index) {
        FILE *input = fopen(argv[index], "rb");
        if (input == NULL) {
            fprintf(stderr, "cat: cannot open %s: %s\n", argv[index], strerror(errno));
            exit_code = EXIT_FAILURE;
            continue;
        }

        size_t bytes_read = 0;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
            if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {
                fprintf(stderr, "cat: write error\n");
                fclose(input);
                return EXIT_FAILURE;
            }
        }

        if (ferror(input)) {
            fprintf(stderr, "cat: read error on %s\n", argv[index]);
            exit_code = EXIT_FAILURE;
        }

        fclose(input);
    }

    return exit_code;
}
