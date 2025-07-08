# Simulador del Sistema Solar 3D – OpenGL

Este proyecto representa una simulación en 3D del sistema solar, desarrollado en **C++** con **OpenGL**, usando **FreeGLUT** y **stb_image.h**, compilado en **Code::Blocks**.

---

## Características

- Movimiento orbital y rotación realista de los planetas.
- Visualización de órbitas circulares.
- Control de cámara con teclado:
  - `x` → Pausa o reanuda rotación planetaria.
  - `1` a `9` → Enfoca automáticamente al planeta correspondiente.
  - `f` o `F` → Restablece la vista original.
- Inclusión de lunas (como Fobos y Deimos en Marte, 95 lunas de Júpiter).
- Texturas realistas cargadas con `stb_image.h`.
- Iluminación con múltiples fuentes (GL_LIGHT1, GL_LIGHT2).

---

## Organización del Proyecto
Guardar la carpeta **sistema_solar** en la ubicación:
C:/Users/TuUsuario/Documents/CodeBlocks/Projects/ (o donde tengas ubicado tus proyectos)

La carpeta de **texturas** debe ir en la raíz del disco local `C:/`, así:
C:/texturas/
├── tierra.jpg
├── marte.jpg
├── jupiter.jpg
├── luna.jpg
├── ...

> ⚠️ Esto es importante porque las rutas del código cargan las texturas desde `C:/texturas/nombre.jpg`. Si la colocas en otro lugar, deberás modificar las rutas dentro del código fuente.

---

## 🛠️ Requisitos

### Librerías necesarias:

- **FreeGLUT**  
  Usada para crear la ventana, manejar eventos y funciones OpenGL.

- **stb_image.h**  
  Librería de cabecera para cargar imágenes JPG/PNG fácilmente en OpenGL.

### 🔹 Instalación

1. Instala [Code::Blocks](http://www.codeblocks.org/downloads/26) con soporte para C++.
2. Descarga e instala FreeGLUT:  
   [FreeGLUT para Windows](https://www.transmissionzero.co.uk/software/freeglut-devel/)
3. Configura en tu proyecto:
   - `Settings > Compiler > Search directories > Compiler` → añade el path donde están los headers de FreeGLUT.
   - `Settings > Linker settings` → enlaza las `.lib` correspondientes.
4. Añade `stb_image.h` a tu carpeta del proyecto (está incluido en este repositorio).

---

## Ejecución

1. Abre el archivo `.cbp` del proyecto en Code::Blocks (`sistema_solar.cbp`).
2. Compila el proyecto en (`main.cpp`).
3. Asegúrate de tener la carpeta `C:/texturas/` con todas las imágenes necesarias.
4. ¡Disfruta de tu sistema solar en 3D!

---

## Autor

Desarrollado por **Leonel Fortunato Lizarbe Almeyda**  
📧 leonel.lizarbe@unmsm.edu.pe  
🔗 [GitHub](https://github.com/LeoNato04)

---

