/*
 * wordlist - Dictionary Generator
 *
 * Copyright (C) 2016  Rafael Ravedutti Lucio Machado
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define OUTPUT_FILE             "dicionario.txt"
#define WORD_DELIMITERS        " ,\n"
#define MAX_PATH_LENGTH         128
#define MAX_WORD_LENGTH         128
#define WORD_HASH_SIZE          38

/* Word Operations */
#define WORD_LIST_INSERT        1
#define WORD_LIST_FREE          2
#define WORD_LIST_WRITE         3

struct word_list {
  char wl_word[MAX_WORD_LENGTH];
  struct word_list *wl_next;
};

void wordlistctl(int op, const char *word) {
  FILE *fp;
  static struct word_list *wl[WORD_HASH_SIZE];
  static int initialized = 0;
  struct word_list *p, *q;
  unsigned int h, i, len;

  if(initialized == 0) {
    for(i = 0; i < WORD_HASH_SIZE; ++i) {
      wl[i] = NULL;
    }

    initialized = 1;
  }

  if(op == WORD_LIST_INSERT) {
    if(word == NULL) {
      return;
    }

    len = strlen(word);

    if(len > MAX_WORD_LENGTH) {
      len = MAX_WORD_LENGTH;
    }

    h = (word[0] >= 'a' && word[0] <= 'z') ? (word[0] - 'a') :
        (word[0] >= 'A' && word[0] <= 'Z') ? (word[0] - 'A') :
        (word[0] >= '0' && word[0] <= '9') ? (word[0] - '0' + 'z' - 'a') : (WORD_HASH_SIZE - 1);

    if(wl[h] == NULL) {
      wl[h] = (struct word_list *) malloc(sizeof(struct word_list));
      wl[h]->wl_next = NULL;
      strncpy(wl[h]->wl_word, word, len);
    } else {
      for(p = wl[h]; p != NULL; p = p->wl_next) {
        if(strncmp(p->wl_word, word, len) == 0) {
          return;
        }
      }

      p = (struct word_list *) malloc(sizeof(struct word_list));
      p->wl_next = wl[h];
      strncpy(p->wl_word, word, len);
      wl[h] = p;
    }
  } else if(op == WORD_LIST_FREE) {
    for(i = 0; i < WORD_HASH_SIZE; ++i) {
      for(p = wl[i], q = NULL; p != NULL; q = p, p = p->wl_next) {
        if(q != NULL) {
          free(q);
        }
      }

      if(q != NULL) {
        free(q);
      }

      wl[i] = NULL;
    }
  } else if(op == WORD_LIST_WRITE) {
    if((fp = fopen(word, "w")) != NULL) {
      for(i = 0; i < WORD_HASH_SIZE; ++i) {
        for(p = wl[i]; p != NULL; p = p->wl_next) {
          fprintf(fp, "%s\n", p->wl_word);
        }
      }

      fclose(fp);
    }
  }
}

void retrieve_file_words(const char *filename) {
  FILE *fp;
  char buffer[MAX_WORD_LENGTH];
  char *word_ptr;

  if((fp = fopen(filename, "r")) == NULL) {
    perror("fopen");
    return;
  }

  while(fgets(buffer, sizeof buffer, fp) != NULL) {
    word_ptr = strtok(buffer, WORD_DELIMITERS);
    wordlistctl(WORD_LIST_INSERT, word_ptr);

    while((word_ptr = strtok(NULL, WORD_DELIMITERS)) != NULL) {
      wordlistctl(WORD_LIST_INSERT, word_ptr);
    }
  }

  fclose(fp);
}

void scan_directory_words(char *path) {
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
          scan_directory_words(new_path); 
        } else {
          retrieve_file_words(new_path);
        }
      }
    }

    closedir(directory);
  }
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stdout, "Use: %s <directory>\n", argv[0]);
    exit(0);
  }

  scan_directory_words(argv[1]);
  wordlistctl(WORD_LIST_WRITE, OUTPUT_FILE);
  wordlistctl(WORD_LIST_FREE, NULL);
  return 0;
}
