# LidarRansac-C

[🇹🇷 Türkçe](#-türkçe) | [🇬🇧 English](#-english)

---

<a name="-türkçe"></a>
## 🇹🇷 Türkçe

**LidarRansac-C**, Lidar sensör verilerini işleyerek 2D ortamda **RANSAC (Random Sample Consensus)** algoritması ile doğru (line) tespiti yapan, saf C dilinde yazılmış performans odaklı bir projedir.

Bu proje, gürültülü sensör verilerinden anlamlı geometrik şekiller çıkarmak ve robotik haritalama/konumlandırma (SLAM) süreçlerine temel oluşturmak amacıyla geliştirilmiştir.

![Project Output Example](proje_ciktisi.png)
*(Not: Görsel, proje çalıştırıldıktan sonra `gnuplot` ile üretilmektedir)*

### 🚀 Özellikler

*   **TOML Veri İşleme:** Lidar tarama verilerini (`scan.toml`) okur ve ayrıştırır.
*   **Gürültü Filtreleme:** Ham veriyi işleyerek aykırı değerleri temizler.
*   **RANSAC Algoritması:** Nokta bulutu içerisindeki en uygun doğruları (lines) iteratif olarak tespit eder.
*   **Segmentasyon:** Tespit edilen doğruları ve bu doğrulara ait "inlier" (içeride kalan) noktaları ayırır.
*   **Görselleştirme Desteği:** `gnuplot` kullanarak sonuçları (`.dat` dosyaları üzerinden) görselleştirir ve PNG formatında çıktı verir.
*   **Mesafe ve Açı Analizi:** Tespit edilen doğrular arasındaki açıları ve robotun (0,0) en yakın nesneye olan uzaklığını hesaplar.

### 🛠️ Gereksinimler

Projeyi derlemek ve görselleştirmek için aşağıdaki araçlara ihtiyacınız vardır:

*   **GCC Compiler:** C kodunu derlemek için.
*   **Gnuplot:** Veri analizi sonuçlarını görselleştirmek için.

#### Linux (Ubuntu/Debian) Kurulumu:
```bash
sudo apt update
sudo apt install build-essential gnuplot
```

### ⚙️ Derleme ve Çalıştırma

Projeyi çalıştırmak için aşağıdaki adımları takip edin:

#### 1. Kodu Derleyin
Matematik kütüphanesini (`-lm`) bağlamayı unutmayın:

```bash
gcc main.c -o lidar-ransac -lm
```

#### 2. Programı Çalıştırın
Program `scan.toml` dosyasını okuyacak ve analiz sonuçlarını `.dat` dosyalarına yazacaktır:

```bash
./lidar-ransac
```

#### 3. Sonuçları Görselleştirin
Üretilen `.dat` dosyalarını kullanarak grafiği oluşturun:

```bash
gnuplot cizim.gp
```
Bu işlem sonucunda dizinde `proje_ciktisi.png` adında bir görsel oluşacaktır.

---

<a name="-english"></a>
## 🇬🇧 English

**LidarRansac-C** is a performance-oriented project written in pure C that processes Lidar sensor data to perform 2D **line detection** using the **RANSAC (Random Sample Consensus)** algorithm.

This project is designed to extract meaningful geometric shapes from noisy sensor data, serving as a foundation for robotic mapping and localization (SLAM) processes.

![Project Output Example](proje_ciktisi.png)
*(Note: The image is generated via `gnuplot` after running the project)*

### 🚀 Features

*   **TOML Data Parsing:** Reads and parses Lidar scan data from `scan.toml`.
*   **Noise Filtering:** Processes raw data to remove outliers.
*   **RANSAC Algorithm:** Iteratively detects the best-fitting lines within the point cloud.
*   **Segmentation:** Separates detected lines and their corresponding "inlier" points.
*   **Visualization Support:** Uses `gnuplot` to visualize results (via `.dat` files) and outputs them in PNG format.
*   **Distance & Angle Analysis:** Calculates angles between detected lines and the distance from the robot (0,0) to the nearest object.

### 🛠️ Requirements

To compile and visualize the project, you need the following tools:

*   **GCC Compiler:** To compile the C code.
*   **Gnuplot:** To visualize the data analysis results.

#### Linux (Ubuntu/Debian) Installation:
```bash
sudo apt update
sudo apt install build-essential gnuplot
```

### ⚙️ Compilation & Execution

Follow these steps to run the project:

#### 1. Compile the Code
Don't forget to link the math library (`-lm`):

```bash
gcc main.c -o lidar-ransac -lm
```

#### 2. Run the Program
The program will read `scan.toml` and write analysis results to `.dat` files:

```bash
./lidar-ransac
```

#### 3. Visualize Results
Generate the graph using the produced `.dat` files:

```bash
gnuplot cizim.gp
```
This process will create an image named `proje_ciktisi.png` in the directory.

## 📂 Project Structure / Proje Yapısı

```
LidarRansac-C/
├── main.c           # Source code (RANSAC implementation)
├── scan.toml        # Input: Lidar scan data
├── cizim.gp         # Gnuplot visualization script
├── .gitignore       # Git ignore file
└── README.md        # Documentation / Dokümantasyon
```

## 📝 License / Lisans

This project is open-source and available for educational/development purposes.