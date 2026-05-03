#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Limit the number of unique destination folders tracked in memory.
// This keeps the per-run cache small and predictable.
#define MAX_CREATED_FOLDERS 32

typedef struct
{
  const char *ext;
  const char *folder;
} Rule;

// Extension-to-folder map used by the organizer.
// The first matching rule wins, and unmatched files fall back to "others".
static const Rule rules[] = {
    // Documents and text files.
    {".txt", "documents"},
    {".md", "documents"},
    {".markdown", "documents"},
    {".pdf", "documents"},
    {".doc", "documents"},
    {".docx", "documents"},
    {".docm", "documents"},
    {".odt", "documents"},
    {".rtf", "documents"},
    {".tex", "documents"},
    {".epub", "documents"},
    {".mobi", "documents"},
    {".azw", "documents"},
    {".azw3", "documents"},
    {".fb2", "documents"},
    {".djvu", "documents"},
    {".xps", "documents"},
    {".pages", "documents"},
    {".srt", "documents"},
    {".ass", "documents"},
    {".ssa", "documents"},
    {".vtt", "documents"},
    {".sub", "documents"},

    // Spreadsheet files.
    {".csv", "spreadsheets"},
    {".tsv", "spreadsheets"},
    {".xls", "spreadsheets"},
    {".xlsx", "spreadsheets"},
    {".xlsm", "spreadsheets"},
    {".ods", "spreadsheets"},
    {".numbers", "spreadsheets"},

    // Presentation files.
    {".ppt", "presentations"},
    {".pptx", "presentations"},
    {".pptm", "presentations"},
    {".odp", "presentations"},
    {".key", "presentations"},

    // Image and photo files.
    {".jpg", "images"},
    {".jpeg", "images"},
    {".png", "images"},
    {".gif", "images"},
    {".bmp", "images"},
    {".tif", "images"},
    {".tiff", "images"},
    {".webp", "images"},
    {".svg", "images"},
    {".ico", "images"},
    {".heic", "images"},
    {".heif", "images"},
    {".raw", "images"},
    {".cr2", "images"},
    {".nef", "images"},
    {".arw", "images"},
    {".dng", "images"},

    // Audio files.
    {".mp3", "music"},
    {".wav", "music"},
    {".flac", "music"},
    {".aac", "music"},
    {".m4a", "music"},
    {".ogg", "music"},
    {".opus", "music"},
    {".wma", "music"},
    {".aiff", "music"},
    {".alac", "music"},
    {".mid", "music"},
    {".midi", "music"},

    // Video files.
    {".mp4", "videos"},
    {".mkv", "videos"},
    {".avi", "videos"},
    {".mov", "videos"},
    {".wmv", "videos"},
    {".flv", "videos"},
    {".webm", "videos"},
    {".m4v", "videos"},
    {".3gp", "videos"},
    {".mpeg", "videos"},
    {".mpg", "videos"},
    {".ts", "videos"},
    {".m2ts", "videos"},
    {".vob", "videos"},
    {".ogv", "videos"},

    // Archives and disk images.
    {".zip", "archives"},
    {".rar", "archives"},
    {".7z", "archives"},
    {".tar", "archives"},
    {".gz", "archives"},
    {".bz2", "archives"},
    {".xz", "archives"},
    {".tgz", "archives"},
    {".tbz2", "archives"},
    {".txz", "archives"},
    {".zst", "archives"},
    {".cab", "archives"},
    {".lz", "archives"},
    {".lzma", "archives"},
    {".iso", "archives"},

    // Executables and installers.
    {".bat", "programs"},
    {".cmd", "programs"},
    {".com", "programs"},
    {".exe", "programs"},
    {".msi", "programs"},
    {".apk", "programs"},
    {".app", "programs"},
    {".appimage", "programs"},
    {".dmg", "programs"},
    {".pkg", "programs"},
    {".deb", "programs"},
    {".rpm", "programs"},
    {".jar", "programs"},
    {".war", "programs"},
    {".ear", "programs"},
    {".bin", "programs"},
    {".run", "programs"},
    {".so", "programs"},
    {".dll", "programs"},
    {".dylib", "programs"},
    {".sys", "programs"},

    // Source code, build files, scripts, and web files.
    {".c", "code"},
    {".h", "code"},
    {".cpp", "code"},
    {".cc", "code"},
    {".cxx", "code"},
    {".hpp", "code"},
    {".hh", "code"},
    {".java", "code"},
    {".kt", "code"},
    {".kts", "code"},
    {".go", "code"},
    {".rs", "code"},
    {".cs", "code"},
    {".fs", "code"},
    {".swift", "code"},
    {".m", "code"},
    {".mm", "code"},
    {".vb", "code"},
    {".vbs", "code"},
    {".dart", "code"},
    {".scala", "code"},
    {".groovy", "code"},
    {".clj", "code"},
    {".cljs", "code"},
    {".lisp", "code"},
    {".el", "code"},
    {".hs", "code"},
    {".nim", "code"},
    {".zig", "code"},
    {".pas", "code"},
    {".erl", "code"},
    {".ex", "code"},
    {".exs", "code"},
    {".jl", "code"},
    {".r", "code"},
    {".vala", "code"},
    {".cmake", "code"},
    {".gradle", "code"},
    {".make", "code"},
    {".mk", "code"},
    {".sh", "code"},
    {".bash", "code"},
    {".zsh", "code"},
    {".fish", "code"},
    {".py", "code"},
    {".rb", "code"},
    {".php", "code"},
    {".pl", "code"},
    {".pm", "code"},
    {".lua", "code"},
    {".ps1", "code"},
    {".js", "code"},
    {".ts", "code"},
    {".mjs", "code"},
    {".cjs", "code"},
    {".jsx", "code"},
    {".tsx", "code"},
    {".sql", "code"},
    {".html", "code"},
    {".htm", "code"},
    {".xhtml", "code"},
    {".css", "code"},
    {".scss", "code"},
    {".sass", "code"},
    {".less", "code"},

    // Config and structured text files.
    {".json", "configs"},
    {".jsonl", "configs"},
    {".xml", "configs"},
    {".yaml", "configs"},
    {".yml", "configs"},
    {".toml", "configs"},
    {".ini", "configs"},
    {".cfg", "configs"},
    {".conf", "configs"},
    {".properties", "configs"},
    {".env", "configs"},
    {".local", "configs"},
    {".rc", "configs"},
    {".plist", "configs"},
    {".desktop", "configs"},

    // Data and database files.
    {".db", "data"},
    {".db3", "data"},
    {".sqlite", "data"},
    {".sqlite3", "data"},
    {".parquet", "data"},
    {".feather", "data"},
    {".orc", "data"},
    {".h5", "data"},
    {".hdf5", "data"},
    {".npy", "data"},
    {".npz", "data"},
    {".mat", "data"},
    {".pkl", "data"},
    {".pickle", "data"},
    {".sav", "data"},
    {".dat", "data"},

    // Design and creative assets.
    {".psd", "design"},
    {".ai", "design"},
    {".xd", "design"},
    {".fig", "design"},
    {".sketch", "design"},
    {".afdesign", "design"},
    {".indd", "design"},

    // 3D models and scene files.
    {".blend", "3d"},
    {".obj", "3d"},
    {".fbx", "3d"},
    {".stl", "3d"},
    {".dae", "3d"},
    {".glb", "3d"},
    {".gltf", "3d"},
    {".ply", "3d"},
    {".usdz", "3d"},
    {".3ds", "3d"},
    {".amf", "3d"},
    {".3mf", "3d"},

    // Fonts and certificates.
    {".ttf", "fonts"},
    {".otf", "fonts"},
    {".woff", "fonts"},
    {".woff2", "fonts"},
    {".eot", "fonts"},
    {".fon", "fonts"},
    {".pem", "certificates"},
    {".crt", "certificates"},
    {".cer", "certificates"},
    {".key", "certificates"},
    {".pfx", "certificates"},
    {".p12", "certificates"},
    {".der", "certificates"},

    // Logs and backup files.
    {".log", "logs"},
    {".out", "logs"},
    {".err", "logs"},
    {".trace", "logs"},
    {".bak", "backups"},
    {".backup", "backups"},
    {".old", "backups"},
    {".orig", "backups"},
    {".tmp", "backups"},
    {".swp", "backups"},
    {".swo", "backups"},

    {NULL, NULL}};

// Joins two path parts into a single buffer.
// Returns -1 when the output would not fit.
static int build_path(char *buffer, size_t buffer_size, const char *left,
                      const char *right)
{
  if (!buffer || !left || !right || buffer_size == 0)
  {
    return -1;
  }

  int written = snprintf(buffer, buffer_size, "%s/%s", left, right);

  // ! debug log to verify path building and buffer usage
  // printf("---Debug: build_path('%s', '%s') -> '%s' (written: %d)\n", left, right, buffer, written);

  if (written < 0 || (size_t)written >= buffer_size)
  {
    return -1;
  }

  return 0;
}

// Returns the extension, including the dot, or NULL when none exists.
static const char *get_extension(const char *filename)
{
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
  {
    return NULL;
  }
  return dot;
}

// Maps a file extension to its destination folder.
// Unknown extensions go to the "others" folder.
const char *find_folder(const char *ext)
{
  if (!ext)
  {
    return "others";
  }
  for (int i = 0; rules[i].ext != NULL; i++)
  {
    if (strcasecmp(ext, rules[i].ext) == 0)
    {
      return rules[i].folder;
    }
  }
  return "others";
}

// Creates a directory if it does not already exist.
// Fails when a non-directory item already occupies the path.
static int ensure_dir(const char *path)
{
  struct stat st;
  if (stat(path, &st) == 0)
  {
    return S_ISDIR(st.st_mode) ? 0 : -1;
  }

#ifdef _WIN32
  return mkdir(path);
#else
  return mkdir(path, 0755);
#endif
}

// Moves a file by renaming it into the target folder.
// This is fast because it does not copy file contents.
static int move_file(const char *src, const char *dest)
{
  if (!src || !dest)
  {
    return -1;
  }
  return rename(src, dest);
}

// Removes trailing '/' or '\\' so path joins stay consistent.
static void trim_trailing_separator(char *path)
{
  if (!path)
  {
    return;
  }
  size_t len = strlen(path);
  while (len > 0 && (path[len - 1] == '/' || path[len - 1] == '\\'))
  {
    path[--len] = '\0';
  }
}

// Checks whether the current directory entry is a regular file.
// Uses d_type when available and falls back to stat() otherwise.
static int is_regular_entry(const struct dirent *entry, const char *path)
{
  (void)entry;

#ifdef DT_DIR
  if (entry->d_type == DT_DIR)
  {
    return 0;
  }
#endif

#ifdef DT_REG
  if (entry->d_type == DT_REG)
  {
    return 1;
  }
#endif

  struct stat st;
  return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

// Creates each destination folder only once per run.
// This avoids repeated stat()/mkdir() calls for common categories.
static int ensure_folder_once(const char *folder, const char *folder_path,
                              const char *created_folders[],
                              size_t *created_count)
{
  for (size_t i = 0; i < *created_count; i++)
  {
    if (strcmp(created_folders[i], folder) == 0)
    {
      return 0;
    }
  }

  if (ensure_dir(folder_path) != 0)
  {
    return -1;
  }

  if (*created_count >= MAX_CREATED_FOLDERS)
  {
    return -1;
  }

  created_folders[*created_count] = folder;
  (*created_count)++;
  return 0;
}

// Scans the target directory and moves each regular file into a category.
// The function keeps subdirectories untouched.
int organize_directory(const char *target_dir)
{
  if (!target_dir)
  {
    fprintf(stderr, "Error: null directory path\n");
    return -1;
  }

  DIR *dir = opendir(target_dir);
  if (!dir)
  {
    perror("Failed to open directory");
    return -1;
  }

  char work_dir[PATH_MAX];
  int copied = snprintf(work_dir, sizeof(work_dir), "%s", target_dir);
  if (copied < 0 || (size_t)copied >= sizeof(work_dir))
  {
    fprintf(stderr, "Error: directory path is too long\n");
    closedir(dir);
    return -1;
  }
  trim_trailing_separator(work_dir);

  struct dirent *entry;
  int moved = 0, failed = 0;
  const char *created_folders[MAX_CREATED_FOLDERS];
  size_t created_count = 0;

  while ((entry = readdir(dir)) != NULL)
  {
    // Skip the current and parent directory entries.
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    // Build the full source path for the current entry.
    char source_path[PATH_MAX];
    if (build_path(source_path, sizeof(source_path), work_dir,
                   entry->d_name) != 0)
    {
      failed++;
      continue;
    }

    // Only move regular files, not directories or special entries.
    if (!is_regular_entry(entry, source_path))
    {
      continue;
    }

    // Resolve the folder name from the file extension.
    const char *ext = get_extension(entry->d_name);
    const char *folder = find_folder(ext);

    // Prepare the destination folder path.
    char dest_folder[PATH_MAX];
    if (build_path(dest_folder, sizeof(dest_folder), work_dir, folder) != 0)
    {
      failed++;
      continue;
    }

    if (ensure_folder_once(folder, dest_folder, created_folders,
                           &created_count) != 0)
    {
      fprintf(stderr, "Failed to prepare folder: %s\n", folder);
      failed++;
      continue;
    }

    // Build the final destination path inside the category folder.
    char dest_path[PATH_MAX];
    if (build_path(dest_path, sizeof(dest_path), dest_folder,
                   entry->d_name) != 0)
    {
      failed++;
      continue;
    }

    // rename() is used here because it is fast and avoids file copying.
    if (move_file(source_path, dest_path) == 0)
    {
      printf("Moved: %s -> %s/\n", entry->d_name, folder);
      moved++;
    }
    else
    {
      fprintf(stderr, "Failed to move: %s\n", entry->d_name);
      failed++;
    }
  }

  closedir(dir);
  printf("\nDone! %d moved, %d failed\n", moved, failed);
  return 0;
}

// Program entry point.
// Uses the provided directory path or the current directory by default.
int main(int argc, char *argv[])
{
  printf("Folderly - Simple File Organizer\n- %d", argc);
  const char *target = (argc > 1) ? argv[1] : ".";

  printf("Organizing files in: %s\n\n", target);

  if (organize_directory(target) != 0)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
