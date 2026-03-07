# Model İhtiyaçları ve Bilgiler

## Giriş

Bu dosya, EAMOto projesini geliştirecek modelin ihtiyaç duyduğu bilgileri ve istekleri içerir. Model bu dosyayı referans alarak projeyi geliştirmelidir. Bu dosyadaki bilgiler yetersiz kaldığında model kullanıcıya sormalıdır.

## Proje Özeti

- **Amaç**: Epic Asset Manager'ı her açtığında authorization code istemeden otomatik giriş yapmak
- **Platform**: Linux (Zen Browser/Firefox)
- **EAM**: Flatpak versiyonu
- **Yöntem**: Firefox profile cookie'lerini kullan, token'ı keyfile'a yaz

---

## Modelin Kendisine Sorması Gereken Sorular

Model geliştirme yaparken aşağıdaki soruları kendisine sormalı ve cevaplayamadığı konularda kullanıcıya sormalıdır:

1. **Auth Ekranı Anlamak İçin**: ✅ ÇÖZÜLDÜ
   - ~~EAM'ın authorization code giriş ekranı nasıl görünüyor?~~
   - ~~Code'u hangi textbox'a/giriş alanına yazıyorum?~~
   - ~~Bu alanın widget adı veya UI elementi nedir?~~

   **CEVAP (Ekran Görüntüsünden Analiz Edildi - 2026-03-08)**:
   EAM açıldığında "Epic Games Login" başlıklı bir ekran gösterir. Ekranın yapısı:
   - **"Open In Browser" butonu**: Tıklandığında Epic Games login sayfasını varsayılan tarayıcıda açar
   - **authorizationCode text input**: Placeholder'ı "authorizationCode" olan bir GTK text entry alanı. Kullanıcı auth code'u buraya yapıştırır
   - **"Authenticate" butonu**: Code girildikten sonra bu butona tıklanarak doğrulama yapılır
   - **"Or copy the authentication link"**: Alternatif olarak auth linkini kopyalama seçeneği sunar
   - **Alt bilgi çubuğu**: Hata durumunda "Relogin request failed" mesajı gösterilir

   **Otomasyon Stratejisi**: Programımız bu ekranı bypass edecek. "Open In Browser" butonuna tıklamak yerine,
   doğrudan Firefox cookie'lerini kullanarak auth code'u API üzerinden alacak ve token exchange yaparak
   keyfile'a yazacak. Böylece EAM açıldığında zaten geçerli bir token olacak ve bu ekran gösterilmeyecek.

   **Auth Code Response Sayfası (Ekran Görüntüsünden Analiz Edildi - 2026-03-08)**:
   "Open In Browser" butonuna tıklandığında veya doğrudan auth URL'si açıldığında, Epic Games
   aşağıdaki JSON response'u döndürür:

   ```json
   {
     "warning": "Do not share this code with any 3rd party service. It allows full access to your Epic account.",
     "redirectUrl": "https://localhost/launcher/authorized?code=<AUTH_CODE>",
     "authorizationCode": "<AUTH_CODE>",
     "exchangeCode": null,
     "sid": null
   }
   ```

   **Auth URL**: `https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code`

   **Önemli Çıkarımlar**:
   - `authorizationCode` alanı → EAM'ın textbox'una yapıştırılan kod budur
   - `redirectUrl` → Code, URL query parametresi olarak da mevcut
   - `exchangeCode` ve `sid` → Her ikisi de null, kullanılmıyor
   - `clientId=34a02cf8f4414e29b15921876da36f9a` → EAM'ın kullandığı clientId (files.md'de de mevcut)

   **Otomasyon İçin**: Program, Firefox cookie'lerini (`cookies.sqlite`) kullanarak bu URL'ye
   libcurl ile GET request yapacak → JSON parse edecek → `authorizationCode` değerini alacak →
   Token exchange endpoint'ine POST edecek → Aldığı token'ları keyfile'a yazacak.

2. **Token Exchange İçin**: ✅ ÇÖZÜLDÜ
   - ~~libcurl POST isteği tam olarak nasıl yapılır?~~
   - ~~clientId ve clientSecret gerekli mi?~~
   - ~~Request body formatı ne olmalı?~~
   - ~~Response'dan hangi alanlar çekilmeli?~~

   **CEVAP (Web Araştırması + KOD Analizi - 2026-03-08)**:
   - **Endpoint**: `https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token`
     (EAM/egs-api-rs'nin kullandığı ana endpoint)
   - **Method**: POST
   - **Content-Type**: `application/x-www-form-urlencoded`
   - **Auth Header**: `Authorization: Basic base64(clientId:clientSecret)`
     - clientId: `34a02cf8f4414e29b15921876da36f9a`
     - clientSecret: `daafbccc737745039dffe53d94fc76cf`
     - Base64: `base64("34a02cf8f4414e29b15921876da36f9a:daafbccc737745039dffe53d94fc76cf")`
     - Bu client "launcherAppClient2" olarak bilinir (Epic Games Launcher client)
   - **Body**: `grant_type=authorization_code&code=<AUTH_CODE>`
   - **Response JSON**:
     ```json
     {
       "access_token": "<TOKEN>",
       "refresh_token": "<REFRESH_TOKEN>",
       "expires_at": "2026-03-09T07:37:48.134Z",
       "refresh_expires_at": "2027-03-07T22:22:50.628Z",
       "token_type": "bearer",
       "account_id": "...",
       "client_id": "34a02cf8f4414e29b15921876da36f9a"
     }
     ```
   - Auth code tek kullanımlıktır ve ~5 dakika içinde expire olur

3. **Keyfile Yazma İçin**: ✅ ÇÖZÜLDÜ
   - ~~Token exact nasıl yazılmalı?~~
   - ~~Tırnak işaretleri, format, encoding ne?~~
   - ~~Dosya izinleri ne olmalı?~~

   **CEVAP (Dosya Okunarak Doğrulandı - 2026-03-08)**:
   Keyfile yolu: `~/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile`
   GLib keyfile formatında, section `[io/github/achetagames/epic_asset_manager]`:

   ```
   token='<ACCESS_TOKEN_VALUE>'
   refresh-token='<REFRESH_TOKEN_VALUE>'
   token-expiration='2026-03-09T07:37:48.134Z'
   refresh-token-expiration='2027-03-07T22:22:50.628Z'
   ```

   - Değerler tek tırnak (`'...'`) içinde yazılır
   - Tarih formatı ISO 8601 (UTC): `YYYY-MM-DDTHH:MM:SS.mmmZ`
   - Dosya ayrıca cache-directory, window-width gibi EAM ayarlarını da içerir
   - Dosya izinleri: standart kullanıcı dosyası (644)

4. **Hata Durumları İçin**:
   - Firefox açılmazsa ne yapılmalı?
   - Auth başarısız olursa nasıl handle edilmeli?
   - Token exchange hatasında ne yapılmalı?

5. **Flatpak İzni İçin**:
   - Config dosyasına erişim izni gerekli mi?
   - Flatpak run komutuna hangi --filesystem argümanları eklenmeli?

---

## Modelin İhtiyaç Duyacağı Bilgiler (eksik olabilir, sorulmalı!)

### Kritik - Öncelikli Sorulacak

1. ✅ **EAM Auth Ekran Görüntüsü** - ÇÖZÜLDÜ (2026-03-08)
2. ✅ **Token Exchange API Detayları** - ÇÖZÜLDÜ. clientId, clientSecret, endpoint, body formatı hepsi bulundu

### Önemli - Gerekirse Sorulacak

3. ✅ **Firefox Profile Yolu** - ÇÖZÜLDÜ. `s850yvut.default-release` profili doğrulandı
4. ✅ **Keyfile Formatı** - ÇÖZÜLDÜ. GLib keyfile formatı, tek tırnak, ISO 8601 tarih
5. **Flatpak Config Erişimi** - Henüz araştırılmadı (gerekirse bakılacak)

### Opsiyonel - Araştırılabilir

6. ✅ **MFA Durumu** - ÇÖZÜLDÜ. 2FA aktif ama cookie'ler sayesinde tekrar sorulmuyor. Cookie ile auth yapıldığında 2FA bypass ediliyor
7. ✅ **Client ID** - ÇÖZÜLDÜ. `34a02cf8f4414e29b15921876da36f9a` (launcherAppClient2)

---

## Başarı Kriterleri (Model Bunları Sağlamalı)

- [ ] Token yoksa Firefox headless açılabilmeli
- [ ] Auth code alındıktan sonra token exchange edilebilmeli
- [ ] Token keyfile'a doğru formatta yazılabilmeli
- [ ] EAM başlatılabilmeli
- [ ] Hata durumları ele alınabilmeli
- [ ] Flatpak ile çalışabilmeli

---

## Notlar

- Model, yukarıdaki soruları kendine sormalı ve cevaplayamadığı konularda kullanıcıya sormalıdır
- Bu dosyadaki bilgiler yetersiz kaldığında model kullanıcıdan ek bilgi istemelidir
- Başarısız olunan noktalarda geliştirme yapılmamalı, önce bilgi alınmalıdır
