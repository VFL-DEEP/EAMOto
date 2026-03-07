# EAMOto - Çalışma Akışı ve Mimari

Bu dosya EAMOto (Epic Asset Manager Otomatik Giriş) aracının, "fast mode" implementasyonunda izleyeceği adım adım çalışma şemasını tanımlar.

## Temel Çalışma Akışı

**1. Başlangıç ve Doğrulama Kontrolü (Token Verification)**
* EAMOto çalıştırılır çalıştırılmaz, ilk olarak sistemde zaten geçerli bir Epic oturumu olup olmadığı sorgulanır.
* Hedef dosya: `~/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile` okunur.
* Bu dosyada `token-expiration` veya `refresh-token-expiration` tarihlerine bakılır.
* **Eğer token geçerliyse**: Hiçbir API isteği yapmadan doğrudan **Adım 6**'ya atlanır.
* **Eğer token yoksa veya süresi dolmuşsa**: **Adım 2**'den işleme devam edilir.

**2. Firefox'tan Çerez (Cookie) Okuma**
* Epic hesap bilgilerine manuel giriş yapmadan erişmek için, kullanıcının daha önce Firefox/Zen Browser üzerinden açtığı Epic Games oturumu kullanılır.
* Profil dizini (örn. `~/.mozilla/firefox/s850yvut.default-release/` veya `profiles.ini` den tespit edilen yolla) bulunur.
* Profil altındaki `cookies.sqlite` SQLite veritabanına bağlanılarak `.epicgames.com` için geçerli cookie verileri (isim=değer formatında) toplanır.

**3. Authorization Code Alınması**
* Toplanan cookie'ler (özellikle `EPIC_SSO` gibi oturum çerezleri) libcurl'e enjekte edilir.
* Auth URL'sine HTTP GET isteği gönderilir:  
  `https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code`
* Dönen JSON yanıtından `authorizationCode` (tek kullanımlık yetki kodu) çıkartılır. 
* _(Eğer hata dönerse kullanıcıya tarayıcı üzerinden Epic'e giriş yapması gerektiği uyarısı verilir ve işlem sonlandırılır)._

**4. Token Takas (Exchange) İşlemi**
* Alınan `authorizationCode`, Epic API'leri kullanılarak kalıcı token'lara dönüştürülmelidir.
* **Hedef POST URL**: `https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token`
* **Header Bilgileri**: 
  * `Content-Type: application/x-www-form-urlencoded`
  * `Authorization: Basic MzRhMDJjZjhmNDQxNGUyOWIxNTkyMTg3NmRhMzZmOWE6ZGFhZmJjY2M3Mzc3NDUwMzlkZmZlNTNkOTRmYzc2Y2Y=` *(base64(clientId:clientSecret) karşılığı)*
* **Gövde (Body)**: `grant_type=authorization_code&code=[ALINAN_AUTH_CODE]`
* Dönen JSON yanıtından `access_token`, `refresh_token`, `expires_at` ve `refresh_expires_at` kaydedilir.

**5. Token'ı Keyfile'a Yazma**
* API'den elde edilen güncel token'lar, EAM'ın anlayacağı formatta Flatpak config dizinindeki keyfile'a yazılır.
* **Bölüm:** `[io/github/achetagames/epic_asset_manager]`
* İlgili GLib keyfile değerleri:
  * `token='[ACCESS_TOKEN]'`
  * `refresh-token='[REFRESH_TOKEN]'`
  * `token-expiration='[EXPIRES_AT]'`
  * `refresh-token-expiration='[REFRESH_EXPIRES_AT]'`
* _Not: Değerler eklenirken var olan EAM ayarlarının ve dosya formatının (tek tırnak içine alma) bozulmamasına özen gösterilir._

**6. İşlem Sonu ve Uygulamanın Başlatılması**
* Tüm doğrulama ve güncelleme işlemleri başarıyla tamamlandıktan sonra, işletim sistemi üzerinde Flatpak ile Epic Asset Manager başlatılır.
* Sistem çağrısıyla çalıştırılan komut: `flatpak run io.github.achetagames.epic_asset_manager`
* Token geçerli olduğu için, uygulama doğrudan login ekranını atlayarak ana arayüzünde açılır. EAMOto başarıyla görevini tamamlarken kapanır.

## Sistem Bileşenleri
- `main.cpp`: Yürütmeyi başlatan, yönlendiren ve hata yönetimi yapan ana iskelet.
- `cookie.cpp`/`cookie.h`: SQLite sorgularını işleyerek gerekli cookie okumalarını yapar.
- `auth.cpp`/`auth.h`: Tüm CURL network işlemlerini ve `nlohmann_json` üzerinden json parse/response işlemlerini kapsar.
- `config.cpp`/`config.h`: Keyfile okuma, ilgili blok altına standartlara uygun insert/update yapma işlemlerini tutar.
