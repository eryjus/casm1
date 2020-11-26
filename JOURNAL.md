# Century Assembler (1-pass version)

This is my usual journal related to the projects I write.  As usual, this journal is meant for me as much as it is meant for you.  I use this journal to review my thinking and conclusions as I work through any given project.  You can read this journal to observe my learning experience and hopefully get something from it.

Please see the [README](README.md) file for more information about what is driving my work in this project.

## Project 1-1

### 2020-Nov-25

Project 1-1 is supposed to be the foundational project for starting the assembler.  It has 2 assemblers to be built: a 1-pass and a 2-pass assembler.  This project will be the 1-pass assembler.  The project is supposed to be simple, so I should not have to put too much effort into a complicated solution.  I really should be able to get in, write the code, and get back out again quickly.  The only thing I think I want to put any time into would be the assembly tables.  For this fictitious system, the tables are small.

The assembler language is as follows:

| mnem | OpCode | operand | description |
|:----:|:------:|:-------:|:------------|
| LOD  | 1      | yes     | Acc←Mem(op) |
| STO  | 2      | yes     | Mem(op)←Acc |
| ADD  | 3      | yes     | Acc←Acc+Mem(op) |
| BZE  | 4      | yes     | branch to Op if Acc=0 |
| BNE  | 5      | yes     | same for Acc<0 |
| BRA  | 6      | yes     | unconditional branch |
| INP  | 7      | no      | Acc←the next character in the input stream |
| OUT  | 8      | no      | next char in output stream←Acc |
| CLA  | 9      | no      | Acc←0 |
| HLT  | 0      | no      | stop |

And the following is a (meaningless) test program:

```s
  INP
  STO 50
  INP
  STO 51
  BZE X
  ADD 50
  OUT
  BRA Y
X LOD 50
  ADD 50
Y STO 52
  HLT
```

The tutorial suggests making some decisions on how to set this line up for ease of parsing.  Since the labels are all single letter and all the opcodes are all 3 letters, this becomes very easy with a fixed format:
* The first position (index 0) in the line either has a label or is blank
* The second position (index 1) in the line is blank
* Positions 2-4 contain the opcode
* Position 5 is blank or does not exist (null)
* Position 6+ contain any optional operand

So, I will also need a symbol table.  This should be able to be a simple array of status/location structures where label `'A'` is index `0` and so on.

Finally, only one Location Counter (`LC`) will be needed.

I think the first thing to put into place here are some error output functions.  These need to be relatively simple and there are only a few to consider:
* `fatal(...)`
* `error(...)`
* `warning(...)`

These should all be able to simply redirect to `fprintf(stderr, ...)`

---

The next step is to take on the Symbol Table.  This is trivial -- an array of 26 entries.

What needs to be tracked for each symbol?  This is also trivial:
* The status of the symbol (unused, undefined, defined)
* It location (when undefined, the next location to be updated; when defined the actual location in memory)

So, the functions that need to take place are:
* `char symbolSearch()` -- returns U or D or (null)
* `uint16_t symbolGetValue()` -- return the value or -1 (expect that only valid symbols are searched)
* `bool symbolAddDefined()` -- returns non-conflicted add
* `bool symbolAddUndefined()` -- returns non-conflicted add

Of course, there needs to be some basic sanity checks in the symbol functions.  These will be fatal problems as the errors should have been trapped prior to trying to interact with the symbol table.

---

Opcodes are next up.  This is a duplicate of the table above.

The functions for this portion will be the following:
* `const struct OpCode_t const *GetOpCode(const char *mnem)`
* `bool IsInvalid(struct OpCode_t *o)`

---

So with that, the next step is to start to lay out the assembler using some to-down development techniques.



