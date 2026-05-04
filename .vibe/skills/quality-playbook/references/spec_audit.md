# Council of Three Spec Audit Protocol (File 5)

This is a static analysis protocol — AI models read the code and compare it to specifications. No code is executed. It catches spec-code divergence, undocumented features, phantom specs, and missing implementations.

## Why Three Models?

Different AI models have different blind spots. Cross-referencing three independent reviews catches defects that any single model would miss.

## Template

```markdown
# Spec Audit Protocol: [Project Name]

## The Definitive Audit Prompt

Give this prompt identically to three independent AI tools (e.g., Claude, GPT, Gemini).

---

**Context files to read:**
1. [List all spec/intent documents with paths]
2. [Architecture docs]
3. [Design decision records]
4. [AGENTS.md or similar bootstrap file]

**Task:** Act as the Tester. Read the actual code in [source directories] and compare it against the specifications listed above.

**Requirement confidence tiers:**
Requirements are tagged with `[Req: tier — source]`. Weight your findings by tier:
- **formal** — written by humans in a spec document. Authoritative. Divergence is a real finding.
- **user-confirmed** — stated by the user but not in a formal doc. Treat as authoritative.
- **inferred** — deduced from code behavior. Report divergence as NEEDS REVIEW, not as a definitive defect.

**Rules:**
- ONLY list defects. Do not summarize what matches.
- For EVERY defect, cite specific file and line number(s). If you cannot cite a line number, do not include the finding.
- Before claiming missing, grep the codebase.
- Before claiming exists, read the actual function body.
- Classify each finding: MISSING / DIVERGENT / UNDOCUMENTED / PHANTOM
- For findings against inferred requirements, add: NEEDS REVIEW

**Defect classifications:**
- **MISSING** — Spec requires it, code doesn't implement it
- **DIVERGENT** — Both spec and code address it, but they disagree
- **UNDOCUMENTED** — Code does it, spec doesn't mention it
- **PHANTOM** — Spec describes it, but it's implemented differently than described

**Project-specific scrutiny areas:**

[5–10 specific questions that force the auditor to read the most critical code. Target:]

1. [The most fragile module — force the auditor to read specific functions]
2. [External data handling — validation, normalization, error recovery]
3. [Assumptions that might not hold — pointer validity, value ranges, format consistency]
4. [Features that cross module boundaries]
5. [The gap between documentation and implementation]
6. [Specific edge cases from the QUALITY.md scenarios]

**Output format:**

### [filename.ext]
- **Line NNN:** [MISSING / DIVERGENT / UNDOCUMENTED / PHANTOM] [Req: tier — source] Description.
  Spec says: [quote or reference]. Code does: [what actually happens].

---

## Running the Audit

1. Give the identical prompt to three AI tools
2. Each auditor works independently — no cross-contamination
3. Collect all three reports

## Triage Process

After all three models report, merge findings:

| Confidence | Found By | Action |
|------------|----------|--------|
| Highest | All three | Almost certainly real — fix or update spec |
| High | Two of three | Likely real — verify and fix |
| Needs verification | One only | Could be real or hallucinated — deploy verification probe |

## Fix Execution Rules

- Group fixes by subsystem, not by defect number
- Never one mega-prompt for all fixes
- Each batch: implement, test, have all three reviewers verify the diff
- At least two auditors must confirm fixes pass before marking complete

## Output

Save audit reports to `quality/spec_audits/YYYY-MM-DD-[model].md`
Save triage summary to `quality/spec_audits/YYYY-MM-DD-triage.md`
```

## The Four Guardrails (Critical for All Auditors)

1. **Mandatory line numbers** — If you cannot cite a line number, do not include the finding.
2. **Grep before claiming missing** — Before saying a feature is absent, search the codebase.
3. **Read function bodies, not just signatures**
4. **Classify defect type** — Forces structured thinking (MISSING/DIVERGENT/UNDOCUMENTED/PHANTOM)

## Tips for Writing Scrutiny Areas

Good scrutiny areas name specific functions, files, and edge cases:
- "Read `process_input()` in `pipeline.cpp` lines 45–120. The spec says it should handle missing fields by substituting defaults. Does it?"
- "The architecture doc says Module A passes validated data to Module B. Read both modules. Is there any path where unvalidated data reaches Module B?"

Bad scrutiny areas:
- "Check if the code is correct"
- "Look for bugs"
