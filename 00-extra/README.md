# Formador de grupos por tema

Web app hecha con Apps Script: cada estudiante ingresa 3 temas que le gustan y luego, desde la planilla, se arman grupos por afinidad.

## Instalación

1. Crea una Google Sheet nueva.
2. Ve a **Extensiones → Apps Script**.
3. Pega el contenido de `code.gs` en `Código.gs`.
4. Crea un archivo HTML llamado `index` (**+ → HTML**) y pega `index.html`.
5. Guarda y recarga la planilla: aparece el menú **Grupos**.
6. **Implementar → Nueva implementación → Aplicación web**
   - Ejecutar como: **Yo**
   - Quién tiene acceso: **Cualquier usuario** (o solo tu dominio UC)
7. Comparte la URL `/exec` con los estudiantes (o como código QR en clases).

## Cómo armar los grupos

En el menú **Grupos** hay tres opciones:

| Opción | Costo | Cómo funciona |
|---|---|---|
| **Armar grupos (sin IA)** | Gratis | Agrupa según las palabras en común entre los temas de cada estudiante. Funciona bien si los temas se parecen ("música", "música electrónica"); no entiende sinónimos ("cine" ≠ "películas"). |
| **Copiar prompt para claude.ai** + **Importar grupos** | Gratis (usa tu cuenta de claude.ai) | Genera un texto que se pega en claude.ai. Claude responde con un JSON, que luego se pega de vuelta en la planilla. |
| **Armar grupos con Claude (API)** | Unos pocos centavos de dólar | Igual que la anterior, pero automático. Necesita una API key. |

El resultado queda en la hoja **Grupos**, donde se puede editar a mano. El tamaño de los grupos se cambia con **Grupos → Cambiar tamaño de grupo** (por defecto son 4).

El formulario sugiere los temas que ya ingresaron otros estudiantes, lo que ayuda a que escriban lo mismo con las mismas palabras y mejora la agrupación sin IA.

## API key (solo para la opción con API)

La suscripción de claude.ai **no** incluye acceso a la API. Es una cuenta aparte, con cobro por uso:

1. Crea una key en <https://platform.claude.com> (hay que cargar crédito).
2. En Apps Script: **Configuración del proyecto → Propiedades del script → Agregar**
   - Propiedad: `ANTHROPIC_API_KEY`
   - Valor: tu key
3. No pegues la key en el código. La propiedad del script no queda visible para los estudiantes.

Si una persona responde dos veces con el mismo nombre, se reemplaza su respuesta anterior.
