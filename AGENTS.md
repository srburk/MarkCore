# AGENTS.md

C markdown library: parse a buffer into an AST, then walk the tree through a renderer vtable. Goal is a small, embeddable engine with room for extra tree passes (spellcheck, suggestions) and extra sinks (not just HTML). It is **not** CommonMark-complete.

Human-facing build notes live in `README.md`. This file is for agents working in the repo.

## Layout

```
include/markcore.h          public API (currently one function)
src/types.h                 MCNode_t + MCNodeType_e (shared AST)
src/parser.c / parser.h     markdown → tree; owns node_stack
src/renderer.c / renderer.h recursive walk + SAFE_RENDER_CALL
src/renderers/html_renderer.c   HTML sink (escaping lives here)
src/stack.c                 generic void* stack
src/markcore.c              parse → html renderer → free
tools/markcore-cli.c        read file, render to stdout
tests/test_markcore.c       public-API regression tests
```

Internal headers stay under `src/`. Only `include/markcore.h` is the library contract.

## Pipeline

```
markdown buffer
    → markcore_parse()           line loop + inline scan → MCNode_t tree
    → render_syntax_tree()       switch on node type, call Renderer_t fns
    → html_renderer emit         FILE* (usually stdout)
    → renderer_destroy + markcore_free_syntax_tree
```

`markcore_render_to_file()` in `src/markcore.c` is the only orchestration. Do not emit HTML from the parser, and do not parse markdown inside a renderer.

### Parser

- Line-oriented. Each line is copied, NUL-terminated, then classified from the first non-whitespace char: `#` heading, `![...](...)` image, `* ` unordered list, digits+`.`/`)` ordered list, ` ``` ` fence, else a `LINE_NODE`.
- Open blocks (root, lists, fenced code) live on a **file-scope** `node_stack`. `escape_if_in_list()` pops a list when a non-list block starts. `ensure_list_context()` closes the other list type before opening a sibling.
- Inlines (`[link]()`, `*`/`**`/`***`, `` `code` ``) are scanned by `markcore_parse_inline_range`. Successful helpers already advance `p`; the loop must `continue`, not `p++`, or the next delimiter is skipped.
- Nodes own heap strings (`content`, `data`) via `strdup`. The README still wants span-into-original-buffer instead of copying.
- `markcore_free_syntax_tree()` DFS-frees the tree. Callers of `markcore_parse()` own the root.

### Renderer

- `Renderer_t` is a vtable plus borrowed `FILE *outfile` and a context stack of `MCNodeType_e *`.
- `render_syntax_tree()` is the walk. List/code context is `&node->type` on the **AST node** (must outlive the recursive walk — do not push stack locals).
- HTML escaping belongs in the HTML sink: `&<>` in text, plus quotes in attributes. Other sinks should not inherit HTML rules.
- Tight list items are `<li>…</li>` without an inner `<p>`. Paragraphs wrap non-list `LINE_NODE`s.

### AST conventions

| Field | Typical use |
|---|---|
| `content` | visible text (text nodes, link label, image alt, heading body, inline code) |
| `data` | extra string (link/image URL) |
| `header_level` | 1–6 on `HEADER_NODE` only |
| `children` | nested inlines or block children |

Links and images are **flat**: label/alt is `content`, not a child subtree. Emphasis nodes have children. If you add nested inlines inside links, both parser and `LINK_NODE` rendering must change together.

`type_labels` is a `static` array in `types.h` (one copy per TU). Prefer `extern` + a single `.c` if you touch it.

## Commands

CMake is C-only (`project(markcore LANGUAGES C)`). Out-of-tree build:

```sh
cmake -S . -B build -DMARKCORE_BUILD_CLI=ON -DMARKCORE_BUILD_TESTS=ON
cmake --build build
./build/markcore-tests
./build/markcore-cli test2.md
```

AddressSanitizer (apply via `-DADDRESS_SANITIZER=ON`). Prefer **gcc** if clang’s ASan runtime is missing:

```sh
CC=gcc cmake -S . -B build -DMARKCORE_BUILD_CLI=ON -DMARKCORE_BUILD_TESTS=ON \
  -DADDRESS_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ASAN_OPTIONS=detect_leaks=1 ./build/markcore-tests
```

There is no CI workflow in-repo. Run tests (and ASan when touching parser/stack/renderer) before finishing.

Fixtures: `test.md` / `test2.md` are sample documents, not the suite. Add regressions to `tests/test_markcore.c` (public API + `open_memstream`).

## How to change things

**New markdown construct** — add `MCNodeType_e`, parse it in `markcore_parse_line` (block) or `markcore_parse_inline_range` (inline), handle it in `render_syntax_tree`, and implement the HTML callbacks. Update `type_labels`.

**New output format** — clone `src/renderers/html_renderer.c` (fill the vtable). Wire a constructor in `markcore.c` or, better, extend the public API so HTML is not hard-coded there. Do not special-case formats inside `parser.c`.

**Plugin / tree pass** — planned as pre/post walk over `MCNode_t` between parse and render (`README.md`). Insert it in `markcore_render_to_file()` after parse, before `render_syntax_tree()`. The walker must not assume HTML.

**Public API** — keep `include/markcore.h` small. Buffer/options APIs are commented stubs; implement those instead of leaking `parser.h` to callers.

## Constraints and pitfalls

- Parser `node_stack` is global: **not re-entrant or thread-safe**. Always `node_stack = NULL` after `stack_free`. Nested `markcore_parse()` will clobber the outer parse.
- Respect `(markdown, length)`; do not assume a trailing NUL on the caller buffer. Line copies are NUL-terminated for C-string helpers.
- Check `malloc`/`realloc`. `add_child_node` on failure currently drops the child (leak) rather than aborting the parse.
- `Renderer_t.outfile` is borrowed; `renderer_destroy()` must not `fclose` it.
- Unclosed ` ``` ` keeps `CODE_BLOCK_NODE` on the stack until EOF.
- No `_` emphasis, blockquotes, thematic breaks, indented code, or indent-nested lists.
- Style: C11-ish, tabs, `snake_case`, `MC`/`markcore_` prefixes. Match the file you edit. No C++.

## Security

Assume untrusted markdown. HTML output must keep escaping in the HTML sink. Do not emit raw `content`/`data` into tags or attributes. URL scheme allowlists (`javascript:`) are not implemented; mention that if you add HTML features that make it worse.
