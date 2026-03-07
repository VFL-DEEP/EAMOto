# EAMOto - Epic Asset Manager Otomatik Giriş

## Nedir?
Epic Asset Manager'ı (EAM) her açtığında sana authorization code soran bir yazılım. Bu proje o code'u otomatik alır ve EAM'a giriş yapar.

## Nasıl Çalışır?
1. **Token Kontrolü**: EAM'ın config dosyasında kayıtlı token var mı kontrol eder
2. **Firefox Cookie**: Zen/Firefox'ta önceden Epic'e giriş yaptıysan cookie'leri kullanır
3. **Giriş Yoksa**: Firefox headless modda açar, senin yerine Epic'e giriş yapar
4. **Token Alışverişi**: Authorization code'u token'a çevirir ve EAM'a yazar
5. **EAM Başlatır**: Artık giriş yapılmış şekilde EAM'ı açar

## Ne Gerekli?
- Linux
- Zen Browser veya Firefox
- Epic Asset Manager (Flatpak)
- CMake
- libcurl
- nlohmann/json
- sqlite3

## Kurulum
(WIP - Geliştirme aşamasında)

## Kullanım
(WIP - Geliştirme aşamasında)
