# AGENTS

This repository expects concise, disciplined English-only collaboration for commits and pull requests.

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
