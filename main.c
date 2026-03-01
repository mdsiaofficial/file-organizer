#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH 512
#define MAX_EXT 16

typedef struct {
  char *ext;
  char *folder;
} Rule;

Rule rules[] = {
    {".txt", "Documents"},  {".pdf", "Documents"}, {".doc", "Documents"},
    {".docx", "Documents"}, {".jpg", "Images"},    {".png", "Images"},
    {".gif", "Images"},     {".mp4", "Videos"},    {".mkv", "Videos"},
    {".mp3", "Music"},      {".wav", "Music"},     {".zip", "Archives"},
    {".tar", "Archives"},   {".gz", "Archives"},   {NULL, NULL}};

char *get_extension(const char *filename) {
  char *dot = strrchr(filename, '.');
  if (!dot || dot == filename) {
    return NULL;
  }
  return dot;
}

char *find_folder(const char *ext) {
  if (!ext) {
    return NULL;
  }
  for (int i = 0; rules[i].ext != NULL; i++) {
    if (strcasecmp(ext, rules[i].ext) == 0) {
      return rules[i].folder;
    }
  }
  return "Others";
}

int create_dir(const char *path) {
  struct stat st = {0};
  if (stat(path, &st) == -1) {
    return mkdir(path, 0755);
  }
  return 0;
}

int move_file(const char *src, const char *dest) {
  if (!src || !dest) {
    return -1;
  }
  return rename(src, dest);
}

void cleanup_path(char *path) {
  if (!path) {
    return;
  }
  size_t len = strlen(path);
  if (len > 0 && path[len - 1] == '/') {
    path[len - 1] = '\0';
  }
}

int organize_directory(const char *target_dir) {
  if (!target_dir) {
    fprintf(stderr, "Error: null directory path\n");
    return -1;
  }

  DIR *dir = opendir(target_dir);
  if (!dir) {
    perror("Failed to open directory");
    return -1;
  }

  char work_dir[MAX_PATH];
  strncpy(work_dir, target_dir, MAX_PATH - 1);
  work_dir[MAX_PATH - 1] = '\0';
  cleanup_path(work_dir);

  struct dirent *entry;
  int moved = 0, failed = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char full_path[MAX_PATH];
    snprintf(full_path, MAX_PATH, "%s/%s", work_dir, entry->d_name);

    struct stat file_stat;
    if (stat(full_path, &file_stat) == -1 || S_ISDIR(file_stat.st_mode)) {
      continue;
    }

    char *ext = get_extension(entry->d_name);
    char *folder = find_folder(ext);

    if (!folder) {
      failed++;
      continue;
    }

    char dest_folder[MAX_PATH];
    snprintf(dest_folder, MAX_PATH, "%s/%s", work_dir, folder);

    if (create_dir(dest_folder) == -1) {
      fprintf(stderr, "Failed!\n");
      failed++;
      continue;
    }

    char dest_path[MAX_PATH];
    snprintf(dest_path, MAX_PATH, "%s/%s", dest_folder, entry->d_name);

    if (move_file(full_path, dest_path) == 0) {
      printf("Moved: %s -> %s/\n", entry->d_name, folder);
      moved++;
    } else {
      fprintf(stderr, "Failed to move: %s\n", entry->d_name);
      failed++;
    }
  }

  closedir(dir);
  printf("\nDone! %d moved, %d failed\n", moved, failed);
  return 0;
}

int main(int argc, char *argv[]) {
  const char *target = (argc > 1) ? argv[1] : ".";

  printf("Organizing files in: %s\n\n", target);

  if (organize_directory(target) != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
