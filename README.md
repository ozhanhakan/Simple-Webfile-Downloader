# Simple Web Downloader (C + libcurl)

Bu proje, verilen bir web sayfasını indirir, HTML içindeki bağlantıları (örn. PDF) tarar ve bulunan dosyaları otomatik olarak indirmeye çalışır. C diliyle yazılmış, küçük ve modüler bir araçtır.

## Özellikler

- HTML içinde `href` linklerini tarar.
- Linklerdeki gereksiz boşlukları (özellikle sonda kalan whitespace) temizler.
- HTTP redirect (301/302) takip eder.
- İsteğe bağlı uzantı filtresi: `pdf`, `doc` vb.
- Göreceli linkleri absolute URL'e çevirir (örn. `/assets/file.pdf`).

## Gereksinimler

Derlemek için sistemde `libcurl` geliştirme paketleri kurulu olmalı.

### Ubuntu / Debian / 42 ortamı

```bash
sudo apt-get update
sudo apt-get install -y libcurl4-openssl-dev
```

### macOS

```bash
brew install curl
```

## Kurulum ve derleme

Bu klasörde (veya projenin kökünde) bir `Makefile` varsa derleme genelde şu şekilde yapılır:

```bash
make
```

## Kullanım

Temel kullanım:

```bash
./mydownloader <url> [uzanti]
```

### 1) Sayfayı tarayıp sadece PDF indirmek

```bash
./mydownloader https://www.ornek-site.com/raporlar pdf
```

### 2) Filtresiz kullanım

Uzantı vermezsen, araç HTML/PHP olmayan “indirilebilir” linkleri bulmaya çalışır.

```bash
./mydownloader https://www.ornek-site.com/dosyalar
```

### 3) Tek dosya indirmek

Doğrudan bir dosya URL'i verirsen sadece onu indirir.

```bash
./mydownloader https://www.ornek-site.com/sunum.pdf
```

## Nasıl çalışır? (kısa)

1. `libcurl` ile hedef URL'den içerik çekilir.
2. Gelen veri dinamik bir buffer'a yazılır (`malloc` / `realloc`).
3. HTML buffer içinde temel string fonksiyonlarıyla link taraması yapılır (`strstr`, `strchr`).
4. Bulunan linkler normalize edilir (whitespace temizliği, relative -> absolute).
5. Geçerli görünen linkler indirilip diske yazılır.

## Notlar

- HTML parsing burada bilerek “hafif” tutuluyor; her site için tam uyumluluk garanti edilmez.
- Bazı siteler bot koruması / JS ile link üretimi kullanıyorsa linkler bulunmayabilir.

## Lisans

MIT.