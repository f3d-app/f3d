Copilot + Graphify integration

Purpose
- Teach Copilot CLI and contributors how to use `graphify` within this repo for architecture mapping, semantic extraction, and reporting.

Key rules for Copilot:
- Always run graphify detect+AST before semantic extraction. Use graphify-out/.graphify_detect.json and ./.graphify_ast.json as checkpoints.
- For semantic extraction, prefer dispatching parallel "general-purpose" subagents (chunks ≤22 files). Use cached results when present; only force re-extraction when user explicitly asks "force semantic extraction".
- If corpus >200 files or >2,000,000 words, prompt user to narrow scope to subdirectories.
- After semantic extraction, merge AST + semantic, cluster, and generate GRAPH_REPORT.md and graph.html under <dir>/graphify-out/.
- When running subagents, always include report: chunks dispatched/succeeded/failed, new_nodes, new_edges, and any errors.

Developer workflow notes
- Use graphify for PlaneBasedOperators, PlaneBasedGeometry, PlaneBasedTests, PlaneBasedIO, Infrastructure scopes.
- For CI-friendly runs, skip semantic step for code-only changes; run AST+cluster only.

Follow-up
- After updating graphs, continue test sanitization work: run targeted tests, produce status report (pass/fail counts), and update plan.md and .github/function-dictionary.md.

(Automated file: created by Copilot CLI to record graphify integration guidance.)
