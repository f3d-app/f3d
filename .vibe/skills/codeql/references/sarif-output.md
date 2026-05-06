# CodeQL SARIF Output Reference

Detailed reference for the SARIF v2.1.0 output produced by CodeQL analysis.

## About SARIF

SARIF (Static Analysis Results Interchange Format) is a standardized JSON format for representing static analysis tool output. CodeQL produces SARIF v2.1.0.

- Specification: OASIS SARIF v2.1.0
- Format type: `sarifv2.1.0` (passed to `--format` flag)

## Top-Level Structure

### `sarifLog` Object

| Property | Always Generated | Description |
|---|:---:|---|
| `$schema` | yes | Link to the SARIF schema |
| `version` | yes | SARIF specification version (`"2.1.0"`) |
| `runs` | yes | Array containing a single `run` object per language |

### `run` Object

| Property | Always Generated | Description |
|---|:---:|---|
| `tool` | yes | Tool information (`toolComponent`) |
| `artifacts` | yes | Array of artifact objects for every file referenced in a result |
| `results` | yes | Array of `result` objects |
| `properties` | yes | Contains `semmle.formatSpecifier` identifying the format |

## Results

### `result` Object

| Property | Always Generated | Description |
|---|:---:|---|
| `ruleId` | yes | Rule identifier (matches `reportingDescriptor.id`) |
| `message` | yes | Problem description |
| `locations` | yes | Array containing a single `location` object |
| `partialFingerprints` | yes | Dictionary with at least `primaryLocationLineHash` for deduplication |
| `codeFlows` | no | Populated for `@kind path-problem` queries |
| `suppressions` | no | If suppressed: single `suppression` object with `@kind: IN_SOURCE` |

## Severity Mapping

| CodeQL `@severity` | SARIF `level` |
|---|---|
| `error` | `error` |
| `warning` | `warning` |
| `recommendation` | `note` |

## Security Severity (Third-Party)

| Property | Description |
|---|---|
| `properties.security-severity` | Score 0.0–10.0. Maps to: >9.0=critical, 7.0–8.9=high, 4.0–6.9=medium, 0.1–3.9=low |
| `properties.precision` | `very-high`, `high`, `medium`, `low` — affects display ordering |

## Automation Details

The `category` value from `github/codeql-action/analyze` appears as `automationDetails.id`:

```json
{
  "automationDetails": {
    "id": "/language:javascript-typescript"
  }
}
```

## Key CLI Flags for SARIF

| Flag | Effect |
|---|---|
| `--format=sarif-latest` | Produce SARIF v2.1.0 output |
| `--sarif-category=<cat>` | Set `automationDetails.id` for result categorization |
| `--sarif-add-file-contents` | Include source file content in `artifact.contents` |
| `--ungroup-results` | Report every occurrence separately |
| `--output=<file>` | Write SARIF to specified file |

## Upload Limits

### File Size
- Maximum: **10 MB** (gzip-compressed)
- If too large: reduce query scope, remove `--sarif-add-file-contents`, or split into multiple uploads

### Object Count Limits

| Object | Maximum |
|---|---|
| Runs per file | 20 |
| Results per run | 25,000 |
| Rules per run | 25,000 |
| Thread flow locations per result | 10,000 |
| Locations per result | 1,000 |
| Tags per rule | 20 |

Files exceeding these limits are rejected. Split analysis across multiple SARIF uploads with different `--sarif-category` values.

## Source File Location Requirements

- Use relative paths (relative to repository root) when possible
- Consistent file paths are required across runs for fingerprint stability

## Fingerprint Requirements

- `partialFingerprints` with `primaryLocationLineHash` prevents duplicate alerts across commits
- CodeQL SARIF automatically includes fingerprints
- API uploads without fingerprints may produce duplicate alerts
