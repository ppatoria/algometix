# Understanding `const`, Read-Only Variables, and Constant Expressions in Rust and C++

## Introduction
Constants in Rust and C++ are essential for ensuring efficiency, safety, and correctness. However, the two languages handle constants, read-only variables, and constant expressions differently. This article explores:
- The meaning of `const`, read-only variables, and constant expressions.
- Their differences in Rust and C++.
- When and how they are evaluated.
- The default mutability of variables in both languages.

## `const` in Rust and C++

### Rust
In Rust, `const` defines a **compile-time constant** that cannot change. It is always evaluated at compile-time and must have a known value.
```rust
const PI: f64 = 3.14159;
const N: usize = 1024;
```
- `const` variables **must** have a value that can be determined at compile-time.
- `const` is **immutable by default** and does not require `mut`.
- Used for defining **global constants** or **constants inside functions**.

### C++
In C++, `const` is a **read-only variable** that prevents modification, but **it does not guarantee compile-time evaluation**.

**Compile time evaluation:**
```cpp
const int PI = 3.14159;
const int N = 1024;
```

**Runtime evaluation:**
```cpp
int getValue() {
    return 10;
}

void example() {
    const int x = getValue(); // `x` is `const`, but its value is set at runtime
}
```
- `const` ensures the variable is **read-only** but might still be assigned at runtime.
- If assigned a runtime value, it **will not** be a compile-time constant.

### Key Differences
| Feature        | Rust `const`                       | C++ `const`                                                     |
|----------------|------------------------------------|-----------------------------------------------------------------|
| **Evaluation** | Always at compile-time             | May be runtime-evaluated                                        |
| **Mutability** | Immutable by default               | Read-only but can be runtime-evaluated                          |
| **Scope**      | Local or global                    | Local or global                                                 |
| **Usage**      | Used anywhere a constant is needed | Read-only variable, but not necessarily a compile-time constant |

## Read-Only Variables
A **read-only** variable is one that **cannot be modified after initialization** but might still be set at runtime.

### Rust
Rust does not have a separate concept of "read-only variables"—everything is **immutable by default**. 
If you declare a variable with `let`, it is automatically read-only unless explicitly marked `mut`.
```rust
let x = 10;
x = 20; //  Error: Cannot assign to `x`
```

### C++
In C++, `const` variables **can** be initialized at runtime but **cannot be changed** after initialization.
```cpp
int getValue() { return 10; }

void example() {
    const int x = getValue(); // x is read-only but evaluated at runtime
    x = 20; // Error: Cannot assign to `x`
}
```

### Key Differences
| Feature                            | Rust `let` (default) | C++ `const`                    |
|------------------------------------|----------------------|--------------------------------|
| **Default behavior**               | Immutable            | Read-only after initialization |
| **Can be initialized at runtime?** | Yes                  | Yes                            |
| **Can be modified later?**         | No                   | No                             |

## Constant Expressions (`constexpr` in C++, `const fn` in Rust)
A **constant expression** is a value that **must be computed at compile-time**.

### C++ (`constexpr`)
In C++, `constexpr` ensures that a value **is always evaluated at compile-time**.
```cpp
constexpr int square(int x) {
    return x * x;  // Guaranteed compile-time evaluation
}

constexpr int y = square(5); // OK: Evaluated at compile-time
```
- `constexpr` guarantees that the function runs **at compile-time**.
- Useful for defining **array sizes** and **constant values**.

### Rust (`const fn`)
Rust provides `const fn` for compile-time function evaluation, similar to `constexpr`.
```rust
const fn square(x: usize) -> usize {
    x * x // Compile-time evaluation
}

const Y: usize = square(5); // OK: Evaluated at compile-time
```
- `const fn` ensures **compile-time execution**.
- Useful for defining **constant expressions** and **precomputed values**.

### Key Differences
| Feature                 | Rust `const fn`                      | C++ `constexpr`                             |
|-------------------------|--------------------------------------|---------------------------------------------|
| **Function evaluation** | Compile-time only                    | Compile-time only                           |
| **Usage**               | `const` variables, array sizes, etc. | `constexpr` variables, function calls, etc. |
| **Default behavior**    | Compile-time by default              | Requires `constexpr` keyword                |

## Default Mutability in Rust vs. C++
One of the biggest differences between Rust and C++ is **default mutability**.

### Rust (Immutable by Default)
```rust
let x = 10;
x = 20; // Error: Cannot assign to `x`

let mut y = 10;
y = 20; // Allowed
```
- **`let` variables are immutable by default**.
- To allow modification, `mut` must be used explicitly.

### C++ (Mutable by Default)
```cpp
int x = 10;
x = 20; // Allowed

const int y = 10;
y = 20; // Error: Read-only
```
- **Variables are mutable by default**.
- `const` must be explicitly used to make them immutable.

### Key Differences
| Feature                 | Rust             | C++              |
|-------------------------|------------------|------------------|
| **Default Mutability**  | Immutable        | Mutable          |
| **Making it Mutable**   | Use `mut`        | Default behavior |
| **Making it Immutable** | Default behavior | Use `const`      |

## Summary of Differences
| Feature                                  | Rust `const`      | C++ `const`             | C++ `constexpr` |
|------------------------------------------|-------------------|-------------------------|-----------------|
| **Compile-time guarantee**               | Yes               | No (may be runtime)     | Yes             |
| **Runtime assignment allowed?**          | No                | Yes                     | No              |
| **Used in constant expressions?**        | Yes               | No                      | Yes             |
| **Functions evaluated at compile-time?** | `const fn`        | No                      | `constexpr`     |
| **Default variable mutability**          | Immutable (`let`) | Mutable (`int x = 10;`) |                 |

### Rust `const` vs C++ `constexpr`
**Rust's `const` is equivalent to C++'s `constexpr`** because both ensure **compile-time evaluation** but there are subtle differences like:

**Default Immutability**
Rust `const` is Always Immutable, and you cannot modify it.
```rust
const fn foo() -> usize {
    const x: usize = 10;  
    x = 20;  // ERROR: Cannot mutate a constant value
    return x;
}
```
Rust enforces immutability for `const` values.

C++ `constexpr` Can Be Mutable Inside Functions
C++ allows `constexpr` mutable objects inside functions.
```cpp
constexpr int foo() {
    int x = 10;  // Allowed inside constexpr function
    x = 20;  // Allowed, but must be resolved at compile-time
    return x;
}

int main() {
    constexpr int result = foo(); // Compile-time evaluation
}
```

## Conclusion
- **Rust `const` is like C++ `constexpr`**: It **must** be evaluated at compile-time.
- **C++ `const` is only read-only**, but it **might be evaluated at runtime**.
- **Rust variables are immutable by default**, whereas **C++ variables are mutable by default**.
- **Rust `const fn` and C++ `constexpr` functions** ensure compile-time function execution.

By understanding these differences, you can make better choices when writing performant, safe, and correct code in Rust and C++.
