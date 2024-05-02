
# Virtual Machine

## Instruction Formats

There are four instruction formats in the VM. The formats are as follows:

```
      0       7       15      23      31
      +-------+-------+-------+-------+
Fmt 0 | -     | -     | -     | op    |
Fmt 1 | -     | -     | a     | op    |
Fmt 2 | -     | b     | a     | op    |
Fmt 3 | c     | b     | a     | op    |
      +-------+-------+-------+-------+
```

## Instruction Set

Following is the instruction set:

| Opcode   | Fmt | Operands    | Description                                    |
| -------- | --- | ----------- | ---------------------------------------------- |
| `Nil`    | 1   | _d_         | Move `nil` to a slot                           |
| `False`  | 1   | _d_         | Move `false` to a slot                         |
| `True`   | 1   | _d_         | Move `true` to a slot                          |
| `Const`  | 2   | _d, idx_    | Move a value from the constant table to a slot |
| `Move`   | 2   | _d, s_      | Move a value from one slot to another          |
| `Add`    | 3   | _d, s1, s2_ | Add two values                                 |
| `Sub`    | 3   | _d, s1, s2_ | Subtract two values                            |
| `Mul`    | 3   | _d, s1, s2_ | Multiply two values                            |
| `Div`    | 3   | _d, s1, s2_ | Divide two values                              |
| `Mod`    | 3   | _d, s1, s2_ | Modulo of two values                           |
| `Neg`    | 2   | _d, s_      | Negate a value                                 |
| `Return` | 0   | -           | Return from the function                       |
