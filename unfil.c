#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* g_fil_path = 0;

enum e_options
{
  OPTION_EXTRACT = 0,
  OPTION_LIST = 'l'
};
typedef enum e_options e_options;

typedef struct item item;
struct item
{
  char filename[13];
  uint32_t offset;
  uint32_t len;
};

void
tidyup(struct item*** items, size_t itemcount)
{
  for (size_t i = 0; i < itemcount; i++)
    {
      free((*items)[i]);
    }
  free(*items);
}

uint32_t
analyse(const char* filename, struct item*** output)
{
  // Set up FD
  FILE* fd = fopen(filename, "rb");

  if (!fd)
    {
      fputs("ERROR: Failed to open file for reading\n", stderr);
      exit(EXIT_FAILURE);
    }

  // Read and decipher the indexcount
  uint32_t indexcount = 0;
  fread(&indexcount, 4, 1, fd);
  indexcount ^= 0x3BD7A59A;

  // Read the enctypted index
  uint8_t* indexe = malloc(indexcount * 17);
  fread(indexe, 17, indexcount, fd);

  // Decrypt the index
  uint8_t* index = malloc(indexcount * 17);
  for (size_t b = 0; b < indexcount * 17; b++)
    {
      uint8_t tmp = indexe[b];
      tmp -= 39;
      tmp ^= 0xA5;
      tmp -= (27 + b);
      index[b] = tmp;
    }

  // create array of items large enough to hold all indexed items
  struct item** out_tmp = malloc(indexcount * sizeof(struct item*));

  // indexcount times
  for (size_t i = 0; i < indexcount; i++)
    {
      // read 17 bytes from index[0+offset] in to temporary string
      struct item* tmp_item = malloc(sizeof(struct item));
      uint8_t tmp[17] = {0};
      memcpy(tmp, &index[0 + (i * 17)], 17);
      memcpy(tmp_item->filename, &tmp[0], 13);
      memcpy(&tmp_item->offset, &tmp[13], 4);
      out_tmp[i] = tmp_item;
    }

  //    calculate the file lengths
  for (size_t i = 0; i < indexcount - 1; i++)
    {
      out_tmp[i]->len = out_tmp[i + 1]->offset - out_tmp[i]->offset - 1;
    }

  free(index);
  free(indexe);
  fclose(fd);
  (*output) = out_tmp;
  return indexcount;
}

void
extract(const char* filename, char* destination)
{
  FILE* src = fopen(filename, "rb");

  if (!src)
    {
      fputs("ERROR: Failed to open file for reading\n", stderr);
      exit(EXIT_FAILURE);
    }

  struct item** items = 0;
  uint32_t itemc = 0;
  if ((itemc = analyse(filename, &items)) > 0)
    {
      for (size_t i = 0; i < itemc - 1; i++)
        {
          char outpath[255] = {0};
          sprintf(outpath, "%s/%s", destination, items[i]->filename);
          fflush(stdout);

          FILE* out = fopen(outpath, "wb");
          if (!out)
            {
              fputs("ERROR: Failed to open file for writing\n", stderr);
              exit(EXIT_FAILURE);
            }

          fseek(src, items[i]->offset, SEEK_SET);
          uint8_t* buffer = malloc(items[i]->len);
          fread(buffer, 1, items[i]->len, src);
          fwrite(buffer, 1, items[i]->len, out);

          free(buffer);
          fclose(out);
        }
    }

  tidyup(&items, itemc);
  fclose(src);
}

void
usage(const char* path)
{
  printf("Usage: %s [OPTION] [FILE]\n\n", path);
  fputs("Options:\n", stdout);
  fputs("  -l    list files without extracting them\n\n", stdout);
  fputs("NOTE: this tool extracts the content of the FIL file in the current directory.\n", stdout);
}

e_options
parse_arguments(int32_t argc, const char* argv[])
{
  e_options result = 0;
  for (size_t i = 1; i < (size_t)argc; ++i)
    {
      if (argv[i][0] == '-')
        {
          result = argv[i][1];
        }
      else
        {
          g_fil_path = argv[i];
        }
    }
  return result;
}

int32_t
main(int32_t argc, const char* argv[])
{
  if (argc == 2 || argc == 3)
    {
      e_options option = parse_arguments(argc, argv);
      switch (option)
        {
        case OPTION_EXTRACT:
          extract(g_fil_path, "./");
          return EXIT_SUCCESS;
        case OPTION_LIST:
          struct item** items = 0;
          uint32_t itemc = analyse(argv[2], &items);
          for (size_t i = 0; i < itemc - 1; i++)
            {
              printf("%*u %s\n", 8, items[i]->len, items[i]->filename);
            }
          tidyup(&items, itemc);
          return EXIT_SUCCESS;
        default:
          usage(argv[0]);
        }
    }
  usage(argv[0]);

  return EXIT_SUCCESS;
}