# AGENTS

This repository expects concise, disciplined English-only collaboration for commits and pull requests.

## Terminology Policy

Contributors and documenters must not interpret or document the `Four Pillars` feature set using the term `bazi` (`八字`)
or astrology/divination-style framing.
Use `Four Pillars` (`四柱/사주/sizhu`) only as a calendrical metadata context for supported Gregorian/Chinese lunisolar
conversions.

## Tooling Policy

For generator tooling under `tools/`, dependency management must use `tools/requirements.txt` only and should be treated as
environment bootstrap only. The shell wrapper (`tools/gen_lunisolar_data.sh`) must install dependencies from that file and avoid
packaging metadata parsing or project-level package installation paths for generation.

The generator framework itself is fully decoupled from any concrete calendar implementation. It has no hard third-party runtime
dependencies; only concrete implementations (such as the default `sxtwl` source) add their own requirements. Consumers can add
custom generators by subclassing `tools/calendar_source.py` and only updating `tools/requirements.txt`, without changing any
other project files. The generator runner (`tools/gen_lunisolar_data.sh`) and decoder surface (`include/lunisolar.h`) remain usable
as-is.

Do not use `pyproject.toml` to drive generator dependency installation. It is intentionally avoided for generation
bootstrapping because each of the following is unfriendly in this context:

- generator dependency loading via `tools/pyproject.toml`
- manual parsing of `pyproject.toml` metadata (for example, via `tomllib`)
- introducing `uv` into generation dependency flows
- adding extra third-party bootstrap dependencies beyond the standard library

`pyproject.toml` remains supported for actual project/package management only, where packaging is the intended use case.

## Documentation Link Policy

Do not use system filesystem absolute paths when referencing repository files in documentation, comments, or PR descriptions.
Allowed path styles are:
- repository-relative paths (for example: `README.md`, `docs/api.md`, `include/lunisolar.h`)
- web URLs.

## Branch Naming

Use `dev` and `fix` as branch prefixes when work should trigger the CI workflow on push.

Examples:

```text
dev/some-feature
fix/cmake-minimum-version
```

Rules:

- Use `dev/...` for development branches.
- Use `fix/...` for bug-fix or CI-fix branches.
- Keep the branch purpose clear and short after the prefix.

## Commit Message Format

Please commit ONLY in English using the following format:

```text
<behavior>(<domain>): <short description>

<detailed explanation: optional, multi-line, no-markdown-formatting, optional `*` for bullet points>
```

Example:

```text
refactor(module1_name, module2_name, tests): do something and something else

This commit refactors module1 and module2 to improve performance and maintainability.

Module1:
* do something
* do something else

Module2:
* do something
* do something else

Tests:
* update some tests to reflect some semantic changes
```

Another example:

```text
docs(module_name): update documentation for some behavior

This commit updates the documentation for module_name to clarify the expected behavior of some function.

module_name::submodule_name:
* some_function: documented to reflect some behavior
* some_sub_class: documented to reflect some behavior

another_module_name:
* some_function: documented to reflect some behavior
```

### Rules

- Use English only.
- Use the exact header shape: `<behavior>(<domain>): <short description>`.
- Use a specific domain such as `docs(readme)`, `fix(calendar)`, or `refactor(tools, tests)`.
- Keep the short description concise and behavior-oriented.
- The body is optional.
- If a body is present, write plain text only.
- Use `*` for flat bullet points only.
- Do not use nested bullet points.
- Do not use Markdown formatting in the commit message body.

### Anti-Patterns

Bad:

```text
fix: fix some bug in some module
```

Explanation: use `fix(module_name): ...` instead of a generic `fix:`.

Bad:

```text
Module2:
  * do something
```

Explanation: no nested bullet points.

Bad:

```text
**Module2**:
 - do something
```

Explanation: Markdown formatting is not allowed in the commit message body. Use `*` for bullet points, not `-`.

## Pull Request Format

Please create PRs ONLY in English, including the following sections:

- What problem does this PR solve / What is new in this PR?
- Is the PR backed by CI tests?
- Any breaking changes?
