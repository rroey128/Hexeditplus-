#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_mode;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

struct fun_desc
{
  char *name;
  void (*fun)(state *s);
};

// static char *hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
// static char *dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

void togDebug(state *s)
{
  if (s->debug_mode)
  {
    s->debug_mode = 0;
    fprintf(stderr, "Debug flag now off\n");
  }
  else
  {
    s->debug_mode = 1;
    fprintf(stderr, "Debug flag now on\n");
  }
}

void setFileName(state *s)
{
  fgets(s->file_name, sizeof(s->file_name), stdin);
  s->file_name[strcspn(s->file_name, "\n")] = '\0';
  if (s->debug_mode)
  {
    fprintf(stderr, "Debug: file name set to %s\n", s->file_name);
  }
}

void setUnitSize(state *s)
{
  int size;
  int n = 10;
  char input[n];
  fgets(input, sizeof(input), stdin);
  size = atoi(input);

  if (size == 1 || size == 2 || size == 4)
  {
    s->unit_size = size;

    if (s->debug_mode)
    {
      fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
    }
  }
  else
  {
    printf("Invalid size. Size remains unchanged.\n");
  }
}

void loadIntoMem(state *s)
{
  if (strcmp(s->file_name, "") == 0)
  {
    printf("Error: file name is empty.\n");
    return;
  }

  FILE *file = fopen(s->file_name, "rb");
  if (file == NULL)
  {
    printf("Error: failed to open file '%s'.\n", s->file_name);
    return;
  }

  char input[30];
  printf("Please enter <location> <length>: ");
  fgets(input, sizeof(input), stdin);

  unsigned int location;
  unsigned int length;

  sscanf(input, "%x %u", &location, &length);

  if (s->debug_mode)
  {
    fprintf(stderr, "Debug: Loading file '%s', location: %X, length: %u.\n", s->file_name, location, length);
  }

  fseek(file, location, SEEK_SET);
  size_t read_count = fread(s->mem_buf, s->unit_size, length, file);
  fclose(file);

  s->mem_count = read_count;

  printf("Loaded %u units into memory.\n", length);
}

void togDisplay(state *s)
{
  if (s->display_mode)
  {
    s->display_mode = 0;
    printf("Display flag now off, decimal representation\n");
  }
  else
  {
    s->display_mode = 1;
    printf("Display flag now on, hexadecimal representation\n");
  }
}

void memDisplay(state *s)
{
  unsigned int addr;
  int u;
  char input[30];
  printf("Enter address and length:\n");
  fgets(input, sizeof(input), stdin);
  sscanf(input, "%x %d", &addr, &u);

  if (s->display_mode)
  {
    printf("Hexadecimal\n===========\n");
    unsigned char *ptr = s->mem_buf + addr;
    for (size_t i = 0; i < u * s->unit_size; i += s->unit_size)
    {
      // Get the hexadecimal value
      unsigned int value = 0;
      for (int j = 0; j < s->unit_size; j++)
      {
        value |= ptr[i + j] << (8 * j);
      }

      // Print the hexadecimal value with a fixed width of 2*s->unit_size characters
      printf("%0*X\n", 2 * s->unit_size, value);
    }
  }
  else
  {
    printf("Decimal\n=======\n");
    unsigned char *ptr = s->mem_buf + addr;
    for (size_t i = 0; i < u * s->unit_size; i += s->unit_size)
    {
      unsigned int value = 0;
      for (int j = 0; j < s->unit_size; j++)
      {
        value |= (unsigned int)ptr[i + j] << (8 * j);
      }
      printf("%-5u\n", value);
    }
  }
}

void saveIntoFile(state *s)
{
  if (strcmp(s->file_name, "") == 0)
  {
    printf("Error: file name is empty.\n");
    return;
  }

  FILE *file = fopen(s->file_name, "rb+");
  if (file == NULL)
  {
    printf("Error: failed to open file '%s'.\n", s->file_name);
    return;
  }

  char input[30];
  printf("Please enter <source-address> <target-location> <length>: ");
  fgets(input, sizeof(input), stdin);

  unsigned char *source_address;
  unsigned int target_location;
  unsigned int length;

  sscanf(input, "%x %x %u", (unsigned int*)&source_address, &target_location, &length);

  if (source_address == 0) source_address = s->mem_buf;

  printf("source_address: %u\n", *source_address);
  if (s->debug_mode)
  {
    fprintf(stderr, "Debug: Saving to file '%s', source address: %p, target location: %X, length: %u.\n", s->file_name, source_address, target_location, length);
  }

  // Check if target location is greater than the size of the file
  fseek(file, 0, SEEK_END);

  long file_size = ftell(file);

  if (target_location > file_size)
  {
    printf("Error: target location exceeds file size.\n");
    fclose(file);
    return;
  }

  fseek(file, target_location, SEEK_SET);
  size_t write_count = fwrite(source_address, s->unit_size, length, file);

  fclose(file);

  printf("Saved %zu units into file.\n", write_count);
}


void memModify(state *s)
{
  char input[30];
  printf("Please enter <location> <val>\n");
  fgets(input, sizeof(input), stdin);

  unsigned int location;
  unsigned int val;

  sscanf(input, "%x %x", &location, &val);

  if (s->debug_mode)
  {
    printf("Location: 0x%X\n", location);
    printf("Value: 0x%X\n", val);
  }

  unsigned char *memory_ptr = s->mem_buf + location;
  for (int i = 0; i < s->unit_size; i++)
  {
    memory_ptr[i] = (val >> (i * 8)) & 0xFF;
  }
  printf("Memory modified successfully.\n");
}

void quit(state *s)
{
  if (s->debug_mode)
  {
    fprintf(stderr, "quitting\n");
  }
  if (s)
    free(s);
  exit(0);
}

struct fun_desc menu[] = {{"Toggle Debug Mode", togDebug}, {"Set File Name", setFileName}, {"Set Unit Size", setUnitSize}, {"Load Into Memory", loadIntoMem}, {"Toggle Display Mode", togDisplay}, {"Memory Display", memDisplay}, {"Save Into File", saveIntoFile}, {"Memory Modify", memModify}, {"Quit", quit}, {NULL, NULL}};

void printMenu(struct fun_desc menu[9])
{
  for (int i = 0; i < 9; i++)
  {
    printf("(%d) %s\n", i, menu[i].name);
  }
}

void printValues(state *s)
{
  if (s->debug_mode)
  {
    fprintf(stderr, "unit_size: %d\n", s->unit_size);
    fprintf(stderr, "file_name: %s\n", s->file_name);
    fprintf(stderr, "mem_count: %zu\n", s->mem_count);

    // printf("mem_buf in func: ");
    // for (size_t i = 0; i < s->mem_count; i++)
    // {
    //   printf(hex_formats[s->unit_size - 1], s->mem_buf[i]);
    // }
    // printf("\n");
  }
}

int main(int argc, char **argv)
{
  printf("Select operation from the following menu:\n");
  state *s = malloc(sizeof(state));
  s->debug_mode = 0;
  int n = 10;
  char input[n];
  int menu_len = sizeof(menu) / 8 - 1;
  printValues(s);
  printMenu(menu);
  while (fgets(input, n, stdin))
  {
    if ((0 <= input[0] - '0') & (input[0] - '0' < menu_len))
    {
      printf("Within bounds\n");
    }
    else
    {
      printf("Not within bounds\n");
      break;
    }
    menu[input[0] - '0'].fun(s);
    printValues(s);
    printMenu(menu);
  }
  if (s)
    free(s);
}