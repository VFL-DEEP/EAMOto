#!/bin/bash

set -e

echo "📦 EAMOto kurulumu başlıyor..."

echo "1) Bağımlılıklar yükleniyor..."
sudo pacman -S --needed base-devel cmake curl sqlite nlohmann-json xclip xdotool

echo "2) Proje derleniyor..."
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

rm -rf build
cmake -B build -S .
cmake --build build

echo "3) Çalıştırılabilir dosya ~/.local/bin dizinine kopyalanıyor..."
mkdir -p ~/.local/bin
cp build/eamoto ~/.local/bin/eamoto
chmod +x ~/.local/bin/eamoto

echo "4) Masaüstü kısayolu oluşturuluyor..."
mkdir -p ~/.local/share/applications

cat << DESKTOP > ~/.local/share/applications/eamoto.desktop
[Desktop Entry]
Name=Epic Asset Manager (Auto-Login)
Comment=EAMOto aracılığıyla otomatik giriş yapan Epic Asset Manager başlatıcısı
Exec=$HOME/.local/bin/eamoto
Icon=io.github.achetagames.epic_asset_manager
Terminal=false
Type=Application
Categories=Development;Game;Utility;
Keywords=epic;unreal;asset;manager;
DESKTOP

update-desktop-database ~/.local/share/applications/ || true

echo ""
echo "✅ Kurulum tamamlandı! 🚀"
echo "───────────────────────────────────────────────────────"
echo "  Sistem menüsünden 'Epic Asset Manager (Auto-Login)'"
echo "  aratıp tıklayarak kullanabilirsin."
echo ""
echo "  Terminal: eamoto"
echo "───────────────────────────────────────────────────────"
