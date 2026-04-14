# 🔧 HexEdit

> 一款面向 MCU 开发者的 Hex/Bin 文件编辑命令行工具
> 合并 · 删除 · 裁剪 · 清零 · 格式转换

---

## ✨ 功能特性

| 功能 | 说明 |
|------|------|
| 🔄 **格式转换** | Bin ↔ Hex 互转，支持指定起始地址 |
| ➕ **文件合并** | 最多 64 个 Hex/Bin 文件合并为一个输出 |
| ✂️ **区域删除** | 删除指定地址范围的数据 |
| 🔍 **区域裁剪** | 提取指定地址范围的数据 |
| 🎨 **区域充填** | 对指定地址范围写入指定字节（清零 / 填充） |
| 📦 **智能填充** | Bin 文件合并时自动填充间隙，默认 `0xFF` |

---

## 🚀 快速开始

### 基本用法

```bash
HexEdit.exe [操作命令] -f <输入文件> -o <输出文件>
```

### 最简示例

```bash
# Bin 转 Hex（STM32 默认 Flash 起始地址 0x08000000）
HexEdit.exe -x 0x08000000 -f firmware.bin -o firmware.hex

# Hex 转 Bin
HexEdit.exe -f firmware.hex -o firmware.bin
```

---

## 📖 命令参考

### 操作命令

| 命令 | 说明 | 格式 |
|------|------|------|
| `--add` | **合并文件**（默认操作） | `--add -f a.bin -f b.bin -o out.hex` |
| `--del` | **删除**指定地址范围 | `--del <地址>:<长度>` |
| `--cut` | **裁剪**指定地址范围 | `--cut <地址>:<长度>` |
| `--set` | **充填**指定地址范围 | `--set <地址>:<长度>:<字节>` |
| `--version` | 输出版本信息 | |
| `--help` | 输出帮助信息 | |

### 参数说明

| 参数 | 说明 | 示例 |
|------|------|------|
| `-f` | 输入文件（最多 64 个） | `-f a.bin -f b.hex` |
| `-o` | 输出文件名 | `-o output.hex` |
| `-b` | 指定后续**输入和输出**文件为 **Bin** 格式 | `-b -f a.bin` |
| `-h` | 指定后续**输入和输出**文件为 **Hex** 格式 | `-h -f a.hex` |
| `-x` | 指定 Bin 文件的**起始地址**（必须在 `-f` 之前），不指定则直接拼接 | `-x 0x08000000` |
| `-i` | 设置 Bin 输出的**填充字节**（`0xFFFFFFFF` 分段输出） | `-i 0x25` |

### 地址格式

```
<十六进制地址>:<十进制长度>[:<十六进制字节>]
```

示例：`0x08000000:4096:0xFF` → 从地址 `0x08000000`（STM32 Flash 起始地址）开始，长度 `4096` 字节，填充值 `0xFF`

### 文件类型推断规则

⚠️ **重要**：`-b` / `-h` 参数一旦指定，后续所有**输入和输出**文件沿用该类型，直到再次遇到 `-b` / `-h` 切换。

- 不指定时：`.bin` 后缀 → Bin，其他 → Hex
- 指定 `-b`：后续文件全部视为 Bin
- 指定 `-h`：后续文件全部视为 Hex

---

## 💡 使用示例

### 📎 文件合并

#### 场景 1：Bin + Bin → Hex

将两个 Bin 文件按指定地址合并，输出 Hex 文件。典型场景：APP 固件 + BootLoader。

```
boot.bin → 起始地址 0x08000000（BootLoader）
app.bin  → 起始地址 0x08010000（应用程序）
```

```bash
HexEdit.exe --add -x 0x08000000 -f boot.bin -x 0x08010000 -f app.bin -o output.hex
```

#### 场景 2：Bin + Bin → Bin

```
boot.bin → 起始地址 0x08000000（BootLoader）
app.bin  → 起始地址 0x08010000（应用程序）
```

```bash
HexEdit.exe --add -x 0x08000000 -f boot.bin -x 0x08010000 -f app.bin -o output.bin
```

💡 Bin 文件无地址信息，`boot.bin` 不足 `0x10000`（64KB）字节时默认填充 `0xFF`。可通过 `-i` 修改填充字节，设置 `0xFFFFFFFF` 将分段输出。

#### 场景 3：Hex + Hex → Hex

将两个 Hex 文件合并，输出 Hex 文件。

```bash
HexEdit.exe --add -f boot.hex -f app.hex -o output.hex
```

#### 场景 4：Hex + Bin → Hex

将 Hex 文件与 Bin 文件合并，输出 Hex 文件。

```
boot.hex → Hex 文件（自带地址）
app.bin  → 起始地址 0x08010000（应用程序）
```

```bash
HexEdit.exe --add -h -f boot.hex -b -x 0x08010000 -f app.bin -o output.hex
```

---

### ✂️ 数据删除

#### 场景 1：从 Bin 文件删除

删除 `0x08010000` 地址开始的 `256` 字节。

```bash
HexEdit.exe --del 0x08010000:256 -x 0x08010000 -f app.bin -o output.hex
```

⚠️ 建议输出 Hex 文件。若删除中间部分，Bin 文件会被截断为两段。

#### 场景 2：从 Hex 文件删除

删除 `0x08010000` 地址开始的 `256` 字节（`0x08010000` ~ `0x080100FF`）。

```bash
HexEdit.exe --del 0x08010000:256 -f app.hex -o output.hex
```

---

### 🔍 数据裁剪

#### 场景 1：从 Bin 文件裁剪

提取 `0x08010000` 地址开始的 `4096` 字节（1 个 Flash Page）。

```bash
# 输出 Hex
HexEdit.exe --cut 0x08010000:4096 -x 0x08010000 -f app.bin -o output.hex

# 输出 Bin
HexEdit.exe --cut 0x08010000:4096 -x 0x08010000 -f app.bin -o output.bin
```

#### 场景 2：从 Hex 文件裁剪

提取 `0x08010000` 地址开始的 `4096` 字节（`0x08010000` ~ `0x08010FFF`）。

```bash
# 输出 Hex
HexEdit.exe --cut 0x08010000:4096 -f app.hex -o output.hex

# 输出 Bin
HexEdit.exe --cut 0x08010000:4096 -f app.hex -o output.bin
```

---

### 🎨 数据充填

#### 场景 1：Bin 文件清零

将 `0x08010000` 地址开始的 `4096` 字节全部设为 `0x00`。

```bash
# 输出 Hex
HexEdit.exe --set 0x08010000:4096:0x00 -x 0x08010000 -f app.bin -o output.hex

# 输出 Bin
HexEdit.exe --set 0x08010000:4096:0x00 -x 0x08010000 -f app.bin -o output.bin
```

#### 场景 2：Hex 文件填充

将 `0x08010000` 地址开始的 `4096` 字节全部设为 `0xFF`（`0x08010000` ~ `0x08010FFF`）。

```bash
# 输出 Hex
HexEdit.exe --set 0x08010000:4096:0xFF -f app.hex -o output.hex

# 输出 Bin
HexEdit.exe --set 0x08010000:4096:0xFF -f app.hex -o output.bin
```

---

## ❓ 常见问题

**Q：Hex 和 Bin 文件有什么区别？**

Bin 是纯二进制数据，不包含地址信息，文件内容就是固件字节本身。Hex（Intel HEX）是文本格式，每行包含地址、数据和校验，天然支持非连续地址段。MCU 开发中，编译器通常生成 Hex 文件，烧录工具两种格式都支持。

**Q：Bin 文件不指定 `-x` 会怎样？**

不指定 `-x` 时，Bin 文件数据直接拼接到当前输出末尾，不设地址偏移。合并多个 Bin 时建议始终指定 `-x`，否则数据会叠在一起。

**Q：合并时地址会重叠吗？**

不会。后输入的文件会覆盖先输入文件中相同地址的数据。如果需要保留中间空隙，使用 `-i` 指定填充字节。

**Q：为什么 Bin 合并后文件中间有大量 `0xFF`？**

Bin 文件本身不包含地址信息。合并时如果两个 Bin 文件的起始地址之间有空隙，HexEdit 会自动用 `0xFF` 填充。可通过 `-i` 参数自定义填充字节，设置 `0xFFFFFFFF` 则分段输出。

**Q：`-i 0xFFFFFFFF` 分段输出是什么意思？**

当合并的文件地址不连续时，设置 `-i 0xFFFFFFFF` 会将输出拆分为多个独立的 Bin 文件，而非用填充字节连接。适用于 BootLoader 和 APP 之间有大段空白的场景。

**Q：输出的 Bin 文件为什么比源文件大？**

Bin 输出会填充地址间隙。例如 BootLoader 结束于 `0x0800FFFF`，APP 起始于 `0x08010000`，中间没有间隙则文件紧凑；如果有间隙则会填充 `0xFF`（或 `-i` 指定的字节）。设置 `-i 0xFFFFFFFF` 可分段输出避免填充。

**Q：删除操作对 Bin 文件有什么影响？**

Bin 文件是连续的二进制数据，删除中间部分会导致数据断开。建议删除操作输出 Hex 文件（Hex 格式天然支持非连续地址）。

**Q：`-b` 和 `-h` 参数的作用范围？**

它们是模式切换参数，一旦指定，后续所有**输入和输出**文件都沿用该类型，直到再次遇到 `-b` 或 `-h` 切换。不指定时按文件后缀推断（`.bin` → Bin，其他 → Hex）。

**Q：最多支持多少个文件合并？**

最多支持 **64** 个输入文件。

---

## 📋 版本历史

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| 1.0.0 | — | 初始版本，支持合并、删除、裁剪、充填、格式转换 |
