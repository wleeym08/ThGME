#include <stdio.h>
#include <stdlib.h>
#define LOOP 2

int extract(void);

int main() {
    int option;

    printf("+-------------------------------------------------+\n"
        "|  Touhou General Music Extractor (ThGME) v0.0.1  |\n"
        "|                  By winghearn                   |\n"
        "+-------------------------------------------------+\n\n"
        "Enter 1 to start, any other keys to quit: ");

    scanf("%d", &option);
    if (option == 1)
        extract();
    else {
        printf("\nExit.\n\nPress any key to continue...");
        while (getchar() != '\n');
        getchar();
    }

    return 0;
}

int extract() {
    FILE* data = NULL;
    FILE* info = NULL;
    FILE* file = NULL;
    char* fileName = (char*)malloc(16 * sizeof(char));
    char* temp = (char*)malloc(sizeof(char));
    long introOffset;
    long introSize;
    long loopOffset;
    long loopSize;
    long totalSize;
    long dataSize;
    long fileSize;
    long infoSize;

    printf("\nStarting...\n\n");

    data = fopen("thbgm.dat", "rb");
    if (!data) {
        fprintf(stderr, "ERROR: Could not open thbgm.dat\nPress any key to continue...");
        while (getchar() != '\n');
        getchar();
        return 1;
    }
    else
        printf("Successfully loaded thbgm.dat\n\n");

    info = fopen("thbgm.fmt", "rb");
    if (!info) {
        fprintf(stderr, "ERROR: Could not open thbgm.fmt\nPress any key to continue...");
        while (getchar() != '\n');
        getchar();
        return 1;
    }
    else
        printf("Successfully loaded thbgm.fmt\n\n");

    fseek(info, 0, SEEK_END);
    infoSize = ftell(info);

    fseek(info, 0, SEEK_SET);

    while (ftell(info) < infoSize - 52) {
        fread(fileName, 16, 1, info);
        fread(&introOffset, 4, 1, info);
        fseek(info, 4, SEEK_CUR);
        fread(&introSize, 4, 1, info);
        fread(&totalSize, 4, 1, info);
        fseek(info, 20, SEEK_CUR);
        loopOffset = introOffset + introSize;
        loopSize = totalSize - introSize;

        printf("Extracting %s...\n", fileName);

        file = fopen(fileName, "wb+");
        fwrite("RIFF\0\0\0\1WAVEfmt ", 1, 16, file);
        fwrite((char[]) {16, 0, 0, 0}, 1, 4, file);
        fwrite((char[]) {1, 0, 2, 0}, 1, 4, file);
        fwrite((char[]) {68, 172, 0, 0}, 1, 4, file);
        fwrite((char[]) {16, 177, 2, 0}, 1, 4, file);
        fwrite((char[]) {4, 0, 16, 0}, 1, 4, file);
        fwrite("data\0\0\0\0", 1, 4, file);

        fseek(data, introOffset, SEEK_SET);  // Intro
        for (long i = 0; i < introSize; i++) {
            fread(temp, 1, 1, data);
            fwrite(temp, 1, 1, file);
        }

        for (int i = 0; i < LOOP; i++) {  // Loop
            fseek(data, loopOffset, SEEK_SET);
            for (long i = 0; i < loopSize; i++) {
                fread(temp, 1, 1, data);
                fwrite(temp, 1, 1, file);
            }
        }

        fclose(file);

        file = fopen(fileName, "rb+");
        fseek(file, 0x28, SEEK_SET);
        dataSize = introSize + LOOP * loopSize;
        fwrite(&dataSize, 4, 1, file);  // Data size
        fseek(file, 0x04, SEEK_SET);
        fileSize = dataSize + 36;  // Total file size - 8 bytes
        fwrite(&fileSize, 4, 1, file);  // File size

        fclose(file);

        printf("Done.\n\n");
    }

    fclose(data);
    fclose(info);

    printf("All done.\nPress any key to continue...");
    while (getchar() != '\n');
    getchar();

    free(fileName);
    free(temp);

    return 0;
}