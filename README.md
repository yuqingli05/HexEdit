# 🔧 HexEdit

> A command-line tool for Hex/Bin file editing, built for MCU developers
> Merge · Delete · Cut · Fill · Format Conversion

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🔄 **Format Conversion** | Bin ↔ Hex conversion with custom start address |
| ➕ **File Merge** | Merge up to 64 Hex/Bin files into one output |
| ✂️ **Region Delete** | Delete data at a specified address range |
| 🔍 **Region Cut** | Extract data from a specified address range |
| 🎨 **Region Fill** | Write a specified byte to an address range (zero-fill / pad) |
| 📦 **Smart Padding** | Auto-fill gaps between Bin files, default `0xFF` |

---

## 🚀 Quick Start

### Basic Usage

```bash
HexEdit.exe [command] -f <input file> -o <output file>
```

### Simplest Examples

```bash
# Bin to Hex (STM32 default Flash start address 0x08000000)
HexEdit.exe -x 0x08000000 -f firmware.bin -o firmware.hex

# Hex to Bin
HexEdit.exe -f firmware.hex -o firmware.bin
```

---

## 📖 Command Reference

### Commands

| Command | Description | Format |
|---------|-------------|--------|
| `--add` | **Merge** files (default operation) | `--add -f a.bin -f b.bin -o out.hex` |
| `--del` | **Delete** a specified address range | `--del <addr>:<length>` |
| `--cut` | **Cut** a specified address range | `--cut <addr>:<length>` |
| `--set` | **Fill** a specified address range | `--set <addr>:<length>:<byte>` |
| `--version` | Print version info | |
| `--help` | Print help info | |

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `-f` | Input file (up to 64) | `-f a.bin -f b.hex` |
| `-o` | Output filename | `-o output.hex` |
| `-b` | Set subsequent **input and output** files to **Bin** format | `-b -f a.bin` |
| `-h` | Set subsequent **input and output** files to **Hex** format | `-h -f a.hex` |
| `-x` | Set **start address** for Bin file (must come before `-f`), appended directly if omitted | `-x 0x08000000` |
| `-i` | Set **fill byte** for Bin output (`0xFFFFFFFF` for split output) | `-i 0x25` |

### Address Format

```
<hex address>:<decimal length>[:<hex byte>]
```

Example: `0x08000000:4096:0xFF` → Start at address `0x08000000` (STM32 Flash start), length `4096` bytes, fill value `0xFF`

### File Type Detection Rules

⚠️ **Important**: Once `-b` or `-h` is specified, all subsequent **input and output** files use that type until `-b` or `-h` is encountered again.

- Not specified: `.bin` extension → Bin, others → Hex
- `-b` specified: all subsequent files treated as Bin
- `-h` specified: all subsequent files treated as Hex

---

## 💡 Usage Examples

### 📎 File Merge

#### Case 1: Bin + Bin → Hex

Merge two Bin files at specified addresses, output a Hex file. Typical use case: BootLoader + APP firmware.

```
boot.bin → start address 0x08000000 (BootLoader)
app.bin  → start address 0x08010000 (Application)
```

```bash
HexEdit.exe --add -x 0x08000000 -f boot.bin -x 0x08010000 -f app.bin -o output.hex
```

#### Case 2: Bin + Bin → Bin

```
boot.bin → start address 0x08000000 (BootLoader)
app.bin  → start address 0x08010000 (Application)
```

```bash
HexEdit.exe --add -x 0x08000000 -f boot.bin -x 0x08010000 -f app.bin -o output.bin
```

💡 Bin files have no address info. If `boot.bin` is shorter than `0x10000` (64KB), gaps are filled with `0xFF` by default. Use `-i` to customize the fill byte, set `0xFFFFFFFF` to split output into separate files.

#### Case 3: Hex + Hex → Hex

Merge two Hex files into one Hex file.

```bash
HexEdit.exe --add -f boot.hex -f app.hex -o output.hex
```

#### Case 4: Hex + Bin → Hex

Merge a Hex file with a Bin file, output a Hex file.

```
boot.hex → Hex file (contains addresses)
app.bin  → start address 0x08010000 (Application)
```

```bash
HexEdit.exe --add -h -f boot.hex -b -x 0x08010000 -f app.bin -o output.hex
```

---

### ✂️ Data Delete

#### Case 1: Delete from Bin File

Delete `256` bytes starting at address `0x08010000`.

```bash
HexEdit.exe --del 0x08010000:256 -x 0x08010000 -f app.bin -o output.hex
```

⚠️ Recommended to output Hex file. Deleting from the middle of a Bin file will split it into two segments.

#### Case 2: Delete from Hex File

Delete `256` bytes starting at address `0x08010000` (`0x08010000` ~ `0x080100FF`).

```bash
HexEdit.exe --del 0x08010000:256 -f app.hex -o output.hex
```

---

### 🔍 Data Cut

#### Case 1: Cut from Bin File

Extract `4096` bytes (1 Flash Page) starting at address `0x08010000`.

```bash
# Output Hex
HexEdit.exe --cut 0x08010000:4096 -x 0x08010000 -f app.bin -o output.hex

# Output Bin
HexEdit.exe --cut 0x08010000:4096 -x 0x08010000 -f app.bin -o output.bin
```

#### Case 2: Cut from Hex File

Extract `4096` bytes starting at address `0x08010000` (`0x08010000` ~ `0x08010FFF`).

```bash
# Output Hex
HexEdit.exe --cut 0x08010000:4096 -f app.hex -o output.hex

# Output Bin
HexEdit.exe --cut 0x08010000:4096 -f app.hex -o output.bin
```

---

### 🎨 Data Fill

#### Case 1: Zero-fill Bin File

Set `4096` bytes starting at `0x08010000` to `0x00`.

```bash
# Output Hex
HexEdit.exe --set 0x08010000:4096:0x00 -x 0x08010000 -f app.bin -o output.hex

# Output Bin
HexEdit.exe --set 0x08010000:4096:0x00 -x 0x08010000 -f app.bin -o output.bin
```

#### Case 2: Fill Hex File

Set `4096` bytes starting at `0x08010000` to `0xFF` (`0x08010000` ~ `0x08010FFF`).

```bash
# Output Hex
HexEdit.exe --set 0x08010000:4096:0xFF -f app.hex -o output.hex

# Output Bin
HexEdit.exe --set 0x08010000:4096:0xFF -f app.hex -o output.bin
```

---

## ❓ FAQ

**Q: What is the difference between Hex and Bin files?**

Bin is raw binary data with no address information — the file content is the firmware bytes themselves. Hex (Intel HEX) is a text format where each line contains address, data, and checksum, natively supporting non-contiguous address ranges. In MCU development, compilers typically generate Hex files, while flashing tools support both formats.

**Q: What happens if `-x` is not specified for a Bin file?**

Without `-x`, the Bin file data is directly appended to the output with no address offset. When merging multiple Bin files, always specify `-x` to avoid data overlapping.

**Q: Can addresses overlap during merge?**

Yes — later input files overwrite data at the same addresses from earlier files. If you need to preserve gaps between regions, use `-i` to specify a fill byte.

**Q: Why are there large blocks of `0xFF` in the merged Bin output?**

Bin files contain no address information. When merging, if there is a gap between the start addresses of two Bin files, HexEdit automatically fills it with `0xFF`. Use `-i` to customize the fill byte, or set `0xFFFFFFFF` to split the output into separate files.

**Q: What does `-i 0xFFFFFFFF` (split output) mean?**

When merging files with non-contiguous addresses, setting `-i 0xFFFFFFFF` splits the output into multiple independent Bin files instead of connecting them with fill bytes. Useful when there are large gaps between BootLoader and APP regions.

**Q: Why is the output Bin file larger than the source files?**

Bin output fills address gaps. For example, if BootLoader ends at `0x0800FFFF` and APP starts at `0x08010000`, the output is compact with no gap. If there is a gap, it is filled with `0xFF` (or the byte specified by `-i`). Set `-i 0xFFFFFFFF` to split output and avoid padding.

**Q: How does delete affect Bin files?**

Bin files are continuous binary data. Deleting from the middle splits the data into two disconnected segments. It is recommended to output Hex files for delete operations (Hex format natively supports non-contiguous addresses).

**Q: What is the scope of `-b` and `-h`?**

These are mode-switching parameters. Once specified, all subsequent **input and output** files use that type until `-b` or `-h` is encountered again. Without them, file type is inferred from the extension: `.bin` → Bin, others → Hex.

**Q: What is the maximum number of files for merge?**

Up to **64** input files.

---

## 📋 Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | — | Initial release: merge, delete, cut, fill, format conversion |
