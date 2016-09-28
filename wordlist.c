#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define OUTPUT_FILE             "dictionary.txt"
#define WORDS_DELIMITERS        " \n"
#define MAX_PATH_LENGTH         128
#define MAX_WORD_LENGTH         64

void retrieve_file_words(const char *filename, FILE *output) {
  FILE *fp;
  char buffer[MAX_WORD_LENGTH];
  char *word_ptr;

  if((fp = fopen(filename, "r")) == NULL) {
    perror("fopen");
    return;
  }

  while(fgets(buffer, sizeof buffer, fp) != NULL) {
    word_ptr = strtok(buffer, WORDS_DELIMITERS);
    fprintf(output, "%s\n", word_ptr);

    while((word_ptr = strtok(NULL, WORDS_DELIMITERS)) != NULL) {
      fprintf(output, "%s\n", word_ptr);
    }
  }
}

void scan_directory_words(char *path, FILE *output) {
  DIR *directory;
  char new_path[MAX_PATH_LENGTH];
  struct dirent *ent;
  struct stat ent_stat;

  if((directory = opendir(path)) != NULL) {
    while((ent = readdir(directory)) != NULL) {
      if(ent->d_name[0] != '.') {
        snprintf(new_path, sizeof new_path, "%s/%s", path, ent->d_name);

        if(stat(new_path, &ent_stat) == -1) {
          perror("stat");
          return;
        }

        if((ent_stat.st_mode & S_IFMT) == S_IFDIR) {
          scan_directory_words(new_path, output); 
        } else {
          retrieve_file_words(new_path, output);
        }
      }
    }

    closedir(directory);
  }
}

int main(int argc, char *argv[]) {
  FILE *fp;

  if(argc < 2) {
    fprintf(stdout, "Use: %s [directory]\n", argv[0]);
    exit(0);
  }

  if((fp = fopen(OUTPUT_FILE, "w")) != NULL) {
    scan_directory_words(argv[1], fp);
    fclose(fp);
  }

  return 0;
}
