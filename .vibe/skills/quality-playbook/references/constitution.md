# Writing the Quality Constitution (File 1: QUALITY.md)

The quality constitution defines what "quality" means for this specific project and makes the bar explicit, persistent, and inherited by every AI session.

## Template

```markdown
# Quality Constitution: [Project Name]

## Purpose

[2–3 paragraphs grounding quality in three principles:]

- **Deming** ("quality is built in, not inspected in") — Quality is built into context files
  and the quality playbook so every AI session inherits the same bar.
- **Juran** ("fitness for use") — Define fitness specifically for this project. Not "tests pass"
  but the actual real-world requirement.
- **Crosby** ("quality is free") — Building a quality playbook upfront costs less than
  debugging problems found after deployment.

## Coverage Targets

| Subsystem | Target | Why |
|-----------|--------|-----|
| [Most fragile module] | 90–95% | [Real edge case or past bug] |
| [Core logic module] | 85–90% | [Concrete risk] |
| [I/O or integration layer] | 80% | [Explain] |
| [Configuration/utilities] | 75–80% | [Explain] |

## Coverage Theater Prevention

[Define what constitutes a fake test for this project. Examples:]
- Asserting a function returned *something* without checking what
- Testing with synthetic data that lacks the quirks of real data
- Asserting an import succeeded
- Asserting mock returns what the mock was configured to return

## Fitness-to-Purpose Scenarios

### Scenario N: [Memorable Name]

**Requirement tag:** [Req: formal — Spec §X]

**What happened:** [The architectural vulnerability. Reference actual code — function names, file names, line numbers. Frame as "this architecture permits the following failure mode."]

**The requirement:** [What the code must do to prevent this failure.]

**How to verify:** [Concrete test or query that would fail if this regressed.]

## AI Session Quality Discipline

1. Read QUALITY.md before starting work.
2. Run the full test suite before marking any task complete.
3. Add tests for new functionality (not just happy path — include edge cases).
4. Update this file if new failure modes are discovered.
5. Never remove a fitness-to-purpose scenario. Only add new ones.

## The Human Gate

[List things that require human judgment: UX, security review of auth changes, backward compatibility decisions, etc.]
```

## Where Scenarios Come From

Scenarios come from two sources — **code exploration** and **domain knowledge**.

### Source 1: Defensive Code Patterns (Code Exploration)

Every defensive pattern is evidence of a past failure or known risk:

1. **Defensive code** — Every null check, try/catch, or guard clause
2. **Normalization functions** — Exist because raw input caused problems
3. **Configuration that could be hardcoded** — Someone learned the value varies
4. **Comments explaining "why"** — Design decisions and constraints

### Source 2: What Could Go Wrong (Domain Knowledge)

For every major subsystem, ask:
- "What happens if this process is killed mid-operation?"
- "What happens if external input is subtly wrong?"
- "What happens if this runs at 10x scale?"
- "What happens if two operations overlap?"
- "What produces correct-looking output that is actually wrong?"

### The Narrative Voice

Each scenario's "What happened" must read like an architectural vulnerability analysis:
- **Specific quantities** — "308 records across 64 batches" not "some records"
- **Cascade consequences** — what breaks downstream
- **Detection difficulty** — "nothing would flag them as missing"
- **Root cause in code** — reference the actual function/line

## Calibrating Scenario Count

Aim for 2+ scenarios per core module. For a medium-sized project: 8–10 scenarios. Quality matters more than count: one scenario that precisely captures an architectural vulnerability is worth more than three generic ones.

## Critical Rule

Each scenario's "How to verify" section must map to at least one automated test in the functional test file.
