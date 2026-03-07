# EAMOto Görev Listesi (Fast Mode)

## 1. Proje Altyapısının Kurulması
- [ ] `src` ve `include` dizinlerinin oluşturulması.
- [ ] `CMakeLists.txt` dosyasının yazılması (C++17, \`libcurl\`, \`sqlite3\`, \`nlohmann_json\` entegrasyonu).
- [ ] Temel `main.cpp` taslağının oluşturulması, modüler yapının tasarlanması.

## 2. Cookie Seçimi ve SQLite Modülü (`src/cookie.cpp`)
- [ ] `~/.mozilla/firefox/profiles.ini` dosyasını okuyup aktif profil dizinini (`s850yvut.default-release`) bulma.
- [ ] Profil klasöründeki `cookies.sqlite` veritabanına bağlanma (`sqlite3`).
- [ ] `.epicgames.com` domainine ait aktif cookie'leri çekme işlemi.
- [ ] Çekilen cookie'leri HTTP istekleri için uygun string formatına dönüştürme.

## 3. HTTP ve Kimlik Doğrulama (Auth) Modülü (`src/auth.cpp`)
- [ ] **Auth Code Alımı**: libcurl kullanarak Epic Auth endpoint'ine (`https://www.epicgames.com/id/api/redirect?clientId=34a02cf8f4414e29b15921876da36f9a&responseType=code`) cookie'lerle GET isteği atma. 
- [ ] Dönen JSON yanıtını parse ederek `authorizationCode` değerini alma. (Eğer başarısızsa kullanıcıya Firefox üzerinden Epic'e giriş yapması gerektiğini gösteren hata mesajı basma).
- [ ] **Token Takası (Exchange)**: Elde edilen `authorizationCode`'u kullanarak libcurl ile `/account/api/oauth/token` endpoint'ine (POST) Basic Auth (Base64) ve `application/x-www-form-urlencoded` header'ları ile istek atma.
- [ ] Yanıt olarak gelen JSON'dan token detaylarını (`access_token`, `refresh_token`, expiration tarihleri vb.) derleme.

## 4. Konfigürasyon ve Keyfile Modülü (`src/config.cpp`)
- [ ] EAM keyfile dizin yolunu statik veya dinamik olarak ayarlama (`~/.var/app/io.github.achetagames.epic_asset_manager/config/glib-2.0/settings/keyfile`).
- [ ] Dosyayı okuma ve token tarihlerini kontrol ederek halihazırda geçerli bir token olup olmadığını anlama (GLib keyfile formatında).
- [ ] Alınan yeni token'ları uygun formatta (`token='...'` vb.) keyfile içine tırnaklı ve geçerli değerlerle yazma veya var olan değerleri güncelleme işlemi.

## 5. Main İş Akışı ve Uygulama Entegrasyonu (`src/main.cpp`)
- [ ] Adım 1: Uygulama başladığında ilk iş olarak keyfile kontrolü (geçerli token varsa doğrudan EAM'ı başlat ve çık).
- [ ] Adım 2: Geçerli token yoksa, Firefox Cookie'lerini okuyarak başla.
- [ ] Adım 3: Auth Code alımından sonra başarılı ise Token Exchange mantığını çağır.
- [ ] Adım 4: Yeni token'ları dosyaya kalıcı olarak işle.
- [ ] Adım 5: EAM flatpak uygulamasını (`flatpak run io.github.achetagames.epic_asset_manager`) başlat.

## 6. Derleme ve Test Süreci 
- [ ] `cmake -B build -S .` ve `cmake --build build` süreçlerini başarıyla tamamlama.
- [ ] Önceki token'ları bilerek expired duruma getirip (veya silip) EAMOto uygulamasının doğru çalışıp EAM programına otomatik login olabildiğini baştan sona EAM üzerinde test etme.
