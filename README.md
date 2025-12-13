# HomeInventory 🏠

> A Qt-based home inventory management system with cloud synchronization

[![Qt](https://img.shields.io/badge/Qt-6.10+-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Firebase](https://img.shields.io/badge/Firebase-Realtime%20DB-orange.svg)](https://firebase.google.com/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## 📖 Overview

**HomeInventory** is a desktop application designed to help you catalog, organize, and track all the items in your home. Whether you're managing storage spaces, organizing belongings, or simply want to remember where you placed that important document, HomeInventory provides an intuitive interface to maintain a complete inventory of your household items.

Built with **Qt 6.10** and powered by **Firebase Realtime Database**, HomeMemory offers a modern, responsive user interface combined with reliable cloud storage, ensuring your inventory is always accessible and up-to-date.

### ✨ Key Features

- 🗂️ **Comprehensive Inventory Management**: Catalog items with detailed attributes (name, color, material, type, location, notes)
- 🏘️ **Hierarchical Location System**: Organize items by rooms, furniture, and specific storage positions
- 🔍 **Advanced Search**: Multi-criteria search with filters for colors, materials, and types
- ☁️ **Cloud Synchronization**: Automatic sync with Firebase Realtime Database
- 🎨 **Dual Theme Support**: Light and Dark themes for comfortable viewing
- 📊 **Visual Organization**: Interactive graphical representation of your home layout
- 🔒 **Data Integrity**: Built-in validation and duplicate detection
- 💾 **Export/Import**: Backup and restore your inventory data

## 🏗️ Architecture

HomeMemory follows modern software engineering principles with a clean, modular architecture:

```
┌─────────────────────────────────────────────────────┐
│                HomeInventoryGUI (EXE)               │
│  ┌──────────────────────────────────────────────┐   │
│  │  Widgets Layer (Qt Widgets)                  │   │
│  │  • Main Window  • Search  • Settings         │   │
│  │  • Object Manager  • Location Views          │   │
│  └──────────────────────────────────────────────┘   │
│                        ↓                            │
│  ┌──────────────────────────────────────────────┐   │
│  │  Application Configuration                   │   │
│  │  • App Settings  • Theme Management          │   │
│  └──────────────────────────────────────────────┘   │
└────────────────────┬────────────────────────────────┘
                     │ Repository Interface
                     ↓
┌─────────────────────────────────────────────────────┐
│             HomeInventoryData (DLL)                 │
│  ┌──────────────────────────────────────────────┐   │
│  │  Repository Layer                            │   │
│  │  • ObjectRepository  • ColorRepository       │   │
│  │  • MaterialRepository  • TypeRepository      │   │
│  └──────────────────────────────────────────────┘   │
│                        ↓                            │
│  ┌──────────────────────────────────────────────┐   │
│  │  Models (Domain Objects)                     │   │
│  │  • HomeObject  • Location                    │   │
│  └──────────────────────────────────────────────┘   │
│                        ↓                            │
│  ┌──────────────────────────────────────────────┐   │
│  │  Database Manager (Interface)                │   │
│  │  • IDatabaseManager (Abstract)               │   │
│  │  • FirebaseDatabaseManager (Implementation)  │   │
│  └──────────────────────────────────────────────┘   │
└────────────────────┬────────────────────────────────┘
                     │ REST API
                     ↓
              ┌────────────┐
              │  Firebase  │
              │  Realtime  │
              │  Database  │
              └────────────┘
```

### Design Patterns

- **Repository Pattern**: Abstracts data access logic from business logic
- **Dependency Injection**: Loose coupling between components
- **Factory Pattern**: Database manager creation
- **Singleton Pattern**: Application configuration management
- **MVC Pattern**: Separation of concerns in UI components

## 🚀 Getting Started

### Prerequisites

- **Qt Framework**: 6.10 or higher
- **C++ Compiler**: Supporting C++17 standard
  - Windows: MSVC 2022 or MinGW
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode 10+
- **CMake**: 3.16+ (optional, if using CMake)
- **Firebase Account**: For cloud database setup

### Installation

1. **Install Qt Visual Studio Tools**
    
    ```
    Visual Studio → Extensions → Manage Extensions
    Search for "Qt Visual Studio Tools" and install
    Restart Visual Studio
    ```
    
2. **Configure Qt in Visual Studio**
    
    ```
    Extensions → Qt VS Tools → Qt Versions
    Add your Qt installation path (e.g., C:\Qt\6.10.0\msvc2022_64)
    ```
    
3. **Clone the repository**
    
    ```bash
    git clone https://github.com/yourusername/HomeMemory.git
    cd HomeMemory
    ```
    
4. **Configure Firebase**
    
    - Create a Firebase project at [Firebase Console](https://console.firebase.google.com/)
    - Enable Realtime Database
    - Copy your Firebase URL and API key
    - Update `appsettings.ini` or configure in-app
5. **Open the solution in Visual Studio**
    
    - Open `HomeMemory.sln`
    - Visual Studio will automatically detect Qt projects
    - Set `HomeMemoryGUI` as the startup project
    - Select your build configuration (Debug/Release, x64)
6. **Build the solution**
    
    - Right-click on Solution → Build Solution
    - Or press `Ctrl+Shift+B`
    - The build will create both HomeMemoryData.dll and HomeMemory.exe
7. **Run the application**
    
    - Press `F5` to run with debugging
    - Or `Ctrl+F5` to run without debugging
    - The executable will be in `x64/Debug/` or `x64/Release/`

## 📁 Project Structure

```
HomeInventory/
├── README.md
├── LICENSE
├── VISUAL_STUDIO_SETUP.md
├── THIRD_PARTY_LICENSES.md
├── .gitignore
├── .editorconfig
├── HomeInventory.sln                   # Visual Studio Solution
│
├── HomeInventoryGUI/                   # GUI Application (EXE)
│   ├── HomeInventoryGUI.vcxproj        # VS Project file
│   ├── HomeInventoryryGUI.vcxproj.filters
│   ├── main.cpp
│   │
│   ├── MainWindow/
│   │   ├── mainwindow.h
│   │   └── mainwindow.cpp
│   │
│   ├── Widgets/
│   │   ├── Environments/                # Home environment widgets
│   │   │   ├── wambienti.h/cpp
│   │   │   ├── wcasa.h/cpp
│   │   │   ├── wcantamb.h/cpp
│   │   │   ├── wsottoambienti.h/cpp
│   │   │   └── wsovraposizioni.h/cpp
│   │   │
│   │   ├── Rooms/                       # Specific rooms widgets
│   │   │   ├── wopenspace.h/cpp/ui
│   │   │   ├── wanticamera.h/cpp/ui
│   │   │   ├── wbagno.h/cpp/ui
│   │   │   ├── wcamera.h/cpp/ui
│   │   │   ├── wcantina.h/cpp/ui
│   │   │   ├── wbox.h/cpp/ui
│   │   │   ├── wbalcone.h/cpp/ui
│   │   │   ├── wripostiglio.h/cpp/ui
│   │   │   ├── wsalotto.h/cpp/ui
│   │   │   ├── wcucina.h/cpp/ui
│   │   │   ├── wscarpiera.h/cpp/ui
│   │   │   ├── warmadiobagno.h/cpp/ui
│   │   │   ├── warmadiettolav.h/cpp/ui
│   │   │   ├── warmadiocassettiera.h/cpp/ui
│   │   │   ├── wscrivaniacomodino.h/cpp/ui
│   │   │   ├── wleftwall.h/cpp/ui
│   │   │   └── wfrontwall.h/cpp/ui
│   │   │
│   │   ├── Movables/                   # Specific movables widgets
│   │   │   ├── wopenspace.h/cpp/ui
│   │   │   ├── wanticamera.h/cpp/ui
│   │   │
│   │   ├── ObjectManager/           # Object management UI
│   │   │   ├── wobjmanager.h
│   │   │   ├── wobjmanager.cpp
│   │   │   └── wobjmanager.ui
│   │   │
│   │   ├── Search/                  # Search interface
│   │   │   ├── wsearch.h
│   │   │   ├── wsearch.cpp
│   │   │   └── wsearch.ui
│   │   │
│   │   ├── Settings/                # Application settings
│   │   │   ├── wsettings.h
│   │   │   ├── wsettings.cpp
│   │   │   └── wsettings.ui
│   │   │
│   │   └── Common/                  # Shared widgets
│   │       ├── wlowbtns.h/cpp
│   │       └── wlogo.h/cpp
│   │
│   ├── Config/
│   │   ├── appconfig.h
│   │   └── appconfig.cpp
│   │
│   └── Resources/
│       ├── resources.qrc            # Qt resource file
│       ├── img/                     # Image assets
│       │   ├── plus.png
│       │   ├── minus.png
│       │   ├── arrowIconForward.png
│       │   ├── arrowIconback.png
│       │   └── settingsBtn-icon.jpg
│       └── styles/                  # Qt stylesheets
│           └── qdarkstyle/
│               └── dark/
│                   ├── darkstyle.qss
│                   ├── darkstyle.qrc
│                   └── rc/          # Style resources
│
└── HomeInventoryData/               # Data Layer Library (DLL)
    ├── HomeInventoryData.vcxproj       # VS Project file
    ├── HomeInventoryData.vcxproj.filters
    ├── homememorydata_global.h      # DLL export macros
    │
    ├── DatabaseManager/
    │   ├── idatabasemanager.h       # Database interface (abstract)
    │   ├── firebasedatabasemanager.h
    │   ├── firebasedatabasemanager.cpp
    │   ├── databasefactory.h
    │   └── databasefactory.cpp
    │
    ├── Models/
    │   ├── homeobject.h
    │   ├── homeobject.cpp
    │   ├── location.h
    │   └── location.cpp
    │
    └── Repositories/
        ├── objectrepository.h
        ├── objectrepository.cpp
        ├── colorrepository.h
        ├── colorrepository.cpp
        ├── materialrepository.h
        ├── materialrepository.cpp
        ├── typerepository.h
        └── typerepository.cpp
```

## 🎯 Usage

### Adding a New Item

1. Navigate to the desired location (room/furniture/position)
2. Click the **Add** button in the Object Manager
3. Fill in the item details:
   - Object name (required)
   - Color, Material, Type (from dropdown lists)
   - Notes (optional)
4. Click **Save** to store the item

### Searching for Items

1. Open the **Search** window
2. Enter search criteria:
   - Object name (partial match supported)
   - Select one or more colors
   - Select one or more materials
   - Select one or more types
3. Click **Search** to view results
4. Click **Clear** to reset filters

### Managing Attributes

In the **Settings** window, you can:
- Add new colors, materials, or types
- Remove unused attributes
- Switch between Light and Dark themes

## 🛠️ Development

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/HomeMemory.git
cd HomeMemory

# Build Data Layer
cd HomeMemoryData
qmake
make

# Build GUI Application
cd ../HomeMemoryGUI
qmake
make

# Run
cd build/release
./HomeMemory
```

### Running Tests

```bash
cd tests
qmake tests.pro
make
./tests
```

### Code Style

This project follows the [Qt Coding Conventions](https://wiki.qt.io/Qt_Coding_Style):
- Indentation: 4 spaces
- Naming: camelCase for variables/functions, PascalCase for classes
- File naming: lowercase with underscores for class files (e.g., `home_object.h`)

## 🔧 Configuration

HomeMemory stores its configuration in:
- **Windows**: `%APPDATA%/YourCompany/HomeMemory.ini`
- **Linux**: `~/.config/YourCompany/HomeMemory.ini`
- **macOS**: `~/Library/Preferences/com.yourcompany.HomeMemory.plist`

### Configuration Options

```ini
[Database]
type=Firebase
firebaseUrl=https://your-project.firebaseio.com
apiKey=your-api-key-here

[UI]
theme=dark
language=en
```

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

Please ensure your code:
- Follows the project's coding style
- Includes appropriate tests
- Updates documentation as needed

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Authors

- **David-Mile** - **Arianna-Lionetti**

## 🙏 Acknowledgments

- Qt Framework for the excellent cross-platform toolkit
- Firebase for reliable cloud database services
- QDarkStyle for the beautiful dark theme
- All contributors who have helped shape this project

## 📞 Support

- 📧 Email: support@homememory.com
- 🐛 Issues: [GitHub Issues](https://github.com/David-Mile/HomeInventoryy/issues)
- 💬 Discussions: [GitHub Discussions](https://github.com/David-Mile/HomeInventory/discussions)

## 🗺️ Roadmap

- [ ] Mobile app (Android/iOS) synchronization
- [ ] Image upload for items
- [ ] Multi-user support with permissions
- [ ] Item value tracking and depreciation
- [ ] Reminder system for warranties/maintenance
- [ ] Integration with smart home devices

## 📊 Project Status

This project is actively maintained and under continuous development. Current version: **v1.0.0-beta**

---

Made with ❤️ using Qt and Firebase
