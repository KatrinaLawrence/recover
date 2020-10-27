#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t BYTE;

int main(int argc, char *argv[])
{
    // check if program was executed with 1 cmd line argument
    if (argc != 2)
    {
        printf("Usage: ./recover image\n");
        return 1;
    }

    // check if file was able to be opened for reading
    FILE *cardptr = fopen(argv[1], "r");
    if (cardptr == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }

    //allocate memory for the jpeg data
    int blocksize = 512;
    BYTE *jpgdata = malloc(blocksize);
    if (jpgdata == NULL)
    {
        printf("Not enough memory.\n");
        return 1;
    }

    // set up the naming of files
    int filenumber = 0;
    char *filename = malloc(8 * (sizeof(char)));
    FILE *outptr = NULL;

    // read the memory card searching for jpegs
    while (fread(jpgdata, sizeof(BYTE), blocksize, cardptr) == 512)
    {
        for (int i = 0; i < blocksize; i++)
        {
            // locate a jpeg file
            if (jpgdata[i] == 0xff && jpgdata[i + 1] == 0xd8 && jpgdata[i + 2] == 0xff && (jpgdata[i + 3] & 0xf0) == 0xe0)
            {
                // found the first jpeg on card
                if (filenumber == 0)
                {
                    // Open a new file to write the jpeg data to
                    sprintf(filename, "%03i.jpg", filenumber);
                    outptr = fopen(filename, "a");

                    // Move through the rest of the file saving all data to the new jpeg file
                    for (int j = i; j < blocksize; j++)
                    {
                        fwrite(&jpgdata[j], sizeof(BYTE), 1, outptr);
                    }

                    filenumber++;

                    // all data in the block has been copied. exit the loop and start searching next block
                    break;
                }

                // found another jpeg on card
                else if (filenumber >= 1)
                {
                    // close the previous jpeg file and open a new jpeg file
                    fclose(outptr);
                    sprintf(filename, "%03i.jpg", filenumber);

                    outptr = fopen(filename, "a");
                    if (outptr == NULL)
                    {
                        free(jpgdata);
                        fclose(cardptr);
                        printf("Could not create %s.\n", filename);
                        return 1;
                    }

                    //start writing jpeg data to this new file
                    for (int j = i; j < blocksize; j++)
                    {
                        fwrite(&jpgdata[j], sizeof(BYTE), 1, outptr);
                    }

                    filenumber++;
                    // all data in the block has been copied. exit the loop and start searching next block
                    break;
                }

            }

            // jpeg was not found on this block
            else
            {
                // we have already found a jpeg, keep writing data to the currently open file
                if (filenumber != 0)
                {

                    fwrite(&jpgdata[i], sizeof(BYTE), 1, outptr);

                }

                // we haven't found a jpeg yet, continue searching for the first jpeg
                else if (filenumber == 0)
                {
                    continue;
                }
            }
        }
    }

    free(jpgdata);
    free(filename);
    fclose(cardptr);
    fclose(outptr);
}
