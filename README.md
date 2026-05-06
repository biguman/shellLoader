# shellLoader

A Windows shellcode loader PoC demonstrating XOR-encrypted payload delivery with a runtime-derived key. The key is never hardcoded — it is generated at runtime from heap allocation arithmetic, a technique developed in [string_resolver](https://github.com/biguman/string_resolver).

The payload is Metasploit calc.exe shellcode (msfvenom x64 Windows) — one of the most heavily signatured payloads in existence. The loader bypasses Windows Defender on Windows 11 build 25H2 with it. The encryption alone isn't novel; what keeps it clean is that the key never exists as a constant in the binary and is only materialized at runtime from heap behavior that a static scanner can't replicate.

---

## How it works

### Key derivation — heap arithmetic

`XOR_maker()` in `obf.c` derives a deterministic 64-bit key from the memory allocator's behavior:

1. Allocate 9 heap blocks with alternating sizes:
   - Even indices: `sizeof(short) * i * 47` bytes
   - Odd indices: `sizeof(char) * 193` bytes
2. For each of the first 8 blocks, compute a delta against the last pointer:
   ```
   sum_list[i] = (uint64_t)ptr_list[8] - ((uint64_t)ptr_list[i] >> 4)
   ```
3. Take the LSB of each delta and pack them byte-by-byte into a `uint64_t`:
   ```
   key |= (sum_list[i] & 0xFF) << (i * 8)
   ```

The allocator's consistent block placement means this key reproduces on the same OS and heap configuration. The key differs enough on sandboxes, VMs, or different allocators that the payload won't decrypt correctly — a passive anti-emulation property.

See [string_resolver](https://github.com/biguman/string_resolver) for where this technique was worked out.

### Encryption (`enc.c`)

Prep step, run once offline:

```c
encr[i] = buf[i] ^ ((key >> ((i % 8) * 8)) & 0xff);
```

Each plaintext byte is XORed against one byte of the 64-bit key, cycling through all 8 bytes. The resulting encrypted blob is embedded as the `payload_encrypted` array in `loader.c`.

### Loader (`loader.c`)

At runtime:

1. `VirtualAlloc` — allocate RW memory for the payload
2. `XOR_maker(&key)` — regenerate the same 64-bit key from heap deltas
3. Decrypt in a loop using the same rotating-byte XOR
4. `VirtualProtect` — flip the region to RX
5. Cast to function pointer and call

```
[ encrypted blob ] --XOR(heap-derived key)--> [ shellcode ] --> VirtualProtect(RX) --> exec()
```

---

## Files

| File | Purpose |
|------|---------|
| `loader.c` | Main loader — decrypt and execute the embedded shellcode |
| `message.c` | Alternate loader variant with a different encrypted payload, shellcode for a popup window, created with "msfvenom -p windows/x64/messagebox TEXT="pwned" TITLE="noob" -f c" |
| `enc.c` | Offline encryption utility — XORs raw shellcode to produce the blob |
| `obf.c` | `XOR_maker()` and `trusted_machine()` — key derivation and demo string decode |
| `obf.h` | Header for `obf.c` |

---

## Build

Requires MinGW (`x86_64-w64-mingw32-gcc`) for the loaders and `gcc` for the encryptor.

```bash
make        # builds loader.exe and message.exe
make enc    # builds the encryptor (native, no MinGW needed)
make clean
```

---

## Swapping payloads

1. Generate new shellcode (e.g. `msfvenom -p windows/x64/exec CMD=calc.exe -f c`)
2. Paste the shellcode bytes into the `buf[]` array in `enc.c`
3. Build and run the encryptor: `make enc && ./enc`
4. Copy the printed hex output into `loader.c` as the `payload_encrypted[]` array
5. Rebuild the loader: `make loader.exe`

---

## Notes

- The heap arithmetic key derivation is allocator-dependent. It was tested with Windows `HeapCreate`/`HeapAlloc` (via the `#ifdef _WIN32` branch in `obf.c`). Linux uses `malloc` as a fallback but the key will differ.
- ASLR affects base addresses but not inter-block deltas, which stay consistent within a run — that's what the key depends on.
- This is a learning/PoC project. The same `XOR_maker` logic from `string_resolver` is reused here with different allocation sizes (scaled up for more entropy in the key bytes).
- The `printf` calls in `XOR_maker` (pointer addresses and derived key) are intentional — this is a PoC and the output is useful for understanding and verifying the key derivation step by step.
