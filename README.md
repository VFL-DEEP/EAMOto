<div align="center">
  <img src="EAMOto.png" alt="EAMOto Logo" width="250"/>

# EAMOto - Epic Asset Manager Auto-Auth

</div>

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B17-blue)

**EAMOto** is a lightweight semi-auto login tool that permanently solves the annoying authorization code prompt for the Flatpak version of **Epic Asset Manager** on Linux.

Tired of opening your browser, copying an auth code, and pasting it every time? EAMOto handles the entire flow automatically — it extracts your active Epic Games session from your browser, fetches a fresh authorization code, launches EAM, and types the code in for you.

---

## How It Works ⚙️

1. **🍪 Cookie Extraction:** Our system Reads your active Epic Games session cookies directly from your Zen Browser or Firefox profile (`cookies.sqlite` + WAL files).
2. **🔑 Auth Code Fetch:** Uses `libcurl` with modern browser headers to request a fresh `authorizationCode` from the Epic Games API — no manual copy-pasting.
3. **🚀 Semi-Auto Login:** Launches Epic Asset Manager and copies the authorization code to your clipboard. Simply paste it (`Ctrl+V`) into the EAM login screen and hit **Authenticate**. No more opening browsers or copying codes manually!

> **Note:** EAMOto works by automating the existing Epic login screen — it does not modify any system files or store credentials.

---

## System Requirements 📋

- **OS:** Linux
- **Browser:** Zen Browser or Firefox (must be logged into Epic Games at least once)
- **Epic Asset Manager:** Flatpak version (`io.github.achetagames.epic_asset_manager`)
- **Dependencies:** `cmake`, `curl`, `sqlite3`, `nlohmann-json`, `xclip`, `xdotool`

---

## 🛠️ Quick Install — Arch Linux

```bash
git clone https://github.com/AliGalipALATLI/EAMOto.git
cd EAMOto
chmod +x install.sh
./install.sh
```

The script will:
- Install all required packages via `pacman`
- Compile the project with CMake
- Copy the binary to `~/.local/bin/eamoto`
- Create a **Desktop Entry** so you can launch it directly from your app menu as **"Epic Asset Manager (Auto-Login)"**

---

## 📦 Manual Install — Debian / Ubuntu / Fedora

**1. Install dependencies:**

- **Debian/Ubuntu:**
  ```bash
  sudo apt install build-essential cmake libcurl4-openssl-dev libsqlite3-dev nlohmann-json3-dev xclip xdotool
  ```
- **Fedora:**
  ```bash
  sudo dnf install gcc-c++ cmake libcurl-devel sqlite-devel json-devel xclip xdotool
  ```

**2. Build:**

```bash
git clone https://github.com/AliGalipALATLI/EAMOto.git
cd EAMOto
cmake -B build -S .
cmake --build build
```

**3. Run:**

```bash
./build/eamoto
```

---

## 🏗️ Project Architecture

- **`main.cpp`:** Orchestrates the full flow — cookie extraction → auth code fetch → EAM launch → clipboard copy.
- **`cookie.cpp`:** Locates your Zen/Firefox profile via `profiles.ini`, copies the SQLite cookie database (including WAL files for live sessions), and extracts Epic Games session cookies.
- **`auth.cpp`:** CURL-based HTTP client that sends cookies to the Epic API and retrieves the authorization code.
- **`config.cpp`:** Keyfile read/write helpers (reserved for future use).
- **`utils.cpp`:** Common utilities (`getHomeDir()`).

---

## ⚠️ Known Limitations

- Requires **XWayland** — works on Wayland desktops as long as XWayland is enabled (most distros have it by default)
- The browser does not need to be closed, but must have an active Epic Games session
- Auth codes are single-use and expire in ~5 minutes — EAMOto fetches one fresh per launch

---

## License 📄

MIT License — see [LICENSE](LICENSE) for details.

---

*The era of copy-pasting codes is over. Happy game developing! 🎮🐧*
