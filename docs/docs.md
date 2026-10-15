# PNG LSB Steganography Tool — Design Document

## 1. Project Overview

This project uses the Least Significant Bit (LSB) method to hide an encrypted message inside of PNG images.


Following a modular design to separate PNG processing, steganogrpahy, encryption, and file operations, the application 
uses XOR based encryption with a One Time Pad (OTP) user provided key to encrypt and decrypt the message.


### Objectives

- Encode and decode messages within PNG images.
- Encrypt and decrypt messages using a user-supplied key.
- Support grayscale and RGB PNG images, including alpha-channel variants.
- Validate inputs and image capacity before encoding.
- Minimize visual changes by modifying only least significant bits.
- Provide a command-line interface with clear error messages.

## 2. System Architecture

Modules
- `main.c`           - Parses command-line arguments and runs operations
- `png_handler.c/.h` - Loads, validates, and saves PNG images using libpng
- `lsb.c/.h`         - Embeds and extras data from pixel color channels
- `otp.c/.h`         - Performs XOR encryption and decryption with OTP
- `file_utils.c/.h`  - Reads and writes plaintext and key files 

Each module uses a header file, allowing components to be developed and tested independently.

### Encoding workflow

The ciphertext is preceded by a four-byte message-length header. Decoding reverses the process: load the stego PNG, extract ciphertext, XOR it with the same key, and write the recovered plaintext.

The script will take the message, key and PNG in the following order:
KEY + MESSAGE + PNG -> main.c -> file_utils.c -> png_handler.c -> otp.c -> lsb.c -> png_handler.c -> OUTPUT PNG

XOR will be handled in the `otp.c` script using something along the lines of:
```c
output[0] = input[0] ^ key[0];
```

The LSB will replaced with something similar to:
```c
channel = (channel & 0xFE) | current_bit;
```

This operation will clear the least significant bit of the current byte, preserving the other 7.
```c
channel & 0xFE
```

This part of the operation will insert the current bit of the current part of the message (0 or 1).
```c
current_bit;
```


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
Functions
- `png_load()`       - validates the PNG signature, reads the image, and prepares pixel data.
- `png_save()`       - writes the modified image.
- `png_capacity()`   - calculates usable LSB storage.
- `free_image_png()` - releases allocated image resources.

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

The user gives a text key file whose byte length must be at least the message length.

### 3.4 File Utilities

Functions

- `file_read()`   - Reads a file into a dynamically allocated buffer
- `file_write()`  - Writes binary data to a file
- `file_exists()` - Checks whether a file exists

Binary file modes preserve arbitrary bytes, including `0x00`. Callers free buffers allocated by `file_read()`.

### 3.5 Command-Line Interface

`main.c` validates arguments and runs the modules

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

##### 4.1 PNG Loading and Saving

Load an image, display properties, and save a copy

##### 4.2 LSB Embedding and Extraction

Hide and recover a test string 

##### 4.3 XOR Encryption and Decryption

Encrypt and decrypt a test message  

##### 4.4 File Utilities

Read and copy a file; compare contents 

##### 4.5 CLI Integration

Run the complete encode/decode workflow    

##### 4.6 Final Verification

Compare original and recovered messages; test invalid inputs


## 5. Testing and Verification

All testing will be done and shown in the Testing.md file.


## 6. Conclusion

The project demonstrates a modular C implementation of PNG steganography with XOR encryption. Separating image handling, LSB processing, encryption, and file operations improves testability and maintainability.

