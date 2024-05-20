
# Virtual Machine

## Instruction Set

Following is the instruction set:

| Opcode          | Operand | Description                          |
| --------------- | ------- | ------------------------------------ |
| `Nil`           |         | Push a `nil` value onto the stack    |
| `False`         |         | Push a `false` value onto the stack  | 
| `True`          |         | Push a `true` value onto the stack   |
| `Int`           | _data_  | Push a 8-bit integer onto the stack  |
| `Const`         | _index_ | Push a constant value onto the stack |
| `Range`         |         | Create a range from two integers     |
| `Array`         | _n_     | Create an array of size _n_          |
| `LocalRef`      | _index_ | Push a reference to a local variable |
| `ElementRef`    |         | Push a reference to an array element |
| `Pop`           |         | Discard the top value from the stack |
| `GetLocal`      | _index_ | Get a local variable                 |
| `SetLocal`      | _index_ | Set a local variable                 |
| `GetLocalByRef` | _index_ | Get a local variable by reference    |
| `SetLocalByRef` | _index_ | Set a local variable by reference    |
| `GetElement`    |         | Get an element from an array         |
| `Add`           |         | Add two values                       |
| `Sub`           |         | Subtract two values                  |
| `Mul`           |         | Multiply two values                  |
| `Div`           |         | Divide two values                    |
| `Mod`           |         | Modulo of two values                 |
| `Neg`           |         | Negate a value                       |
| `Return`        |         | Return from the function             |
