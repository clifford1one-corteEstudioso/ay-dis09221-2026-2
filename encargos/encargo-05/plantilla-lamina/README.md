# plantilla lámina A4 — encargo 05

`lamina-a4.html` es la plantilla de la lámina entregable. Se edita como texto y se exporta a PDF desde el navegador.

## cómo usarla

1. Copiar la carpeta completa y renombrarla con el apellido (`lamina-apellido/`).
2. Abrir `lamina-a4.html` en el navegador y editarlo con cualquier editor de texto (VS Code, Sublime, Bloc de notas).
3. Reemplazar solo lo que está dentro de `<body>`:
   - encabezado: nombre del estudiante, curso, encargo
   - título del proyecto y la bajada de una línea
   - los tres párrafos: **descripción**, **fundamentos**, **proceso**
   - los ODS que aborda el proyecto (1 a 3 máximo)
   - los referentes
4. Dejar la foto del prototipo en esta misma carpeta con el nombre `prototipo.jpg`.

## exportar a PDF

`Ctrl + P` (Windows) o `Cmd + P` (Mac) →

- Destino: **Guardar como PDF**
- Márgenes: **Ninguno**
- Escala: **100%** (no "ajustar a la página")
- Desmarcar encabezados y pies de página
- Marcar **Gráficos de fondo** (para que impriman los colores de los ODS)

## notas

- La hoja es exactamente 210 × 297 mm. Si el texto se pasa de largo, la lámina se corta: recortar el texto, no achicar la tipografía.
- Los tres párrafos van en torno a 70 palabras cada uno.
- Se imprime en A4: el cuerpo de texto está en 8,4 pt, el mínimo legible. No bajar de ahí.
- Los colores oficiales de los 17 ODS están listados como comentario al final del HTML.
- Si se quiere un color de acento, cambiar la variable `--tinta` o agregar color solo en el título.
