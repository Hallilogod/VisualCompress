#define _FILE_OFFSET_BITS 64
#include "lodepng.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define UNINIT_PTR ((unsigned char *)0xCD)


typedef struct _FACTORS
{
    unsigned int factor1;
    unsigned int factor2;
} FACTORS, *PFACTORS;

void FindClosestFactors(unsigned long long number, PFACTORS pFactors)
{
    unsigned long long factor2 = floor(sqrt(number));

    while (factor2 > 1 && number % factor2 != 0)
    {
        factor2--;
    }

    pFactors->factor1 = (unsigned int)number / factor2;

    pFactors->factor2 = (unsigned int)factor2;
}

unsigned char *ReadFileToBuffer(char *fileName, off_t *pBufferSizeBytes)
{
    FILE *inputFile = fopen(fileName, "rb");

    if (inputFile == NULL)
    {
        printf("Error opening file '%s'\n", fileName);
        return NULL;
    }

    fseek(inputFile, 0, SEEK_END);

    off_t inputFileSize = ftello(inputFile);

    rewind(inputFile);

    unsigned char *inputFileBuffer = malloc(inputFileSize);

    if (inputFileBuffer == NULL)
    {
        printf("Failed to allocate %lld bytes of memory\n", inputFileSize);

        fclose(inputFile);
        free(inputFileBuffer);

        return NULL;
    }

    if (fread(inputFileBuffer, 1, inputFileSize, inputFile) < (size_t)inputFileSize)
    {
        printf("Read less bytes than file size\n");

        fclose(inputFile);
        free(inputFileBuffer);
        return NULL;
    }

    fclose(inputFile);

    *pBufferSizeBytes = inputFileSize;

    return inputFileBuffer;
}


int main(int argc, char **argv)
{

    off_t inputDataBufferSizeBytes = 0xCD;
    unsigned char *inputDataBuffer = UNINIT_PTR;
    bool decode = 0;

    if (argc < 3)
    {
        printf("Usage: %s [encode|decode] <file> (<outputfile>)", *argv);

        return 0;
    }

    if (strcmp(argv[1], "encode") == 0)
    {
        decode = 0;
    }
    else if (strcmp(argv[1], "decode") == 0)
    {
        decode = 1;
    }
    else
    {
        printf("Invalid operation '%s'", argv[1]);

        return 1;
    }

    char *outputFileName = argc > 3 ? argv[3] : (decode ? "out.bin" : "out.png");

    inputDataBuffer = ReadFileToBuffer(argv[2], &inputDataBufferSizeBytes);

    if (inputDataBuffer == NULL)
    {
        printf("Failed to read file '%s' into buffer", argv[2]);
        return 1;
    }

    if(inputDataBufferSizeBytes == 0)
    {
        printf("File '%s' is empty", argv[2]);

        free(inputDataBuffer);
        return 1;
    }

    printf("Read %zu bytes from '%s' into memory\n", inputDataBufferSizeBytes, argv[2]);

    if (decode)
    {
        unsigned int width, height = 0xCD;

        unsigned char *decodedImageBuffer = UNINIT_PTR;

        if (lodepng_decode24(&decodedImageBuffer, &width, &height, inputDataBuffer, (size_t)inputDataBufferSizeBytes))
        {
            printf("Error while decoding '%s'", argv[2]);

            free(inputDataBuffer);

            return 1;
        }

        FILE *outputFile = fopen(outputFileName, "wb");

        if (outputFile == NULL)
        {
            printf("Failed to open '%s'", outputFileName);

            free(decodedImageBuffer);
            free(inputDataBuffer);

            return 1;
        }
        
        // Figure out the size of the padding bytes (stored in the last pixel's last byte) to subtract it and the metadata pixel from the actual data
        unsigned int paddingSize = 3 + decodedImageBuffer[width * height * 3 - 1];
        

        fwrite(decodedImageBuffer, 1, width * height * 3 - paddingSize, outputFile);

        fclose(outputFile);

        free(decodedImageBuffer);

        printf("Decoded '%s' to '%s'\n", argv[2], outputFileName);
    }
    else
    {
        FACTORS inputFileSizeFactors = {0xCD, 0xCD};
        unsigned int paddingBytesCount = 0xCD;

        switch (inputDataBufferSizeBytes % 3)
        {
        case 0:
            paddingBytesCount = 0;
        break;
        case 1:
            paddingBytesCount = 2;
        break;
        case 2:
            paddingBytesCount = 1;
        break;

        default:
            assert(0);
        break;
        }


        unsigned char* newInputDataBuffer = realloc(inputDataBuffer, inputDataBufferSizeBytes + paddingBytesCount + 3);

        if(newInputDataBuffer == NULL)
        {
            printf("Failed to realloc input buffer");
            free(inputDataBuffer);

            return 1;
        }

        inputDataBuffer = newInputDataBuffer;

        // Zero out the padding and metadata info
        memset(&inputDataBuffer[inputDataBufferSizeBytes], 0x00, paddingBytesCount + 3);

        // Update the data buffer size so it includes the padding and metadata
        inputDataBufferSizeBytes += paddingBytesCount + 3;

        // Finally set the last value of the last pixel to the number of padding bytes so it can later be extracted 
        inputDataBuffer[inputDataBufferSizeBytes - 1] = paddingBytesCount;

        // We search the closest factors of the pixel count so we can set the image ratio to be as close to 1:1 as possible
        FindClosestFactors((unsigned long long)inputDataBufferSizeBytes / 3, &inputFileSizeFactors);

        if (lodepng_encode24_file(outputFileName, inputDataBuffer, inputFileSizeFactors.factor1, inputFileSizeFactors.factor2))
        {
            printf("Error while encoding '%s'", argv[2]);

            free(inputDataBuffer);
            return 1;
        }

        printf("Encoded '%s' to '%s'\n", argv[2], outputFileName);
    }

    free(inputDataBuffer);

    return 0;
}
