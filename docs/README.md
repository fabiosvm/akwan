
# The Akwan Programming Language

Akwan is a simple, dynamically typed scripting language with mutable value semantics, designed for flexible data manipulation and cross-platform compatibility.

> **Note:** This documentation maybe doesn't reflect the current state of the project.

## Basic Types

Akwan has few basic types:

- `Nil`: Represents the absence of a value. Has only one value, `nil`.
- `Bool`: Represents a boolean value (`true` or `false`).
- `Int`: Represents a 64-bit signed integer.
- `Number`: Represents a 64-bit floating-point number.
- `Char`: Represents a single Unicode character.
- `String`: Represents a sequence of `Char` values.
- `Range`: Represents a range of integers.
- `Array`: Represents a heterogeneous sequence of values.
- `Ref`: A special type that holds a reference to a value.

> **Note:** The `Int` is stored exactly as `Number` internally.

## Variables

Variables in Akwan are declared using the `let` keyword:

```rs
let x = 10;
let text = "Hello, world!";
println(x); // 10
println(text); // Hello, world!
```

### Scope

In Akwan, all variables are local to the block where they are declared:

```rs
  let x = 10;
  {
    let y = 20;
    println(x); // 10
  }
  println(y); // Error: variable 'y' used but not defined
```

### Shadowing

Shadowing is allowed:

```rs
let x = 10;
{
  let x = 20;
  println(x); // 20
}
println(x); // 10
```

## Assignment

Variables can be reassigned:

```rs
let x = 10;
x = 20;
println(x); // 20
```
## Arithmetic Operators

Akwan has the following arithmetic operators:

| Operator    | Description    |
| ----------- | -------------- |
| `+`         | Addition       |
| `-`         | Subtraction    |
| `*`         | Multiplication |
| `/`         | Division       |
| `%`         | Modulus        |
| `-` (unary) | Negation       |

```rs
println(1 + 2 * 3); // 7
println((1 + 2) * 3); // 9
println(10 / 3); // 3
println(10 % 3); // 1
println(-10); // -10
```

## Ranges

Ranges are created using the `..` operator:

```rs
let r = 1..5;
println(r); // 1..5
println(len(r)); // 4
```

> **Note:** The range is inclusive on the start and exclusive on the end.

### Indexing a range

Ranges are zero-based indexed:

```rs
let r = 1..5;
println(r[0]); // 1
println(r[3]); // 4
println(r[4]); // Error: index out of range
```

## Arrays

Arrays are created using brackets `[]`:

```rs
let a = [1, 2, 3];
println(a); // [1, 2, 3]
println(len(a)); // 3
```

### Indexing an array

Arrays are zero-based indexed:

```rs
let a = [1, 2, 3];
println(a[0]); // 1
println(a[1]); // 2
println(a[2]); // 3
println(a[3]); // Error: index out of range
```

### Slicing an array

Arrays can be sliced passing a range inside brackets:

```rs
let a = [1, 2, 3];
println(a[0..2]); // [1, 2]
```
