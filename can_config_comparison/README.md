# can_config_comparison

Manual vs. C++26 reflection-based JSON serialization, same domain type.

## Files

- `domain.h`:  shared `CanDataSource` / `RetryPolicy` definitions. Plain
  data only, no serialization logic.
- `manual_serialisation.cpp`: today's C++ (C++17): hand-written,
  field-by-field JSON mapping via `ManualJson`.
- `reflective_serialisation.cpp`: C++26 (P2996 reflection): one generic
  `to_json<T>()` that walks any struct's members automatically.

## Prerequisites

- Poco JSON library (`libpoco-dev` on Debian/Ubuntu, or equivalent)
- A C++17 compiler for the manual version (any recent GCC/Clang)
- GCC 16.1+ built with `-freflection` support for the reflection version

## Build
For instructions on setting up GCC 16.1 and compiling your first C++26 program, see [Compile Your First C++26 Program with GCC 16.1](https://techfortalk.co.uk/2026/05/04/compile-your-first-c26-program-with-gcc-16-1/).       
```bash
# Manual version
g++ -std=c++17 manual_serialisation.cpp -o can_config_manual \
    -lPocoJSON -lPocoFoundation

# Reflection version
export PATH=/opt/gcc-16.1/bin:$PATH  
export LD_LIBRARY_PATH=/opt/gcc-16.1/lib64:$LD_LIBRARY_PATH  
g++ -std=c++26 -freflection -Wl,-rpath,/opt/gcc-16.1/lib64 \
    reflective_serialisation.cpp -o can_config_reflection \
    -lPocoJSON -lPocoFoundation
```

If Poco isn't on your default include/lib path, add
`-I/path/to/poco/include -L/path/to/poco/lib`. If your distro ships a
`poco-json.pc` file you can use `pkg-config --cflags --libs poco-json`
instead of the bare `-l` flags (Ubuntu's `libpoco-dev` package does not
ship one, so this repo defaults to the bare flags).

## Run

```bash
./can_config_manual
./can_config_reflection
```

Both should print identical JSON.

## Verify identical output

```bash
diff <(./can_config_manual) <(./can_config_reflection)
echo "exit: $?"   # 0 = identical
```

## The actual demonstration: add a field

Edit `domain.h`, add one member to `CanDataSource` (private section and
constructor):

```cpp
std::string m_protocol;
```

Rebuild both binaries.

- `can_config_manual` compiles cleanly and silently omits `"protocol"`
  from its output — `ManualJson::toJson` was never told the field exists.
- `can_config_reflection` picks up the new field automatically, with
  zero changes to `reflective_serialisation.cpp`.

```bash
./can_config_manual | grep protocol       # nothing
./can_config_reflection | grep protocol   # present
```

That gap — same one-line domain edit, silently incomplete vs.
automatically correct — is the point of this example.
