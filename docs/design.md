# PNG LSB Steganography Tool — Design Document

## 1. Project Overview

This project implements a command-line steganography application in C that hides encrypted text messages inside PNG images using the Least Significant Bit (LSB) method.

The application uses XOR-based encryption with a user-provided key to protect the message before embedding it into the image. The same key is required to extract and decrypt the original message.

The program follows a modular design separating PNG processing, steganography, encryption, file operations, and command-line handling.

### Objectives

- Encode and decode messages within PNG images.
- Encrypt and decrypt messages using a user-supplied key.
- Support grayscale and RGB PNG images, including alpha-channel variants.
- Validate inputs and image capacity before encoding.
- Minimize visual changes by modifying only least significant bits.
- Provide a command-line interface with clear error messages.

## 2. System Architecture

| Module             | Responsibility                                           |
| ------------------ | -------------------------------------------------------- |
| `main.c`           | Parses command-line arguments and coordinates operations |
| `png_handler.c/.h` | Loads, validates, and saves PNG images using libpng      |
| `lsb.c/.h`         | Embeds and extracts data from pixel color channels       |
| `otp.c/.h`         | Performs XOR encryption and decryption                   |
| `file_utils.c/.h`  | Reads and writes plaintext and key files                 |

Each module exposes functions through a header file, allowing components to be developed and tested independently.

### Encoding workflow

```text
message.txt ──┐
              ├──> XOR encryption ──> LSB embedding ──> stego.png
key.txt ──────┘                           ▲
                                      cover.png
```

The ciphertext is preceded by a four-byte message-length header. Decoding reverses the process: load the stego PNG, extract ciphertext, XOR it with the same key, and write the recovered plaintext.

## 3. Implementation Design

### 3.1 PNG Handler

The PNG handler uses `libpng` for image input and output. Image information is stored in a custom `PNGImage` structure:

```c
typedef struct {
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    int channels;
    size_t rowbytes;
    png_bytep *rows;
    png_structp png_ptr;
    png_infop info_ptr;
} PNGImage;
```

- `png_load()` validates the PNG signature, reads the image, and prepares pixel data.
- `png_save()` writes the modified image.
- `png_capacity()` calculates usable LSB storage.
- `stego_png_free()` releases allocated image resources.

### 3.2 LSB Steganography

The LSB module embeds one bit of ciphertext into the least significant bit of each usable color channel.

```text
Original channel: 11001000 (200)
Embedded bit:    1
Modified channel: 11001001 (201)
```

Only the last bit changes, so each channel value changes by at most one.

A four-byte big-endian header stores the ciphertext length, followed by the ciphertext itself. Alpha channels are skipped to preserve transparency.

The usable payload capacity is:

```text
capacity_bytes = floor(width × height × usable_channels / 8) - 4
```

Here, `usable_channels` is 1 for grayscale or grayscale-alpha and 3 for RGB or RGBA. The four bytes are reserved for the length header.

### 3.3 XOR Encryption

The encryption module uses XOR for both encryption and decryption:

```text
ciphertext[i] = plaintext[i] XOR key[i]
plaintext[i]  = ciphertext[i] XOR key[i]
```

The user supplies a text key file whose byte length must be at least the message length. The key is not embedded in the image.

**Security limitation:** A manually written text key is not a cryptographically secure one-time pad unless its bytes are independently random, as long as the message, and never reused. The implementation is accurately described as XOR encryption with a user-supplied key.

### 3.4 File Utilities

| Function        | Purpose                                          |
| --------------- | ------------------------------------------------ |
| `file_read()`   | Reads a file into a dynamically allocated buffer |
| `file_write()`  | Writes binary data to a file                     |
| `file_exists()` | Checks whether a file exists                     |

Binary file modes preserve arbitrary bytes, including `0x00`. Callers free buffers allocated by `file_read()`.

### 3.5 Command-Line Interface

`main.c` validates arguments and coordinates the modules.

**Encode:**

```bash
./stego encode cover.png message.txt key.txt stego.png
```

**Decode:**

```bash
./stego decode stego.png key.txt recovered.txt
```

The implementation also provides an optional `keygen` command for generating random key files.

## 4. Development Stages

| Stage | Implementation                | Validation approach                                          |
| ----- | ----------------------------- | ------------------------------------------------------------ |
| 1     | PNG loading and saving        | Load an image, display properties, save a copy               |
| 2     | LSB embedding and extraction  | Hide and recover a test string                               |
| 3     | XOR encryption and decryption | Encrypt and decrypt a test message                           |
| 4     | File utilities                | Read and copy a file; compare contents                       |
| 5     | CLI integration               | Run the complete encode/decode workflow                      |
| 6     | Final verification            | Compare original and recovered messages; test invalid inputs |

This staged approach isolates defects before combining components.

## 5. Testing and Verification

The following cases describe the intended verification. **Record actual outcomes after running the final application; these are not claims that all tests have passed.**

| Test case                | Expected result                        | Actual result |
| ------------------------ | -------------------------------------- | ------------- |
| Valid PNG and message    | Encoding succeeds                      | Not recorded  |
| Decode with correct key  | Original message recovered             | Not recorded  |
| Message exceeds capacity | Error displayed                        | Not recorded  |
| Key shorter than message | Error displayed                        | Not recorded  |
| Invalid PNG              | File rejected                          | Not recorded  |
| Missing input file       | Error displayed                        | Not recorded  |
| Empty message file       | Error displayed                        | Not recorded  |
| Existing output file     | Overwriting prevented in normal use    | Not recorded  |
| Wrong decryption key     | Incorrect plaintext; no key validation | Not recorded  |
| Grayscale and RGB images | Both supported                         | Not recorded  |

### 5.1 Message Integrity

After encoding and decoding, compare the original and recovered files:

```bash
cmp -s message.txt recovered.txt && echo "PASS" || echo "FAIL"
```

`PASS` indicates byte-for-byte equality.

### 5.2 Image Quality

The program modifies only the least significant bit of usable color channels, limiting each modified channel's difference to one. Compare original and stego images visually; pixel-level differences may also be measured.

### 5.3 Input Validation

The program checks the PNG signature, file readability, message size, key length, and image capacity. Errors should result in a clear message and nonzero exit status.

## 6. Limitations

- The current PNG writer does not preserve all metadata; this is an outstanding assignment requirement.
- Some PNG formats are converted to 8-bit channels during loading.
- A human-written text key does not provide true one-time-pad security.
- Incorrect decryption keys are not automatically detected.
- File-existence checks do not guarantee atomic protection against overwriting.

## 7. Conclusion

The project demonstrates a modular C implementation of PNG steganography with XOR encryption. Separating image handling, LSB processing, encryption, and file operations improves testability and maintainability.

The principal end-to-end verification is an encode/decode round trip where the recovered message matches the original byte-for-byte. Before submission, execute and record the tests above and address PNG metadata preservation to meet the full assignment specification.
