#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
      printf("failed to open file for reading\n");
      return -1;
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

  //    create array of items large enough to hold all indexed items
  //
  struct item** out_tmp = malloc(indexcount * sizeof(struct item*));

  //    indexcount times
  //
  for (size_t i = 0; i < indexcount; i++)
    {
      //    read 17 bytes from index[0+offset] in to temporary string
      //
      struct item* tmp_item = malloc(sizeof(struct item));
      uint8_t tmp[17] = {0};
      memcpy(tmp, &index[0 + (i * 17)], 17);
      memcpy(tmp_item->filename, &tmp[0], 13);
      memcpy(&tmp_item->offset, &tmp[13], 4);
      out_tmp[i] = tmp_item;
    }

  //    calculate the file lengths
  //
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
      printf("failed to open file for reading\n");
      exit(EXIT_FAILURE);
    }

  struct item** items = 0;
  uint32_t itemc = 0;
  if ((itemc = analyse(filename, &items)) > 0)
    {
      for (size_t i = 0; i < itemc - 1; i++)
        {
          printf("Extracting: %s\n", items[i]->filename);
          char* outpath = malloc(255);
          memset(outpath, 0, 255);
          sprintf(outpath, "%s/%s", destination, items[i]->filename);
          fflush(stdout);

          FILE* out = fopen(outpath, "wb");
          if (!out)
            {
              printf("failed to open file for writing\n");
              exit(EXIT_FAILURE);
            }

          fseek(src, items[i]->offset, SEEK_SET);
          uint8_t* buffer = malloc(items[i]->len);
          fread(buffer, 1, items[i]->len, src);
          fwrite(buffer, 1, items[i]->len, out);

          free(outpath);
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
  printf("Usage: %s a|e /path/to/file.fil\n\n", path);
  printf("CAUTION: This will extract the .fil to the current directory.\n");
  printf("Use a for analyse.\n");
  printf("Use e for extract.\n\n");
}

int32_t
main(int32_t argc, const char* argv[])
{
  if (argc == 3)
    {
      if (argv[1][0] == 'e')
        {
          extract(argv[2], "./");
        }
      else if (argv[1][0] == 'a')
        {
          struct item** items = 0;
          uint32_t itemc = analyse(argv[2], &items);
          for (size_t i = 0; i < itemc - 1; i++)
            {
              printf("%s ", items[i]->filename);
              printf("%d\n", items[i]->len);
            }
          tidyup(&items, itemc);
        }
      else
        {
          printf("unrecognised action\n");
        }
    }
  else
    {
      usage(argv[0]);
    }

  return 0;
}
