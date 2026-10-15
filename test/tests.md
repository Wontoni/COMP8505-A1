# Testing Documentation

## Table of Contents

1. [Test 1: All-White Image](#test-1-all-white-image)
2. [Test 2: All-Black Image](#test-2-all-black-image)
3. [Test 3: Simple Image](#test-3-simple-image)
4. [Test 4: Complex Image](#test-4-complex-image)
5. [Test 5: Insufficient Image Capacity](#test-5-insufficient-image-capacity)
6. [Test 6: Invalid PNG File](#test-6-invalid-png-file)
7. [Test 7: Insufficient Key Length](#test-7-insufficient-key-length)

---

## Test 1: All-White Image

**Description:** Encode and decode a small text file containing a few sentences using an all-white PNG image. Verify that the image appearance is preserved and the decoded message matches the original.

### Image Comparison

**Original Carrier Image:**
![Original White](screenshots/white_original.png)

**Stego Image:**
![Stego White](screenshots/white_stego.png)

**Visual Comparison:** [Pass/Fail] — The stego image visually resembles the original.

### File Verification

Run:

```bash
diff message.txt recovered.txt
```

**Screenshot:**

![White Image Diff](screenshots/white_diff.png)

**Result:** [Pass/Fail]

---

## Test 2: All-Black Image

**Description:** Encode and decode the same small text file using an all-black PNG image.

### Image Comparison

**Original Carrier Image:**
![Original White](screenshots/white_original.png)

**Stego Image:**
![Stego White](screenshots/white_stego.png)

### File Verification

```bash
diff message.txt recovered.txt
```

**Screenshot:**

![Black Image Diff](screenshots/black_diff.png)

**Result:** [Pass/Fail]

---

## Test 3: Simple Image

**Description:** Encode and decode the text file using a PNG containing solid colours or geometric patterns.

### Image Comparison

**Original Carrier Image:**
![Original White](screenshots/white_original.png)

**Stego Image:**
![Stego White](screenshots/white_stego.png)

**Visual Comparison:** [Pass/Fail]

### File Verification

```bash
diff message.txt recovered.txt
```

**Screenshot:**

![Simple Image Diff](screenshots/simple_diff.png)

**Result:** [Pass/Fail]

---

## Test 4: Complex Image

**Description:** Encode and decode the text file using a detailed photograph saved as PNG.

### Image Comparison

**Original Carrier Image:**
![Original White](screenshots/white_original.png)

**Stego Image:**
![Stego White](screenshots/white_stego.png)

**Visual Comparison:** [Pass/Fail]

### File Verification

```bash
diff message.txt recovered.txt
```

**Screenshot:**

![Complex Image Diff](screenshots/complex_diff.png)

**Result:** [Pass/Fail]

---

## Test 5: Insufficient Image Capacity

**Description:** Attempt to embed a message larger than the PNG image's available capacity. The program should reject the operation.

**Screenshot:**

![Insufficient Capacity](screenshots/capacity_error.png)

**Result:** [Pass/Fail]

---

## Test 6: Invalid PNG File

**Description:** Attempt to encode a message using a non-PNG or corrupted image file. The program should display an error.

**Screenshot:**

![Invalid PNG](screenshots/invalid_png.png)

**Result:** [Pass/Fail]

---

## Test 7: Insufficient Key Length

**Description:** Attempt to encode a message using a key shorter than the plaintext. The program should display an error.

**Screenshot:**

![Short Key](screenshots/short_key.png)

**Result:** [Pass/Fail]
