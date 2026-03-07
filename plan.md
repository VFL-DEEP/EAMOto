# Epic Auth Automation - Plan

## Hedef
Linux'ta Epic Asset Manager'ı her açtığında authorization code istemeden otomatik olarak giriş yapılmasını sağlamak.

## Mevcut Durum
- Kullanıcı: Linux (Zen Browser/Firefox)
- Epic Asset Manager: Flatpak versiyonu
- Sorun: Her açılışta manuel authorization code girilmesi gerekiyor

## Çözüm

### 1. Token Storage Konumu
```
~/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile
```

### 2. Firefox Profile Konumu
```
~/.mozilla/firefox/s850yvut.default-release/
```

### 3. İş Akışı
1. Token kontrolü (keyfile'dan oku)
2. Token geçerli → EAM başlat
3. Token yok/geçersiz → Firefox headless aç → Auth → Token exchange → keyfile'a yaz → EAM başlat

### 4. Teknoloji Stack
- Dil: C++
- Build: CMake
- HTTP: libcurl
- JSON: nlohmann/json
- Cookie: sqlite3

### 5. Proje Yapısı
```
EAMOto/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── browser.cpp
│   ├── auth.cpp
│   ├── config.cpp
│   └── cookie.cpp
└── include/
```
