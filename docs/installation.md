### Compilation
```bash
clang db.c -o dblite
```

Notes:
-- Name the executable `dblite` to prevent clashing with other files

- After building the executable, copy it into the `.bin` directory
```bash
mv dblite ./.bin/
```
- Add the `.bin` directory to path. This should make the executable available in the shell.