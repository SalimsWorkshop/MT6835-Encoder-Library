# 🧭 MT6835 Encoder Library

This library allows you to read the **21-bit absolute angle** via **SPI** and the **16-bit incremental angle** via **QEI**.  
It was developed for the **Teensy 4.0**, but should work with other MCUs (e.g. Arduino) with minor adjustments.

---

## ⚙️ Features

- Read **absolute angle (21-bit)** via SPI  
- Read **incremental angle (16-bit)** via QEI  
- Simple and lightweight interface  
- Easy to integrate into your project  

---

## 🚀 Usage

1. Include the library header:  
   ```cpp
   #include "MT6835.h"
   ```

2. Create the sensor object:  
   ```cpp
   MT6835 mt6835(CS_PIN, SPI_FREQUENCY, ENC_A, ENC_B);
   ```
   - `CS_PIN` – Chip Select pin  
   - `SPI_FREQUENCY` – SPI clock frequency  
   - `ENC_A`, `ENC_B` – encoder channel pins  

3. Initialize in `setup()`:
   ```cpp
   void setup() {
       mt6835.initialize();
   }
   ```

4. Read angles:
   ```cpp
   float absAngle = mt6835.readAbsAng();
   float incAngle = mt6835.readIncAng();
   ```

📄 See **`Usage_example.cpp`** for a complete example.

---

## 🎥 Video Explanation

Here’s a video of me writing and explaining the sensor library:  
[![Watch the video](https://img.youtube.com/vi/dmIMhL8mpH8/hqdefault.jpg)](https://www.youtube.com/watch?v=dmIMhL8mpH8)

---

## ❤️ Support

If you found my code helpful, I’d really appreciate it if you subscribed to my YouTube channel:

<p align="center">
  <a href="https://www.youtube.com/@SalimsWorkshop" target="_blank">
    <img src="https://img.shields.io/badge/Subscribe-YouTube-red?logo=youtube&style=for-the-badge" alt="Subscribe on YouTube"/>
  </a>
</p>

---

### 💡 Contributions

Suggestions and improvements are always welcome!  
Feel free to open an issue or submit a pull request if you have ideas for making this library better.

### 📡 Author

**Salim’s Workshop**  
[YouTube Channel →](https://www.youtube.com/@SalimsWorkshop)

---

### 🧩 License

This project is open source and available under the **MIT License**.
