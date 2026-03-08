# EAMOto - Epic Asset Manager Auto-Auth (Fast Mode)

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B17-blue)

**EAMOto** is a lightweight and fast auto-login tool designed to permanently solve the annoying authorization code prompt issue for the Flatpak version of **Epic Asset Manager** on Linux.

Are you tired of constantly opening your browser to copy and paste authorization codes? EAMOto runs in the background, grabs your active Epic Games session cookies directly from your Firefox or Zen Browser profile, fetches a new token, and seamlessly injects it into the Epic Asset Manager's configuration file.

---

## How It Works ⚙️

EAMOto's "Fast Mode" follows three main steps:

1. **🍪 Cookie Extraction:** It pulls your SSO cookies (`cookies.sqlite`) for `epicgames.com` directly from your Firefox profile.
2. **🔑 Token Exchange:** Using these cookies, it sends a request to the Epic Games APIs (via `libcurl`), quietly retrieves an `authorizationCode`, and exchanges it for a permanent **Access Token**.
3. **🚀 Injection and Launch:** It writes the newly fetched token data into the Epic Asset Manager's Flatpak configuration file (`~/.var/app/.../keyfile`) while strictly preserving the GLib format standards, and then launches EAM. The entire process takes less than a second!

---

## System Requirements 📋

- **Operating System:** Linux
- **Browser:** Firefox or Zen Browser (you must have logged into Epic Games at least once)
- **Epic Asset Manager:** Flatpak version
- **Development Environment:** C++17 compatible compiler, CMake, Curl, SQLite3, Nlohmann-JSON library

---

## 🛠️ Quick Install for Arch Linux

There is an installation script provided in the main directory for Arch-based distributions (EndeavourOS, Manjaro, etc.) that installs dependencies via `pacman` and compiles the project.

Run the following command in the terminal from the project's root directory:

```bash
git clone https://github.com/aligalipalatli/EAMOto.git
cd EAMOto
chmod +x install.sh
./install.sh
```

**What does this script do?**

- Installs the required packages (`base-devel cmake curl sqlite nlohmann-json`).
- Compiles the project using CMake.
- Copies the resulting executable (`eamoto`) to your `~/.local/bin` directory.
- Creates a **Desktop Entry** (.desktop) file and adds a new icon named **"Epic Asset Manager (Auto-Login)"** to your Linux application launcher (Rofi, Whisker, Gnome, etc.).
- 🎉 You can now search for EAM in your system menu and launch it directly via EAMOto!

---

## 📦 Manual Installation (Debian / Ubuntu / Fedora)

For other Linux distributions, you can follow these steps to compile it manually:

1. **Install required libraries:**
   - **Debian/Ubuntu:** `sudo apt install build-essential cmake libcurl4-openssl-dev libsqlite3-dev nlohmann-json3-dev`
   - **Fedora:** `sudo dnf install gcc-c++ cmake libcurl-devel sqlite-devel json-devel`

2. **Compile the project:**

   ```bash
   git clone https://github.com/aligalipalatli/EAMOto.git
   cd EAMOto
   cmake -B build -S .
   cmake --build build
   ```

3. **Run:**
   ```bash
   ./build/eamoto
   ```

---

## 🏗️ Project Architecture

- **`main.cpp`:** Controls the execution chain, checks if valid permissions exist, and starts the process.
- **`cookie.cpp`:** Uses SQLite to search and locate Firefox profiles and extract `epicgames.com` session data.
- **`auth.cpp`:** The CURL-based HTTP client. It sends SSO cookies, receives the Authorization Code, and returns the final token from the `/api/oauth/token` route using Base64 Headers (includes JSON Parsing).
- **`config.cpp`:** The module that reads and safely manipulates the configuration (`keyfile`) structure inside EAM's Flatpak sandbox without breaking it.

---

## License 📄

This project is licensed under the **MIT License**. Check the [LICENSE](LICENSE) file in the repository for details.

---

_The era of copy-pasting code every time is over. Happy game developing! 🎮_
