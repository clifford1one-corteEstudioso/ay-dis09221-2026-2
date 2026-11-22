# **Guía de instalación: Memoria y Cuerpo**

Paso a paso para instalar y abrir el programa en Windows y en Mac. No se necesita saber programar: solo copiar y pegar los comandos que aparecen en recuadro.

## **Antes de empezar**

Necesitas:

* Un computador con **cámara web** (o un celular usado como cámara).  
* **Conexión a internet** solo durante la instalación. Después el programa funciona sin internet.  
* La carpeta **Memoria\_y\_Cuerpo** completa, con estos archivos dentro:  
  * pose\_app.py, pose\_common.py, pose\_enroll.py, pose\_editor.py  
  * requirements.txt  
  * pose\_landmarker\_full.task (el modelo de detección)  
  * pose\_descriptors.npz, poses.json, poses\_manual.json  
  * la carpeta **poses** con todas las fotos (1a.png, 2a.png, etc.)

Importante: la carpeta poses debe estar junto a pose\_app.py. Si falta, el programa no puede mostrar las fotos.

**Versión de Python recomendada: 3.11 o 3.12.** Las versiones más nuevas (3.13 o superiores) pueden no ser compatibles con MediaPipe, la librería que detecta el cuerpo.

## **WINDOWS**

### **Paso 1\. Instalar Python**

1. Entra a la página de descargas: [https\://www\.python.org/downloads/windows/](https://www.python.org/downloads/windows/)  
2. Busca **Python 3.11.9** (o la última 3.12) y descarga el instalador **"Windows installer (64-bit)"**. Enlace directo a la página de la versión 3.11.9: [https\://www\.python.org/downloads/release/python-3119/](https://www.python.org/downloads/release/python-3119/)  
3. Abre el archivo descargado.  
4. **MUY IMPORTANTE:** en la primera pantalla del instalador, marca la casilla **"Add python.exe to PATH"** (abajo). Si no la marcas, los comandos no funcionarán.  
5. Pulsa **"Install Now"** y espera a que termine. Pulsa **Close**.

### **Paso 2\. Comprobar que Python quedó instalado**

1. Pulsa la tecla **Windows**, escribe PowerShell y ábrelo.  
2. Escribe esto y pulsa Enter:

python \--version  
Debe responder algo como Python 3.11.9. Si en cambio se abre la Microsoft Store o dice que no reconoce el comando, vuelve al Paso 1 y revisa que marcaste "Add python.exe to PATH". También puedes probar con py \--version.

### **Paso 3\. Abrir PowerShell dentro de la carpeta del proyecto**

1. Abre la carpeta **Memoria\_y\_Cuerpo** en el Explorador de archivos.  
2. Haz clic en la **barra de dirección** (arriba, donde aparece la ruta), borra el texto, escribe powershell y pulsa Enter.  
3. Se abre una ventana azul de PowerShell ya ubicada en esa carpeta.

### **Paso 4\. Crear el entorno e instalar las librerías**

Escribe estos comandos, uno por uno, pulsando Enter después de cada uno:

python \-m venv venv

.\\venv\\Scripts\\Activate.ps1  
Si aparece un error rojo que habla de "ejecución de scripts deshabilitada", escribe esto, responde S (o Y) y vuelve a ejecutar el comando anterior:

Set-ExecutionPolicy \-Scope CurrentUser RemoteSigned  
Cuando funcione, verás (venv) al comienzo de la línea. Entonces instala las librerías:

pip install \-r requirements.txt  
Esto descarga MediaPipe, OpenCV, NumPy y Pillow. Puede tardar varios minutos. Espera a que termine y aparezca de nuevo la línea con (venv).

### **Paso 5\. Abrir el programa**

python pose\_app.py \--fullscreen  
Se abre la ventana en pantalla completa. Si quieres probarlo en ventana normal, escribe solo python pose\_app.py.

Teclas: **espacio** salta de etapa, **s** muestra u oculta el esqueleto, **q** o **Esc** cierra el programa.

### **Para abrirlo otro día**

Cada vez que quieras volver a abrirlo: abre PowerShell dentro de la carpeta (Paso 3\) y escribe:

.\\venv\\Scripts\\Activate.ps1

python pose\_app.py \--fullscreen  
No hay que reinstalar nada.

## **MAC**

Esta tabla dice qué usar en Mac para cada cosa que la parte de Windows pide hacer:

| En Windows | En Mac |
| :---- | :---- |
| Tecla Windows (abrir la búsqueda) | ⌘ Cmd \+ barra espaciadora. La tecla ⌘ Cmd está a cada lado de la barra espaciadora |
| Enter | Return (la tecla grande de la derecha, con una flecha) |
| Ctrl \+ C / Ctrl \+ V (copiar / pegar) | ⌘ Cmd \+ C / ⌘ Cmd \+ V |
| Explorador de archivos | Finder (el ícono de la cara azul y blanca, en el Dock de abajo) |
| PowerShell | Terminal |
| Bloc de notas | TextEdit |
| Clic derecho | Control \+ clic, o clic con dos dedos en el trackpad |

### **Paso 1\. Instalar Python**

1. Entra a la página de descargas: [https\://www\.python.org/downloads/macos/](https://www.python.org/downloads/macos/)  
2. Descarga el instalador de **Python 3.11.9** (o la última 3.12): "macOS 64-bit universal2 installer". Enlace directo a la versión 3.11.9: [https\://www\.python.org/downloads/release/python-3119/](https://www.python.org/downloads/release/python-3119/)  
3. Abre el archivo .pkg descargado, con doble clic (está en la carpeta Descargas del Finder). Sigue el asistente: Continuar, Continuar, Aceptar, Instalar. Te pedirá la contraseña con la que enciendes el Mac.  
4. Al terminar se abre una carpeta de Python. Haz doble clic en **Install Certificates.command** y espera a que termine. Luego ciérralo.

### **Paso 2\. Comprobar que Python quedó instalado**

1. Abre la aplicación **Terminal**: mantén pulsada la tecla ⌘ Cmd (a cada lado de la barra espaciadora) y pulsa la barra espaciadora. Se abre un cuadro de búsqueda; escribe Terminal y pulsa Return. Se abre una ventana con una línea de texto: esa es la Terminal. También la encuentras en Launchpad \> Otros \> Terminal.  
2. Escribe:

python3 \--version  
Debe responder algo como Python 3.11.9.

### **Paso 3\. Abrir Terminal dentro de la carpeta del proyecto**

1. En la Terminal escribe cd  (con un espacio después de cd, sin pulsar Enter todavía).  
2. Arrastra la carpeta **Memoria\_y\_Cuerpo** desde el Finder (ábrelo con el ícono de la cara azul y blanca, en el Dock) hasta la ventana de la Terminal y suéltala ahí. Se escribe sola su ruta, por ejemplo /Users/tunombre/Desktop/Memoria\_y\_Cuerpo.  
3. Pulsa Return.

### **Paso 4\. Crear el entorno e instalar las librerías**

Escribe cada comando y pulsa Return. Puedes copiarlo desde esta guía (⌘ Cmd \+ C) y pegarlo en la Terminal (⌘ Cmd \+ V). Mientras instala puede pasar un rato sin que aparezca nada nuevo: espera.

python3 \-m venv venv

source venv/bin/activate  
Verás (venv) al comienzo de la línea. Entonces:

pip install \-r requirements.txt  
Puede tardar varios minutos. Espera a que termine.

### **Paso 5\. Dar permiso a la cámara**

La primera vez, macOS preguntará si la Terminal puede usar la cámara: pulsa **Aceptar**. Si lo rechazaste antes, ve a **Ajustes del Sistema \> Privacidad y seguridad \> Cámara** y activa **Terminal**. Luego cierra y vuelve a abrir la Terminal.

### **Paso 6\. Abrir el programa**

python pose\_app.py \--fullscreen  
Para probarlo en ventana normal: python pose\_app.py.

Teclas: **espacio** salta de etapa, **s** muestra u oculta el esqueleto, **q** o **Esc** cierra el programa.

### **Para abrirlo otro día**

Abre la Terminal (⌘ Cmd \+ barra espaciadora, escribe Terminal y pulsa Return), ve a la carpeta como en el Paso 3 y escribe:

source venv/bin/activate

python pose\_app.py \--fullscreen

## **Si algo falla**

* **"No se pudo abrir la cámara":** cierra otros programas que usen la cámara (Zoom, Meet, Teams). Prueba con otro número de cámara: python pose\_app.py \--camera 1 (o 2).  
* **Usar el celular como cámara:** instala DroidCam en el celular y en el computador ([https\://droidcam.app/](https://droidcam.app/)) y ábrelo con la dirección que muestra la app, por ejemplo python pose\_app.py \--camera http\://192.168.1.50:4747/video.  
* **Va lento o se traba:** usa el modelo liviano: python pose\_app.py \--lite \--fullscreen. La primera vez descarga el modelo, así que necesita internet.  
* **Error "No existe pose\_descriptors.npz":** falta ese archivo en la carpeta. Debe venir incluido.  
* **No aparecen las fotos:** revisa que la carpeta poses esté junto a pose\_app.py y que se llame exactamente así, en minúscula.  
* **Error "No module named...":** el entorno no está activado. Vuelve a ejecutar el comando de activación (verás (venv) al inicio de la línea) y luego pip install \-r requirements.txt.  
* **El esqueleto tiembla mucho:** mejora la luz frontal y verifica que se vean bien los hombros y los brazos.

## **Ajustes útiles**

Todos se escriben al final del comando de apertura:

* \--camara-seg 15: segundos con la cámara sola antes de mostrar la foto.  
* \--foto-seg 10: segundos que se queda la foto.  
* \--relato-seg 50: segundos que se muestra el relato.  
* \--final-seg 6: segundos que se muestra la frase final.

Ejemplo completo:

python pose\_app.py \--fullscreen \--relato-seg 40 \--final-seg 8

## **Cambiar los textos o las fotos**

* **Textos de cada foto** (título, relato, fuente): se editan en el archivo poses.json. Se pueden abrir con el Bloc de notas (Windows) o TextEdit (Mac). Después de guardar, cierra y vuelve a abrir el programa.  
* **Agregar o cambiar fotos:** ponlas en la carpeta poses, agrega su entrada en poses.json y ejecuta python pose\_enroll.py antes de abrir el programa.