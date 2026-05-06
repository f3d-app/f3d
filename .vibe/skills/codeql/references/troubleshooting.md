# CodeQL Troubleshooting Reference

Comprehensive guide for diagnosing and resolving CodeQL analysis errors, SARIF upload issues, and common configuration problems.

## Build and Analysis Errors

### "No source code was seen during the build"

**Cause:** CodeQL extractor did not find any source files during database creation.

**Solutions:**
- Verify the `--source-root` points to the correct directory
- For compiled languages, ensure the build command actually compiles source files
- Check that `autobuild` is detecting the correct build system
- Switch from `autobuild` to `manual` build mode with explicit build commands
- Verify the language specified matches the actual source code language

### Automatic Build Failed

**Solutions:**
- Switch to `build-mode: manual` and provide explicit build commands
- Ensure all build dependencies are installed on the runner
- For C/C++: verify `gcc`, `make`, `cmake`, or `msbuild` are available
- For C#: verify `.NET SDK` or `MSBuild` is installed
- For Java: verify `gradle` or `maven` is installed

### C# Compiler Unexpectedly Failing

**Cause:** CodeQL injects `/p:EmitCompilerGeneratedFiles=true` which can cause issues with legacy .NET Framework projects or `.sqlproj` files.

**Solutions:**
- Add `<EmitCompilerGeneratedFiles>false</EmitCompilerGeneratedFiles>` to problematic project files
- Use `build-mode: none` for C# if build accuracy is acceptable

### Analysis Takes Too Long

**Solutions:**
- Use `build-mode: none` where accuracy is acceptable (significantly faster)
- Enable dependency caching: `dependency-caching: true`
- Set `timeout-minutes` on the job to prevent hung workflows
- Use `--threads=0` (CLI) to use all available CPU cores
- Reduce query scope: use `default` suite instead of `security-and-quality`
- Use `paths` in config file to limit analyzed directories

### CodeQL Scanned Fewer Lines Than Expected

**Solutions:**
- Switch from `none` to `autobuild` or `manual` build mode
- Ensure the build command compiles the full codebase (not just a subset)
- Check the code scanning logs for extraction metrics

### Kotlin Detected in No-Build Mode

**Solutions:**
- Disable default setup and re-enable it (switches to `autobuild`)
- Or switch to advanced setup with `build-mode: autobuild` for `java-kotlin`

## Permission and Access Errors

### Error: 403 "Resource not accessible by integration"

**Solutions:**
- Add explicit permissions to the workflow:
  ```yaml
  permissions:
    security-events: write
    contents: read
    actions: read
  ```
- For Dependabot PRs, use `pull_request_target` instead of `pull_request`

## Configuration Errors

### Two CodeQL Workflows Running

**Solutions:**
- Disable default setup if using advanced setup, or
- Delete the old workflow file if using default setup

### Error: "is not a .ql file, .qls file, a directory, or a query pack specification"

**Solutions:**
- Verify query pack names and versions exist
- Use correct format: `owner/pack-name@version` or `owner/pack-name:path/to/query.ql`
- Run `codeql resolve packs` to verify available packs

## Resource Errors

### "Out of disk" or "Out of memory"

**Solutions:**
- Use larger GitHub-hosted runners (if available)
- For self-hosted runners, increase RAM and disk (SSD with ≥14 GB)
- Reduce analysis scope with `paths` configuration
- Use `build-mode: none` to reduce resource usage

## Logging and Debugging

### Enable Debug Logging

**GitHub Actions:**
1. Re-run the workflow with debug logging enabled
2. In the workflow run, click "Re-run jobs" → "Enable debug logging"

**CodeQL CLI:**
```bash
codeql database create my-db \
  --language=javascript-typescript \
  --verbosity=progress++ \
  --logdir=codeql-logs
```

**Verbosity levels:** `errors`, `warnings`, `progress`, `progress+`, `progress++`, `progress+++`

## SARIF Upload Errors

### SARIF File Too Large (10 MB limit)

**Solutions:**
- Focus on fewer query suites (use `default` instead of `security-and-quality`)
- Split analysis into multiple jobs with separate SARIF uploads
- Remove `--sarif-add-file-contents` flag

### SARIF Results Exceed Limits

**Solutions:**
- Reduce query scope to focus on high-impact rules
- Split analysis across multiple SARIF uploads with different `--sarif-category`

### Missing Authentication Token

**Solutions:**
- Set `GITHUB_TOKEN` environment variable with `security-events: write` scope
- Or use `--github-auth-stdin` to pipe the token

## Hardware Requirements (Self-Hosted Runners)

| Codebase Size | RAM | CPU |
|---|---|---|
| Small (<100K LOC) | 8 GB+ | 2 cores |
| Medium (100K–1M LOC) | 16 GB+ | 4–8 cores |
| Large (>1M LOC) | 64 GB+ | 8 cores |

All sizes: SSD with ≥14 GB free disk space.
