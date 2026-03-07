#!/bin/bash

# EAMOto Arch Linux Kurulum Betiği

set -e # Hata olursa betiği durdur

echo "📦 EAMOto kurulumu başlıyor..."

echo "1) Arch Linux bağımlılıkları kontrol ediliyor ve yükleniyor..."
sudo pacman -S --needed base-devel cmake curl sqlite nlohmann-json

echo "2) Proje derleniyor..."
# Proje dizininde olduğumuzdan emin olalım (bu sh dosyasının olduğu dizin)
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# Eski build klasörü varsa silelim ki temiz kurulum olsun (opsiyonel)
# rm -rf build

cmake -B build -S .
cmake --build build

echo "3) Çalıştırılabilir dosya kullanıcı bin dizinine (~/.local/bin) kopyalanıyor..."
mkdir -p ~/.local/bin
cp build/eamoto ~/.local/bin/eamoto
chmod +x ~/.local/bin/eamoto

echo "4) Masaüstü Kısayolu (Desktop Entry) güncelleniyor..."
# Kendi menü simgemizi oluşturarak direkt Epic Asset Manager yazmasını sağlıyoruz. 
# Böylece normal uygulamayı açıyormuş gibi EAMOto'yu açarsın.
mkdir -p ~/.local/share/applications

# Masaüstü kısayol dosyasını yazıyoruz. (Burada $HOME değişkeni script çalışırken senin ev dizinine çevrilir)
cat << EOF > ~/.local/share/applications/eamoto.desktop
[Desktop Entry]
Name=Epic Asset Manager (Auto-Login)
Comment=EAMOto aracılığıyla otomatik giriş yapan Epic Asset Manager başlatıcısı
Exec=$HOME/.local/bin/eamoto
Icon=io.github.achetagames.epic_asset_manager
Terminal=false
Type=Application
Categories=Development;Game;Utility;
Keywords=epic;unreal;asset;manager;
EOF

# Uygulama simgelerinin yenilenmesi için
update-desktop-database ~/.local/share/applications/ || true

echo "✅ Kurulum Başarıyla Tamamlandı! 🚀"
echo "───────────────────────────────────────────────────────"
echo "  Artık sistem menünden 'Epic Asset Manager (Auto-Login)' "
echo "  uygulamasını aratıp tıklayarak programı doğrudan"
echo "  token'ı yenilenmiş şekilde çalıştırabilirsin!"
echo " "
echo "  Terminalden manuel çalıştırmak istersen: ~/.local/bin/eamoto"
echo "───────────────────────────────────────────────────────"
