# Vibe Configuration & Documentation Hub

This directory contains all configuration, agents, skills, and instructions organized for Mistral Vibe.

## Directory Structure

```
.vibe/
├── AGENTS.md                    # Project-specific agents config (Apeirogon)
├── README.md                    # This file
├── LESSONS_LEARNED.md          # Self-update from this session
├── ROBUSTNESS_ANALYSIS.md       # Full robustness analysis report
├── CRITICAL_REVIEW.md           # Critical verification of analysis
├── TDD_PLAN_SUMMARY.md         # Test-driven development plan
├── agents/                      # Agent definitions
│   ├── adr-generator.agent.md
│   ├── blueprint-mode.agent.md
│   ├── context-architect.agent.md
│   ├── critical-review.agent.md
│   ├── debug.agent.md
│   ├── documentalist.agent.md
│   └── expert-cpp-software-engineer.agent.md
├── docs/                        # Reference documentation
│   ├── COPILOT_GRAPHIFY.md
│   ├── copilot-instructions.md
│   └── function-dictionary.md
├── instructions/                # Instruction sets
│   ├── cmake-vcpkg.instructions.md
│   ├── code-review-generic.instructions.md
│   ├── context-engineering.instructions.md
│   ├── cpp-language-service-tools.instructions.md
│   ├── github-actions-ci-cd-best-practices.instructions.md
│   ├── oop-design-patterns.instructions.md
│   └── update-docs-on-code-change.instructions.md
└── skills/                      # Skill definitions
    ├── add-educational-comments/
    ├── architecture-blueprint-generator/
    ├── autoresearch/
    ├── codeql/
    ├── context-map/
    ├── documentation-writer/
    ├── excalidraw-diagram-generator/
    ├── lsp-setup/
    ├── quality-playbook/
    └── refactor-plan/
```

## Quick Reference

### Agents

| Agent | Purpose |
|-------|---------|
| `critical-review` | Mandatory code review for non-trivial changes |
| `blueprint-mode` | Architecture planning and design |
| `context-architect` | Context engineering for complex tasks |
| `debug` | Debugging and issue resolution |
| `documentalist` | Documentation generation |
| `adr-generator` | Architecture Decision Records |
| `expert-cpp-software-engineer` | C++ expertise |

### Key Project Conventions (from AGENTS.md)

- **Mandatory Review**: All non-trivial changes require `Critical Review` agent
- **C++20** library for exact 3D Boolean operations on polyhedra
- **Exact arithmetic**: No tolerances/epsilons in geometric logic
- **Plane-based points**: Points = intersection of 3 planes
- **Factory pattern**: Use `GeoObjectFactory` for object creation

### Build & Test

```bash
cmake --preset ninja-release-vcpkg
cmake --build --preset ninja-release-vcpkg
cd builds/ninja-release-vcpkg && ctest
```

### Sanitizer Presets

| Preset | Sanitizer | Use Case |
|--------|-----------|----------|
| `ninja-asan` | AddressSanitizer | buffer overflows, use-after-free |
| `ninja-ubsan` | UndefinedBehaviorSanitizer | signed overflow, bad casts |
| `ninja-asan-ubsan` | Both | General-purpose |
| `ninja-tsan` | ThreadSanitizer | data races (incompatible with ASan/LSan) |

## Workflow

1. **Before coding**: Consult `blueprint-mode.agent.md` for architecture
2. **During development**: Use `expert-cpp-software-engineer.agent.md` for C++ guidance
3. **Before commit**: Run `Critical Review` agent (mandatory)
4. **For debugging**: Use `debug.agent.md`
5. **For documentation**: Use `documentalist.agent.md`

## Source

All files in this directory are copies from `.github/` organized for Vibe's use.
Original source: `.github/{agents,instructions,skills,*.md}`
