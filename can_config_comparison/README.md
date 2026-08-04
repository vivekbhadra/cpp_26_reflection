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
For instructions on setting up GCC 16.1 and compiling your first C++26 program, refer to
<a href="https://techfortalk.co.uk/2026/05/04/compile-your-first-c26-program-with-gcc-16-1/" target="_blank" rel="noopener noreferrer">this guide</a>.     
```bash
# Manual version
g++ -std=c++17 manual_serialisation.cpp -o can_config_manual \
    -lPocoJSON -lPocoFoundation

# Reflection version
export PATH=/opt/gcc-16.1/bin:$PATH
g++ -std=c++26 -freflection -Wl,-rpath,/opt/gcc-16.1/lib64 \
    reflective_serialisation.cpp -o can_config_reflection \
    -lPocoJSON -lPocoFoundation
```

If Poco isn't on your default include/lib path, add
`-I/path/to/poco/include -L/path/to/poco/lib`. If your distro ships a
`poco-json.pc` file you can use `pkg-config --cflags --libs poco-json`
instead of the bare `-l` flags (Ubuntu's `libpoco-dev` package does not
ship one, so this repo defaults to the bare flags).

## Verification status

- **`manual_serialisation.cpp`** — compiled and run successfully with
  `g++ 13.3.0 -std=c++17` against `libpoco-dev 1.11.0` on Ubuntu 24.04.
  Confirmed output:
  ```json
  {
      "bitrate": 250000,
      "canLine": "can1",
      "extendedFrame": true,
      "id": "can-source-1",
      "pgnPassFilter": ["FEF1", "FEEE"],
      "retryPolicy": {"backoffMs": 500, "jitterEnabled": true, "maxAttempts": 3}
  }
  ```
- **`reflective_serialisation.cpp`** — **not compiled in this repo's test
  environment**, since GCC 16.1 with `-freflection` isn't available here.
  Reviewed for API consistency with GCC 16.1's documented P2996 support
  (`std::meta::nonstatic_data_members_of`, `access_context::unchecked()`,
  `identifier_of`, `template for`, `^^`/`[: :]`). **Build it against your
  own GCC 16.1 install and confirm output matches the manual version
  before publishing.**

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
