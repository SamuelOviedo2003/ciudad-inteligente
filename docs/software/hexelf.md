---
source: https://isa262.davinsony.com/software/hexelf/
title: Extración de archivos HEX/BIN & ELF
---

# Extración de archivos HEX/BIN & ELF

Para extraer los códigos necesarios para **Wokwi** realizaremos los siguientes pasos:
1. Activar la opción de mostrar la carpeta de compilación en preferencias: [Imagen: Preferencias en el Arduino IDE] (imagen local: ../../assets/images/hexelf-preferences.webp) Clic en el cuadrado de la opción `compile`. [Imagen: Opción de mostrar la compilación] (imagen local: ../../assets/images/hexelf-compile-option.webp)
2. Compilar el código en Arduino IDE con el botón señalado en la imagen: [Imagen: Usando el botón para generar los archivos] (imagen local: ../../assets/images/hexelf-verify-button.webp)
3. Una vez compilado, en la carpeta de compilación encontraremos los archivos `code.hex/code.bin` y `code.elf`. Para encontrar la carpeta de compilación, en la ventana output, buscaremos el `PATH` a los archivos. [Imagen: Carpeta de salida] (imagen local: ../../assets/images/hexelf-output-folder.webp)
4. En el explorador de archivos, pegaremos el `PATH` y encontraremos los archivos `code.hex/code.bin` y `code.elf`. [Imagen: Explorador] (imagen local: ../../assets/images/hexelf-explorer.webp)
5. Copiar los archivos `code.hex` y `code.elf` a la carpeta de simulación.
