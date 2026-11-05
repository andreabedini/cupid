# Inside Cupid

A guided tour of the interpreter's source code, which follows a Haskell
program through the stages that compile and run it.  If you know Haskell and
can read C, you have what it assumes.

Start with [Reading Cupid in a day](reading-cupid-in-a-day.md), which has the
map of the source tree and a shorter route for a first day: five of these
chapters, three of them in part.  The list below is the order for the complete
tour.

**Getting your bearings**

1. [Reading Cupid in a day](reading-cupid-in-a-day.md) — what a day's
   reading gets you, the map of `src/`, and a first-day route
2. [The design of Cupid](design.md) — the aim, and what is inherited from
   Gofer and Hugs

**The path of a program**

3. [How Cupid runs a program](how-cupid-runs-a-program.md) — one small
   program from the bytes in its file to its output
4. [Cells, text and tables](cells-text-and-tables.md) — the one data type,
   the heap, the collector, and the checkpoints that reloading rests on
5. [How source is read](how-source-is-read.md) — the lexer, the grammar's
   shortcuts, and static analysis
6. [How types are inferred](how-types-are-inferred.md) — structure sharing,
   generalisation, and the in-place rewriting that inserts dictionaries
7. [How constraints are solved](how-constraints-are-solved.md) — predicates,
   evidence, entailment and improvement
8. [How code is compiled and run](how-code-is-compiled-and-run.md) — pattern
   matching, lambda lifting, the abstract machine and its instructions

**Around the pipeline**

9. [The running interpreter](the-running-interpreter.md) — the modes, the
   subsystems, interrupts, signals and output
10. [Modules, files and the search path](modules-and-the-search-path.md) —
    how a module name becomes a file
11. [Foreign functions](foreign-functions.md) — stubs, and why callbacks
    into Haskell aren't available
12. [Character encoding](character-encoding.md) — where bytes become
    characters
13. [Platforms and the build](platforms-and-the-build.md) — why nothing is
    probed, and why there are so few options
14. [How the C is written](how-the-c-is-written.md) — the conventions you'll
    see in every file

**Why it is like this**

15. [Why the language is what it is](language-design.md) — how a feature
    gets in, and what stays out
16. [History](history.md) — from Gofer to this tree
