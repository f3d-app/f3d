# CodeQL CLI Command Reference

Detailed reference for the CodeQL CLI — installation, database creation, analysis, SARIF upload, and CI integration.

## Installation

### Download the CodeQL Bundle

Always download the CodeQL bundle (CLI + precompiled queries) from:
**https://github.com/github/codeql-action/releases**

The bundle includes:
- CodeQL CLI product
- Compatible queries and libraries from `github/codeql`
- Precompiled query plans for faster analysis

### Platform-Specific Bundles

| Platform | File |
|---|---|
| All platforms | `codeql-bundle.tar.zst` |
| Linux | `codeql-bundle-linux64.tar.zst` |
| macOS | `codeql-bundle-osx64.tar.zst` |
| Windows | `codeql-bundle-win64.tar.zst` |

### Setup

```bash
tar xf codeql-bundle-linux64.tar.zst
export PATH="$HOME/codeql:$PATH"
codeql resolve packs
codeql resolve languages
```

## Core Commands

### `codeql database create`

```bash
# Interpreted language
codeql database create <output-dir> \
  --language=<language> \
  --source-root=<source-dir>

# Compiled language with build command
codeql database create <output-dir> \
  --language=java-kotlin \
  --command='./gradlew build' \
  --source-root=.

# Multiple languages (cluster mode)
codeql database create <output-dir> \
  --db-cluster \
  --language=java,python,javascript-typescript \
  --command='./build.sh' \
  --source-root=.
```

**Key flags:**

| Flag | Description |
|---|---|
| `--language=<lang>` | Language to extract (required) |
| `--source-root=<dir>` | Root directory of source code |
| `--command=<cmd>` | Build command for compiled languages |
| `--db-cluster` | Create databases for multiple languages |
| `--overwrite` | Overwrite existing database directory |
| `--threads=<n>` | Threads for extraction (0 = all cores) |
| `--ram=<mb>` | RAM limit in MB |

### `codeql database analyze`

```bash
codeql database analyze <database-dir> \
  <query-suite-or-pack> \
  --format=sarif-latest \
  --sarif-category=<category> \
  --output=<output-file>
```

**Key flags:**

| Flag | Description |
|---|---|
| `--format=sarif-latest` | Output format |
| `--sarif-category=<cat>` | Category tag for SARIF results |
| `--output=<file>` | Output file path |
| `--threads=<n>` | Threads for analysis |
| `--ram=<mb>` | RAM limit |
| `--sarif-add-file-contents` | Include source file contents |
| `--ungroup-results` | Disable result grouping |
| `--no-download` | Skip downloading query packs |

**Common query suites:**

| Suite | Description |
|---|---|
| `<lang>-code-scanning.qls` | Standard code scanning queries |
| `<lang>-security-extended.qls` | Extended security queries |
| `<lang>-security-and-quality.qls` | Security + code quality queries |

### `codeql github upload-results`

```bash
codeql github upload-results \
  --repository=<owner/repo> \
  --ref=<git-ref> \
  --commit=<commit-sha> \
  --sarif=<sarif-file>
```

**Key flags:**

| Flag | Description |
|---|---|
| `--repository=<owner/repo>` | Target GitHub repository |
| `--ref=<ref>` | Git ref (e.g., `refs/heads/main`) |
| `--commit=<sha>` | Full commit SHA |
| `--sarif=<file>` | Path to SARIF file |
| `--github-url=<url>` | GitHub instance URL (for GHES) |
| `--github-auth-stdin` | Read auth token from stdin |

**Authentication:** Set `GITHUB_TOKEN` with `security-events: write` scope.

### `codeql resolve packs`

```bash
codeql resolve packs
```

Lists available query packs. Use to verify installation and diagnose missing packs.

### `codeql resolve languages`

```bash
codeql resolve languages
```

Shows which language extractors are available.

### `codeql database bundle`

```bash
codeql database bundle <database-dir> --output=<archive-file>
```

Creates a relocatable archive for sharing or troubleshooting.

## CLI Server Mode

```bash
codeql execute cli-server [options]
```

Runs a persistent server to avoid repeated JVM initialization. The server accepts commands via stdin and returns results, keeping the JVM warm between commands.

**Key flags:**

| Flag | Description |
|---|---|
| `--verbosity=<level>` | `errors`, `warnings`, `progress`, `progress+`, `progress++`, `progress+++` |
| `--logdir=<dir>` | Write detailed logs to directory |
| `--common-caches=<dir>` | Location for persistent cached data (default: `~/.codeql`) |
| `-J=<opt>` | Pass option to the JVM |

## CI Integration Pattern

```bash
#!/bin/bash
set -euo pipefail

REPO="my-org/my-repo"
REF="refs/heads/main"
COMMIT=$(git rev-parse HEAD)
LANGUAGES=("javascript-typescript" "python")

codeql database create codeql-dbs \
  --db-cluster \
  --source-root=. \
  --language=$(IFS=,; echo "${LANGUAGES[*]}")

for lang in "${LANGUAGES[@]}"; do
  codeql database analyze "codeql-dbs/$lang" \
    "${lang}-security-extended.qls" \
    --format=sarif-latest \
    --sarif-category="$lang" \
    --output="${lang}-results.sarif" \
    --threads=0

  codeql github upload-results \
    --repository="$REPO" \
    --ref="$REF" \
    --commit="$COMMIT" \
    --sarif="${lang}-results.sarif"
done
```

## Environment Variables

| Variable | Purpose |
|---|---|
| `GITHUB_TOKEN` | Authentication for `github upload-results` |
| `CODEQL_EXTRACTOR_CPP_AUTOINSTALL_DEPENDENCIES` | Auto-install C/C++ build dependencies on Ubuntu |
| `CODEQL_RAM` | Override default RAM allocation |
| `CODEQL_THREADS` | Override default thread count |
