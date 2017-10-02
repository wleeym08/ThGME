#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define LOOP 2
#define EXTEND 20
#define FADEOUT 10

int extract(void);

int main() {
    int option;

    printf("+------------------------------------------------+\n"
        "|  Touhou General Music Extractor (ThGME) v0.10  |\n"
        "|                  By winghearn                  |\n"
        "+------------------------------------------------+\n\n"
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
    unsigned int introOffset;
    unsigned int introSize;
    unsigned int loopOffset;
    unsigned int loopSize;
    unsigned int totalSize;
    unsigned int dataSize;
    unsigned int fileSize;
    unsigned int infoSize;
    double volume;
    int extendSample = EXTEND * 44100 * 2;
    int fadeoutSample = FADEOUT * 44100 * 2;
    int fadeoutCount;
    short sample;

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
        for (unsigned int i = 0; i < introSize / 2; i++) {
            fread(&sample, 2, 1, data);
            fwrite(&sample, 2, 1, file);
        }

        for (unsigned int i = 0; i < LOOP; i++) {  // Loop
            fseek(data, loopOffset, SEEK_SET);
            for (long i = 0; i < loopSize / 2; i++) {
                fread(&sample, 2, 1, data);
                fwrite(&sample, 2, 1, file);
            }
        }

        fseek(data, loopOffset, SEEK_SET);  // Extend
        for (unsigned int i = 0; i < extendSample; i++) {
            fread(&sample, 2, 1, data);
            fwrite(&sample, 2, 1, file);
        }

        fadeoutCount = 0;
        volume = 1.0;
        
        for (unsigned int i = 0; i < fadeoutSample; i++) {  // Fade out
            fread(&sample, 2, 1, data);
            sample = (int)round(sample * volume);
            fwrite(&sample, 2, 1, file);
            fadeoutCount++;
            if (fadeoutCount == fadeoutSample / 1000) {
                if (volume > 0.0) 
                    volume -= 0.001;
                fadeoutCount = 0;
            }
        }

        fclose(file);

        file = fopen(fileName, "rb+");
        fseek(file, 0x28, SEEK_SET);
        dataSize = introSize + LOOP * loopSize + extendSample * 2 + fadeoutSample * 2;
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

    return 0;
}