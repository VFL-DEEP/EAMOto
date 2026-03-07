# Önemli Dosya Yolları ve Açıklamaları

## Kullanıcıya Ait Dosyalar

### 1. Epic Asset Manager Token Storage
```
~/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile
```
**Açıklama**: EAM'ın token, refresh-token ve expiration bilgilerini sakladığı GLib keyfile formatındaki config dosyası. Bu dosyaya token yazılacak.

**Örnek İçerik**:
```
[io/github/achetagames/epic_asset_manager]
token='...'
refresh-token='...'
token-expiration='2026-03-09T07:37:48.134Z'
refresh-token-expiration='2027-03-07T22:22:50.628Z'
```

### 2. Firefox Profile
```
~/.mozilla/firefox/s850yvut.default-release/
```
**Açıklama**: Kullanıcının Firefox profile dizini. Cookie'ler buradaki `cookies.sqlite` dosyasında saklanır. Bu profile kullanılarak Epic'e giriş yapılmışsa cookie'ler otomatik kullanılır.

**Önemli Dosyalar**:
- `cookies.sqlite` - Epic sitesi cookie'leri
- `cert9.db` - SSL sertifikaları

### 3. Epic Asset Manager Database
```
~/.var/app/io.github.achetagames.epic_asset_manager/data/epic_asset_manager/eam.db
```
**Açıklama**: EAM'ın SQLite veritabanı. Kullanıcı bilgileri burada saklanır.

---

## Sistem Dosyaları

### 4. Firefox profiles.ini
```
~/.mozilla/firefox/profiles.ini
```
**Açıklama**: Firefox profile yapılandırması. Hangi profile'in default olduğunu gösterir.

### 5. Flatpak App Konumu
```
/var/lib/flatpak/
/usr/bin/flatpak
```
**Açıklama**: Flatpak kurulum dizini ve çalıştırılabilir.

---

## Proje Dosyaları (Oluşturulacak)

### 6. Proje Kök Dizini
```
/home/aligalip/EAMOto/
```

### 7. Kaynak Kod
```
/home/aligalip/EAMOto/src/
/home/aligalip/EAMOto/include/
```

### 8. Build Çıktısı
```
/home/aligalip/EAMOto/build/
```

---

## API Endpoint'leri

### 9. Epic Auth URL
```
https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code
```
**Açıklama**: Authorization code almak için kullanılan URL.

### 10. Token Endpoint
```
https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token
```
**Açıklama**: Authorization code'u token'a çeviren API endpoint'i.

**Client Credentials (launcherAppClient2)**:
- **clientId**: `34a02cf8f4414e29b15921876da36f9a`
- **clientSecret**: `daafbccc737745039dffe53d94fc76cf`

**Request Formatı**:
```
POST /account/api/oauth/token
Content-Type: application/x-www-form-urlencoded
Authorization: Basic base64("34a02cf8f4414e29b15921876da36f9a:daafbccc737745039dffe53d94fc76cf")

grant_type=authorization_code&code=<AUTH_CODE>
```

---

## Notlar

- Dosya yolları kullanıcıya göre değişebilir (home dizini, profile adı vb.)
- Flatpak uygulamaları ~/.var/app/ altında user-specific veri saklar
- Firefox cookie'leri SQLite formatındadır, sqlite3 kütüphanesi ile okunabilir
