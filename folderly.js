#!/usr/bin/env node
'use strict';

const fs = require('fs/promises');
const path = require('path');

// Extension-to-folder map.
// The first matching rule wins, and unmatched files go to "others".
const RULES = [
  // Documents and text files.
  { ext: '.txt', folder: 'documents' },
  { ext: '.md', folder: 'documents' },
  { ext: '.markdown', folder: 'documents' },
  { ext: '.pdf', folder: 'documents' },
  { ext: '.doc', folder: 'documents' },
  { ext: '.docx', folder: 'documents' },
  { ext: '.docm', folder: 'documents' },
  { ext: '.odt', folder: 'documents' },
  { ext: '.rtf', folder: 'documents' },
  { ext: '.tex', folder: 'documents' },
  { ext: '.epub', folder: 'documents' },
  { ext: '.mobi', folder: 'documents' },
  { ext: '.azw', folder: 'documents' },
  { ext: '.azw3', folder: 'documents' },
  { ext: '.fb2', folder: 'documents' },
  { ext: '.djvu', folder: 'documents' },
  { ext: '.xps', folder: 'documents' },
  { ext: '.pages', folder: 'documents' },
  { ext: '.srt', folder: 'documents' },
  { ext: '.ass', folder: 'documents' },
  { ext: '.ssa', folder: 'documents' },
  { ext: '.vtt', folder: 'documents' },
  { ext: '.sub', folder: 'documents' },

  // Spreadsheet files.
  { ext: '.csv', folder: 'spreadsheets' },
  { ext: '.tsv', folder: 'spreadsheets' },
  { ext: '.xls', folder: 'spreadsheets' },
  { ext: '.xlsx', folder: 'spreadsheets' },
  { ext: '.xlsm', folder: 'spreadsheets' },
  { ext: '.ods', folder: 'spreadsheets' },
  { ext: '.numbers', folder: 'spreadsheets' },

  // Presentation files.
  { ext: '.ppt', folder: 'presentations' },
  { ext: '.pptx', folder: 'presentations' },
  { ext: '.pptm', folder: 'presentations' },
  { ext: '.odp', folder: 'presentations' },
  { ext: '.key', folder: 'presentations' },

  // Image and photo files.
  { ext: '.jpg', folder: 'images' },
  { ext: '.jpeg', folder: 'images' },
  { ext: '.png', folder: 'images' },
  { ext: '.gif', folder: 'images' },
  { ext: '.bmp', folder: 'images' },
  { ext: '.tif', folder: 'images' },
  { ext: '.tiff', folder: 'images' },
  { ext: '.webp', folder: 'images' },
  { ext: '.svg', folder: 'images' },
  { ext: '.ico', folder: 'images' },
  { ext: '.heic', folder: 'images' },
  { ext: '.heif', folder: 'images' },
  { ext: '.raw', folder: 'images' },
  { ext: '.cr2', folder: 'images' },
  { ext: '.nef', folder: 'images' },
  { ext: '.arw', folder: 'images' },
  { ext: '.dng', folder: 'images' },

  // Audio files.
  { ext: '.mp3', folder: 'music' },
  { ext: '.wav', folder: 'music' },
  { ext: '.flac', folder: 'music' },
  { ext: '.aac', folder: 'music' },
  { ext: '.m4a', folder: 'music' },
  { ext: '.ogg', folder: 'music' },
  { ext: '.opus', folder: 'music' },
  { ext: '.wma', folder: 'music' },
  { ext: '.aiff', folder: 'music' },
  { ext: '.alac', folder: 'music' },
  { ext: '.mid', folder: 'music' },
  { ext: '.midi', folder: 'music' },

  // Video files.
  { ext: '.mp4', folder: 'videos' },
  { ext: '.mkv', folder: 'videos' },
  { ext: '.avi', folder: 'videos' },
  { ext: '.mov', folder: 'videos' },
  { ext: '.wmv', folder: 'videos' },
  { ext: '.flv', folder: 'videos' },
  { ext: '.webm', folder: 'videos' },
  { ext: '.m4v', folder: 'videos' },
  { ext: '.3gp', folder: 'videos' },
  { ext: '.mpeg', folder: 'videos' },
  { ext: '.mpg', folder: 'videos' },
  { ext: '.m2ts', folder: 'videos' },
  { ext: '.vob', folder: 'videos' },
  { ext: '.ogv', folder: 'videos' },

  // Archives and disk images.
  { ext: '.zip', folder: 'archives' },
  { ext: '.rar', folder: 'archives' },
  { ext: '.7z', folder: 'archives' },
  { ext: '.tar', folder: 'archives' },
  { ext: '.gz', folder: 'archives' },
  { ext: '.bz2', folder: 'archives' },
  { ext: '.xz', folder: 'archives' },
  { ext: '.tgz', folder: 'archives' },
  { ext: '.tbz2', folder: 'archives' },
  { ext: '.txz', folder: 'archives' },
  { ext: '.zst', folder: 'archives' },
  { ext: '.cab', folder: 'archives' },
  { ext: '.lz', folder: 'archives' },
  { ext: '.lzma', folder: 'archives' },
  { ext: '.iso', folder: 'archives' },

  // Executables and installers.
  { ext: '.bat', folder: 'programs' },
  { ext: '.cmd', folder: 'programs' },
  { ext: '.com', folder: 'programs' },
  { ext: '.exe', folder: 'programs' },
  { ext: '.msi', folder: 'programs' },
  { ext: '.apk', folder: 'programs' },
  { ext: '.app', folder: 'programs' },
  { ext: '.appimage', folder: 'programs' },
  { ext: '.dmg', folder: 'programs' },
  { ext: '.pkg', folder: 'programs' },
  { ext: '.deb', folder: 'programs' },
  { ext: '.rpm', folder: 'programs' },
  { ext: '.jar', folder: 'programs' },
  { ext: '.war', folder: 'programs' },
  { ext: '.ear', folder: 'programs' },
  { ext: '.bin', folder: 'programs' },
  { ext: '.run', folder: 'programs' },
  { ext: '.so', folder: 'programs' },
  { ext: '.dll', folder: 'programs' },
  { ext: '.dylib', folder: 'programs' },
  { ext: '.sys', folder: 'programs' },

  // Source code, build files, scripts, and web files.
  { ext: '.c', folder: 'code' },
  { ext: '.h', folder: 'code' },
  { ext: '.cpp', folder: 'code' },
  { ext: '.cc', folder: 'code' },
  { ext: '.cxx', folder: 'code' },
  { ext: '.hpp', folder: 'code' },
  { ext: '.hh', folder: 'code' },
  { ext: '.java', folder: 'code' },
  { ext: '.kt', folder: 'code' },
  { ext: '.kts', folder: 'code' },
  { ext: '.go', folder: 'code' },
  { ext: '.rs', folder: 'code' },
  { ext: '.cs', folder: 'code' },
  { ext: '.fs', folder: 'code' },
  { ext: '.swift', folder: 'code' },
  { ext: '.m', folder: 'code' },
  { ext: '.mm', folder: 'code' },
  { ext: '.vb', folder: 'code' },
  { ext: '.vbs', folder: 'code' },
  { ext: '.dart', folder: 'code' },
  { ext: '.scala', folder: 'code' },
  { ext: '.groovy', folder: 'code' },
  { ext: '.clj', folder: 'code' },
  { ext: '.cljs', folder: 'code' },
  { ext: '.lisp', folder: 'code' },
  { ext: '.el', folder: 'code' },
  { ext: '.hs', folder: 'code' },
  { ext: '.nim', folder: 'code' },
  { ext: '.zig', folder: 'code' },
  { ext: '.pas', folder: 'code' },
  { ext: '.erl', folder: 'code' },
  { ext: '.ex', folder: 'code' },
  { ext: '.exs', folder: 'code' },
  { ext: '.jl', folder: 'code' },
  { ext: '.r', folder: 'code' },
  { ext: '.vala', folder: 'code' },
  { ext: '.cmake', folder: 'code' },
  { ext: '.gradle', folder: 'code' },
  { ext: '.make', folder: 'code' },
  { ext: '.mk', folder: 'code' },
  { ext: '.sh', folder: 'code' },
  { ext: '.bash', folder: 'code' },
  { ext: '.zsh', folder: 'code' },
  { ext: '.fish', folder: 'code' },
  { ext: '.py', folder: 'code' },
  { ext: '.rb', folder: 'code' },
  { ext: '.php', folder: 'code' },
  { ext: '.pl', folder: 'code' },
  { ext: '.pm', folder: 'code' },
  { ext: '.lua', folder: 'code' },
  { ext: '.ps1', folder: 'code' },
  { ext: '.js', folder: 'code' },
  { ext: '.ts', folder: 'code' },
  { ext: '.mjs', folder: 'code' },
  { ext: '.cjs', folder: 'code' },
  { ext: '.jsx', folder: 'code' },
  { ext: '.tsx', folder: 'code' },
  { ext: '.sql', folder: 'code' },
  { ext: '.html', folder: 'code' },
  { ext: '.htm', folder: 'code' },
  { ext: '.xhtml', folder: 'code' },
  { ext: '.css', folder: 'code' },
  { ext: '.scss', folder: 'code' },
  { ext: '.sass', folder: 'code' },
  { ext: '.less', folder: 'code' },

  // Config and structured text files.
  { ext: '.json', folder: 'configs' },
  { ext: '.jsonl', folder: 'configs' },
  { ext: '.xml', folder: 'configs' },
  { ext: '.yaml', folder: 'configs' },
  { ext: '.yml', folder: 'configs' },
  { ext: '.toml', folder: 'configs' },
  { ext: '.ini', folder: 'configs' },
  { ext: '.cfg', folder: 'configs' },
  { ext: '.conf', folder: 'configs' },
  { ext: '.properties', folder: 'configs' },
  { ext: '.env', folder: 'configs' },
  { ext: '.local', folder: 'configs' },
  { ext: '.rc', folder: 'configs' },
  { ext: '.plist', folder: 'configs' },
  { ext: '.desktop', folder: 'configs' },

  // Data and database files.
  { ext: '.db', folder: 'data' },
  { ext: '.db3', folder: 'data' },
  { ext: '.sqlite', folder: 'data' },
  { ext: '.sqlite3', folder: 'data' },
  { ext: '.parquet', folder: 'data' },
  { ext: '.feather', folder: 'data' },
  { ext: '.orc', folder: 'data' },
  { ext: '.h5', folder: 'data' },
  { ext: '.hdf5', folder: 'data' },
  { ext: '.npy', folder: 'data' },
  { ext: '.npz', folder: 'data' },
  { ext: '.mat', folder: 'data' },
  { ext: '.pkl', folder: 'data' },
  { ext: '.pickle', folder: 'data' },
  { ext: '.sav', folder: 'data' },
  { ext: '.dat', folder: 'data' },

  // Design and creative assets.
  { ext: '.psd', folder: 'design' },
  { ext: '.ai', folder: 'design' },
  { ext: '.xd', folder: 'design' },
  { ext: '.fig', folder: 'design' },
  { ext: '.sketch', folder: 'design' },
  { ext: '.afdesign', folder: 'design' },
  { ext: '.indd', folder: 'design' },

  // 3D models and scene files.
  { ext: '.blend', folder: '3d' },
  { ext: '.obj', folder: '3d' },
  { ext: '.fbx', folder: '3d' },
  { ext: '.stl', folder: '3d' },
  { ext: '.dae', folder: '3d' },
  { ext: '.glb', folder: '3d' },
  { ext: '.gltf', folder: '3d' },
  { ext: '.ply', folder: '3d' },
  { ext: '.usdz', folder: '3d' },
  { ext: '.3ds', folder: '3d' },
  { ext: '.amf', folder: '3d' },
  { ext: '.3mf', folder: '3d' },

  // Fonts and certificates.
  { ext: '.ttf', folder: 'fonts' },
  { ext: '.otf', folder: 'fonts' },
  { ext: '.woff', folder: 'fonts' },
  { ext: '.woff2', folder: 'fonts' },
  { ext: '.eot', folder: 'fonts' },
  { ext: '.fon', folder: 'fonts' },
  { ext: '.pem', folder: 'certificates' },
  { ext: '.crt', folder: 'certificates' },
  { ext: '.cer', folder: 'certificates' },
  { ext: '.key', folder: 'certificates' },
  { ext: '.pfx', folder: 'certificates' },
  { ext: '.p12', folder: 'certificates' },
  { ext: '.der', folder: 'certificates' },

  // Logs and backup files.
  { ext: '.log', folder: 'logs' },
  { ext: '.out', folder: 'logs' },
  { ext: '.err', folder: 'logs' },
  { ext: '.trace', folder: 'logs' },
  { ext: '.bak', folder: 'backups' },
  { ext: '.backup', folder: 'backups' },
  { ext: '.old', folder: 'backups' },
  { ext: '.orig', folder: 'backups' },
  { ext: '.tmp', folder: 'backups' },
  { ext: '.swp', folder: 'backups' },
  { ext: '.swo', folder: 'backups' },
];

const RULE_MAP = new Map();
for (const rule of RULES) {
  if (!RULE_MAP.has(rule.ext)) {
    RULE_MAP.set(rule.ext, rule.folder);
  }
}

// Returns the extension, including the dot, or null when none exists.
// Dotfiles like ".env" are treated as extensions when a rule exists.
function getExtension(filename) {
  const baseName = path.basename(filename).toLowerCase();
  if (!baseName || baseName === '.' || baseName === '..') {
    return null;
  }

  const lastDot = baseName.lastIndexOf('.');
  if (lastDot > 0) {
    return baseName.slice(lastDot);
  }

  if (baseName.startsWith('.') && RULE_MAP.has(baseName)) {
    return baseName;
  }

  return null;
}

// Maps a file extension to its destination folder.
// Unknown extensions go to the "others" folder.
function findFolder(ext) {
  if (!ext) {
    return 'others';
  }

  return RULE_MAP.get(ext.toLowerCase()) || 'others';
}

// Removes trailing path separators so joins stay predictable.
function trimTrailingSeparator(input) {
  if (!input) {
    return '.';
  }

  const normalized = path.normalize(input);
  const root = path.parse(normalized).root;
  if (normalized === root) {
    return normalized;
  }

  const trimmed = normalized.replace(/[\\/]+$/, '');
  return trimmed || '.';
}

// Creates a directory if it does not already exist.
async function ensureDir(dirPath, createdFolders) {
  if (createdFolders.has(dirPath)) {
    return;
  }

  await fs.mkdir(dirPath, { recursive: true });
  createdFolders.add(dirPath);
}

// Scans the target directory and moves each regular file into a category.
// Subdirectories are intentionally left untouched.
async function organizeDirectory(targetDir) {
  if (!targetDir) {
    throw new Error('Error: null directory path');
  }

  const workDir = path.resolve(trimTrailingSeparator(targetDir));
  const entries = await fs.readdir(workDir, { withFileTypes: true });
  const createdFolders = new Set();
  let moved = 0;
  let failed = 0;

  for (const entry of entries) {
    if (entry.name === '.' || entry.name === '..') {
      continue;
    }

    const sourcePath = path.join(workDir, entry.name);

    let isFile = entry.isFile();
    if (!isFile) {
      if (entry.isDirectory()) {
        continue;
      }

      try {
        isFile = (await fs.stat(sourcePath)).isFile();
      } catch {
        failed += 1;
        continue;
      }
    }

    if (!isFile) {
      continue;
    }

    let folder = findFolder(getExtension(entry.name));

    // Special rule for .ts files: decide by file size.
    // If file > 1024 KB (1 MB) treat as video, otherwise treat as code.
    const ext = getExtension(entry.name);
    if (ext && ext.toLowerCase() === '.ts') {
      try {
        const st = await fs.stat(sourcePath);
        const bytes = st.size;
        const threshold = 1024 * 1024; // 1024 KB == 1,048,576 bytes
        folder = bytes > threshold ? 'videos' : 'code';
      } catch (err) {
        // if stat fails, fall back to the default folder
      }
    }
    const folderPath = path.join(workDir, folder);

    try {
      await ensureDir(folderPath, createdFolders);
    } catch {
      console.error(`Failed to prepare folder: ${folder}`);
      failed += 1;
      continue;
    }

    const destPath = path.join(folderPath, entry.name);

    try {
      await fs.rename(sourcePath, destPath);
      console.log(`Moved: ${entry.name} -> ${folder}/`);
      moved += 1;
    } catch {
      console.error(`Failed to move: ${entry.name}`);
      failed += 1;
    }
  }

  console.log(`\nDone! ${moved} moved, ${failed} failed`);
  return { moved, failed };
}

async function main(argv = process.argv) {
  const target = argv[2] || '.';

  console.log('Folderly - Simple File Organizer');
  console.log(`Organizing files in: ${target}\n`);

  await organizeDirectory(target);
}

if (require.main === module) {
  main().catch((error) => {
    console.error(error.message || String(error));
    process.exitCode = 1;
  });
}

module.exports = {
  RULES,
  RULE_MAP,
  getExtension,
  findFolder,
  organizeDirectory,
};
