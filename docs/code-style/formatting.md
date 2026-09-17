---
title: Formatting
description: The clang-format baseline every repository starts from, and the reasoning behind the choices whose option name does not explain them.
sidebar:
  order: 1
---

A repository's `.clang-format` is the authoritative statement of its formatting, and the file is short enough to read. What follows is the baseline every repository is created from.

`liara-interfaces` starts from a variant adapted to C: no C++-specific options, and include categories reflecting a header that has to compile standalone.

<details>
<summary>The baseline, in full</summary>

```yaml title=".clang-format"
---
# Liara Engine - clang-format configuration
# See the code style pages for explanations of each section.

BasedOnStyle: LLVM
Language: Cpp
Standard: c++20

# === INDENTATION & SPACING ===
IndentWidth: 4
TabWidth: 4
UseTab: Never
ContinuationIndentWidth: 4
ConstructorInitializerIndentWidth: 4
IndentCaseLabels: true
IndentPPDirectives: BeforeHash
AccessModifierOffset: -4

# === LINE LENGTH ===
ColumnLimit: 120
ReflowComments: Always
AlignTrailingComments: true
SpacesBeforeTrailingComments: 2

# === BRACES ===
BreakBeforeBraces: Custom
BraceWrapping:
    AfterCaseLabel: false
    AfterClass: true
    AfterControlStatement: Never
    AfterEnum: true
    AfterFunction: false
    AfterNamespace: true
    AfterStruct: true
    AfterUnion: true
    AfterExternBlock: false
    BeforeCatch: true
    BeforeElse: true
    BeforeWhile: true
    IndentBraces: false
    SplitEmptyFunction: false
    SplitEmptyRecord: false
    SplitEmptyNamespace: true

# === SPACING ===
SpaceAfterCStyleCast: false
SpaceAfterLogicalNot: false
SpaceAfterTemplateKeyword: false
SpaceBeforeAssignmentOperators: true
SpaceBeforeCpp11BracedList: true
SpaceBeforeCtorInitializerColon: true
SpaceBeforeInheritanceColon: true
SpaceBeforeParens: ControlStatements
SpaceBeforeRangeBasedForLoopColon: true
SpacesInAngles: Never
SpacesInContainerLiterals: false
SpacesInSquareBrackets: false

SpacesInParens: Custom
SpacesInParensOptions:
    InEmptyParentheses: false
    InCStyleCasts: false

# === POINTERS & REFERENCES ===
PointerAlignment: Left
ReferenceAlignment: Left

# === INCLUDES ===
SortIncludes: CaseInsensitive
IncludeBlocks: Regroup
IncludeCategories:
    # 0. Special headers that must come first
    - Regex: '^<SDL\.h>$'
      Priority: 0
      SortPriority: 0
    # 1. Module's own header (corresponding .h for a .cpp)
    - Regex: '^"[^/]+\.h"$'
      Priority: 1
      SortPriority: 1
    # 2. Other headers from the same module
    - Regex: '^"[^"]+/[^"]+\.h"$'
      Priority: 2
      SortPriority: 2
    # 3. Liara public interface headers
    - Regex: '^<liara/.*>$'
      Priority: 3
      SortPriority: 3
    # 4. Vulkan, SDL, ImGui, GLM and other third-party with C-style includes
    - Regex: '^<(vulkan|SDL|imgui|glm|cgltf)/.*>$'
      Priority: 4
      SortPriority: 4
    # 5. C++ standard library
    - Regex: '^<[a-z_]+>$'
      Priority: 5
      SortPriority: 5
    # 6. C standard library
    - Regex: '^<[a-z_]+\.h>$'
      Priority: 6
      SortPriority: 6
    # 7. Other (catch-all)
    - Regex: '.*'
      Priority: 7
      SortPriority: 7

# === ALIGNMENT ===
AlignAfterOpenBracket: true
AlignArrayOfStructures: Right
AlignConsecutiveAssignments: false
AlignConsecutiveBitFields: Consecutive
AlignConsecutiveDeclarations: false
AlignConsecutiveMacros: Consecutive
AlignEscapedNewlines: Left
AlignOperands: Align

# === BREAK / WRAP ===
BreakAfterReturnType: Automatic
PenaltyReturnTypeOnItsOwnLine: 100000
AlwaysBreakBeforeMultilineStrings: false
AlwaysBreakTemplateDeclarations: Yes
BinPackArguments: false
BinPackParameters: false
AllowAllArgumentsOnNextLine: false
AllowAllParametersOfDeclarationOnNextLine: false
BreakBeforeBinaryOperators: NonAssignment
BreakBeforeConceptDeclarations: Always
BreakBeforeTernaryOperators: true
BreakConstructorInitializers: BeforeComma
BreakInheritanceList: BeforeComma
BreakStringLiterals: true
PackConstructorInitializers: Never

# === LAMBDA ===
AllowShortLambdasOnASingleLine: All
LambdaBodyIndentation: Signature

# === CONTROL FLOW ===
AllowShortBlocksOnASingleLine: Always
AllowShortCaseLabelsOnASingleLine: true
AllowShortEnumsOnASingleLine: true
AllowShortFunctionsOnASingleLine: All
AllowShortIfStatementsOnASingleLine: AllIfsAndElse
AllowShortLoopsOnASingleLine: true

# === EMPTY LINES ===
EmptyLineAfterAccessModifier: Never
EmptyLineBeforeAccessModifier: LogicalBlock
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
SeparateDefinitionBlocks: Always

# === MISC ===
CompactNamespaces: false
FixNamespaceComments: true
NamespaceIndentation: All
SortUsingDeclarations: true
```

</details>

## The choices worth explaining

Options whose meaning is obvious from the name are not discussed. Six are not obvious.

**`ColumnLimit: 120`** is long enough to avoid constant wrapping in code where templates and namespaced types are verbose by construction, and short enough to keep two files side by side on a normal monitor.

**`IndentWidth: 4` with `UseTab: Never`** is four spaces, never tabs. A tab renders differently depending on the editor, and a space does not.

**The custom brace style** puts the opening brace on a new line for classes, structs, enums and namespaces, and keeps it on the same line for functions and control statements. It matches what the previous engine used, and it is close to Microsoft's.

**`PointerAlignment: Left` and `ReferenceAlignment: Left`** give `Type* var` and `Type& var` rather than `Type *var`, because the `*` and the `&` belong to the type.

**`IncludeBlocks: Regroup` with explicit categories** sorts includes into defined groups separated by blank lines. That prevents merge conflicts on include lists and makes the origin of every header visible at a glance.

**`BinPackArguments: false` and `BinPackParameters: false`** put each argument on its own line once they stop fitting on one. Slightly more verbose, and it produces a clean diff when an argument is added or removed rather than a reflow of the whole call.

The short-form allowances are worth reading together, since `AllowShortBlocksOnASingleLine: Always`, `AllowShortFunctionsOnASingleLine: All` and `AllowShortIfStatementsOnASingleLine: AllIfsAndElse` are what produce the one-line guard clauses the shim examples are full of.

## Adapting it

A module may change any of this. The two changes most likely to be justified are `IncludeCategories`, which depends on which third-party libraries the module actually includes, and `ColumnLimit`, when a module's material genuinely reads better wider or narrower. Everything else is more likely to be a preference than a need, which does not make it wrong, only worth a moment's thought.
