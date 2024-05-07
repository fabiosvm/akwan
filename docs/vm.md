
# Virtual Machine

## Instruction Set

Following is the instruction set:

| Opcode   | Operand | Description                          |
| -------- | ------- | ------------------------------------ |
| `Nil`    |         | Push a `nil` value onto the stack    |
| `False`  |         | Push a `false` value onto the stack  | 
| `True`   |         | Push a `true` value onto the stack   |
| `Const`  | _index_ | Push a constant value onto the stack |
| `Load`   | _index_ | Load a value from the stack          |
| `Store`  | _index_ | Store a value onto the stack         |
| `Add`    |         | Add two values                       |
| `Sub`    |         | Subtract two values                  |
| `Mul`    |         | Multiply two values                  |
| `Div`    |         | Divide two values                    |
| `Mod`    |         | Modulo of two values                 |
| `Neg`    |         | Negate a value                       |
| `Return` |         | Return from the function             |
