# folderly — Simple File Organizer (CLI)

[![npm version](https://img.shields.io/npm/v/folderly.svg)](https://www.npmjs.com/package/folderly)
[![downloads](https://img.shields.io/npm/dy/folderly.svg)](https://www.npmjs.com/package/folderly)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/mdsiaofficial/folderly/blob/main/LICENSE) 
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://folderly.vervel.app/how-to-contribute.html#your-first-pull-request)
![GitHub Repo stars](https://img.shields.io/github/stars/mdsiaofficial/folderly)

*folderly* is a small, dependency-free Node.js CLI that scans a directory (non-recursively) and sorts regular files into category folders based on file extension (documents, images, videos, code, etc.). It is designed to be fast and predictable and publishes as an npm binary `folderly`.

## Features

- Classifies files by extension into named folders (documents, images, music, videos, code, archives, etc.)
- Uses safe filesystem operations (Node's fs/promises)
- Small, single-file entrypoint with programmatic exports for testing
- Case-insensitive extension matching

## Installation

Install globally with npm:

```bash
npm install -g folderly
```

Or run directly with npx:

```bash
npx folderly
```

## Usage

Run in the directory you want to organize (defaults to current directory):

```bash
folderly
# or specify a path
folderly path/to/dir
```

### Example Output

```
Folderly - Simple File Organizer
Organizing files in: test_folder

Moved: photo.jpg -> images/
Moved: sample.txt -> documents/
Moved: audio.wav -> music/
Moved: movie.mp4 -> videos/
Moved: unknown.xyz -> others/

Done! 5 moved, 0 failed
```

## Notes & Behavior

- Only regular files are moved; subdirectories are left untouched.
- Files with unknown extensions are moved to the `others` folder.
- The command uses `fs.rename()` so moves are fast but can fail across different mount points/filesystems.
- No recursive scanning (only the specified directory level).

## Contributing

Contributions are welcome. Preferred workflow:

1. Fork the repo
2. Create a feature branch: `git checkout -b feature/your-change`
3. Make changes and add tests if appropriate
4. Commit and push your branch
5. Open a Pull Request

## License

MIT. See the LICENSE file for details.

## Links

- [npm Package](https://www.npmjs.com/package/folderly)
- [GitHub Repository](https://github.com/mdsiaofficial/folderly)
- [Issues](https://github.com/mdsiaofficial/folderly/issues)