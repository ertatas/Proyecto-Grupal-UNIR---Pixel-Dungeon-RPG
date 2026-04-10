# Proyecto Grupal UNIR - Pixel Dungeon RPG

## ⚠️ REQUISITOS PREVIOS (IMPORTANTE)
Antes de empezar, asegúrate de tener instalado y configurado:

- Visual Studio (con desarrollo en C++)
- Git instalado
- Cuenta de GitHub
- Git configurado con tu cuenta (user.name y user.email)

---

## ⚠️ IMPORTANTE
- Colocar el proyecto en:
  `C:\Proyecto-Grupal-UNIR---Pixel-Dungeon-RPG`

## 📁 ESTRUCTURA DEL PROYECTO
La carpeta raíz debe contener:
- `DungeonJuego`
- `UNIR-2D`
- `.gitignore`
- `README.md`

Ruta de SFML:
`C:\Proyecto-Grupal-UNIR---Pixel-Dungeon-RPG\UNIR-2D\SFML-2.5.1`

---

## 🧩 GUÍA DE INSTALACIÓN

1. Abrir `UNIR-2D.sln` dentro de la carpeta `UNIR-2D`
2. Click derecho en **DungeonJuego**
3. Seleccionar **"Establecer como proyecto de inicio"**
4. En **Propiedades**, arriba seleccionar:
   - **Configuración:** `Todas las configuraciones`
   - **Plataforma:** `x64`

---

## ⚙️ CONFIGURACIÓN DE SFML

### INCLUDE
Ruta:
`DungeonJuego > Propiedades > C/C++ > General > Directorios de inclusión adicionales`

Añadir:
```text
C:\Proyecto-Grupal-UNIR---Pixel-Dungeon-RPG\UNIR-2D\SFML-2.5.1\include
```

### LIB
Ruta:
`DungeonJuego > Propiedades > Vinculador > General > Directorios de bibliotecas adicionales`

Añadir:
```text
C:\Proyecto-Grupal-UNIR---Pixel-Dungeon-RPG\UNIR-2D\SFML-2.5.1\lib
```

---

## 📚 LIBRERÍAS NECESARIAS

Ruta:
`DungeonJuego > Propiedades > Vinculador > Entrada > Dependencias adicionales`

Añadir:
```text
sfml-graphics-s-d.lib;sfml-window-s-d.lib;sfml-system-s-d.lib;sfml-audio-s-d.lib;opengl32.lib;winmm.lib;gdi32.lib;freetype.lib;openal32.lib;flac.lib;vorbis.lib;vorbisenc.lib;vorbisfile.lib;ogg.lib;ws2_32.lib;user32.lib;advapi32.lib
```

---

## 🧠 PREPROCESADOR

Ruta:
`DungeonJuego > Propiedades > C/C++ > Preprocesador > Definiciones del preprocesador`

Añadir:
```text
SFML_STATIC
```

---

## ▶️ COMPILACIÓN

1. **Compilar > Limpiar solución**
2. **Compilar > Recompilar solución**

---

## 🚫 GIT - ARCHIVOS IGNORADOS (.gitignore)

Ya incluido, pero por si acaso:

```gitignore
.vs/
x64/
Debug/
Release/
*.obj
*.exe
*.pdb
*.ilk
*.log
*.tlog
*.user
*.VC.db
SFML-2.5.1/bin/
```

---

## 🚀 SUBIR A GITHUB

### Primera vez
```bash
git init
git add .
git commit -m "Proyecto funcionando"
git remote add origin URL
git branch -M main
git push -u origin main
```

### Actualizaciones
```bash
git add .
git commit -m "Cambios"
git push
```

---

## 💡 RECOMENDACIONES

- Todos usar la misma ruta del proyecto
- No subir archivos compilados
- Usar commits claros
- No modificar configuración sin avisar
