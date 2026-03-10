#!/bin/bash

set -e

echo "📦 EAMOto kurulumu başlıyor..."

echo "1) Arch Linux bağımlılıkları kontrol ediliyor ve yükleniyor..."
sudo pacman -S --needed base-devel cmake curl sqlite nlohmann-json xclip

echo "2) Proje derleniyor..."

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"


rm -rf build
sudo rm -rf ~/.local/bin/eamoto

cmake -B build -S .
cmake --build build

echo "3) Çalıştırılabilir dosya kullanıcı bin dizinine (~/.local/bin) kopyalanıyor..."
mkdir -p ~/.local/bin
cp build/eamoto ~/.local/bin/eamoto
chmod +x ~/.local/bin/eamoto

echo "4) Masaüstü Kısayolu (Desktop Entry) güncelleniyor..."

mkdir -p ~/.local/share/applications

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