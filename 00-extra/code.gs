/**
 * Formador de grupos por tema
 * ---------------------------
 * Script vinculado a una Google Sheet.
 *  - Los estudiantes abren la web app (index.html) e ingresan su nombre y 3 temas.
 *  - Las respuestas quedan en la hoja "Respuestas".
 *  - Desde el menú "Grupos" de la planilla se arman los grupos en la hoja "Grupos":
 *      1. Sin IA: agrupa por palabras en común (gratis, instantáneo).
 *      2. Con Claude vía API: requiere ANTHROPIC_API_KEY en Propiedades del script.
 *      3. Con Claude sin API: genera un prompt para pegar en claude.ai y luego
 *         se importa la respuesta (JSON) de vuelta.
 */

// ID de la planilla: lo que va entre /d/ y /edit en su URL.
// Déjalo vacío ('') si el script se creó desde la planilla (Extensiones → Apps Script).
const SPREADSHEET_ID = '';

const HOJA_RESPUESTAS = 'Respuestas';
const HOJA_GRUPOS = 'Grupos';
const TAMANO_GRUPO_DEFAULT = 4;

const STOPWORDS = new Set((
  'a al algo como con de del el ella ellas ellos en entre era es esa ese eso esta este esto ' +
  'hay la las le les lo los me mi mis muy no nos o para pero por que se sin sobre su sus ' +
  'tambien te tema temas todo tu un una uno unos unas y ya yo the and of me gusta gustan ' +
  'cosa cosas mucho mucha'
).split(' '));

// ───────────────────────────── Web app ─────────────────────────────

function doGet() {
  return HtmlService.createHtmlOutputFromFile('index')
    .setTitle('¿Qué tema te gusta?')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1');
}

/** Llamado desde index.html. */
function guardarRespuesta(datos) {
  const nombre = String(datos.nombre || '').trim();
  const temas = (datos.temas || []).map(t => String(t || '').trim());
  if (!nombre) throw new Error('Falta el nombre.');
  if (temas.length !== 3 || temas.some(t => !t)) throw new Error('Debes escribir los 3 temas.');
  const fila = [new Date(), nombre].concat(temas);

  const lock = LockService.getScriptLock();
  lock.waitLock(10000);
  try {
    const hoja = obtenerHojaRespuestas_();
    // Si la persona ya respondió, se reemplaza su respuesta anterior.
    const valores = hoja.getDataRange().getValues();
    for (let i = 1; i < valores.length; i++) {
      if (normalizar_(valores[i][1]) === normalizar_(nombre)) {
        hoja.getRange(i + 1, 1, 1, fila.length).setValues([fila]);
        return 'actualizada';
      }
    }
    hoja.appendRow(fila);
    return 'guardada';
  } finally {
    lock.releaseLock();
  }
}

/** Temas ya ingresados, para sugerirlos en el formulario (ayuda a que converjan). */
function obtenerTemas() {
  const temas = leerRespuestas_().flatMap(r => r.temas);
  return [...new Set(temas)].sort();
}

// ───────────────────────────── Menú ─────────────────────────────

function onOpen() {
  SpreadsheetApp.getUi().createMenu('Grupos')
    .addItem('Armar grupos (sin IA)', 'armarGruposSinIA')
    .addItem('Armar grupos con Claude (API)', 'armarGruposConClaude')
    .addSeparator()
    .addItem('Copiar prompt para claude.ai', 'mostrarPrompt')
    .addItem('Importar grupos desde claude.ai', 'importarGrupos')
    .addSeparator()
    .addItem('Cambiar tamaño de grupo', 'configurarTamano')
    .addToUi();
}

function configurarTamano() {
  const ui = SpreadsheetApp.getUi();
  const r = ui.prompt('Tamaño de grupo', `Integrantes por grupo (actual: ${tamanoGrupo_()}):`, ui.ButtonSet.OK_CANCEL);
  if (r.getSelectedButton() !== ui.Button.OK) return;
  const n = parseInt(r.getResponseText(), 10);
  if (!(n >= 2)) return ui.alert('Debe ser un número mayor o igual a 2.');
  PropertiesService.getScriptProperties().setProperty('TAMANO_GRUPO', String(n));
}

// ─────────────────────── Opción 1: sin IA ───────────────────────

/**
 * Clustering aglomerativo con tope de tamaño:
 * cada estudiante es un conjunto de palabras (sus 3 temas), la similitud es
 * Jaccard, y se van uniendo los grupos más parecidos hasta llegar al número
 * de grupos objetivo. Los grupos que quedan muy chicos se reparten.
 */
function armarGruposSinIA() {
  const alumnos = leerRespuestas_();
  if (alumnos.length < 2) return aviso_('Aún no hay suficientes respuestas.');

  const T = tamanoGrupo_();
  const maximo = T + 1;
  const objetivo = Math.max(1, Math.round(alumnos.length / T));
  const tokens = alumnos.map(a => tokenizar_(a.temas.join(' ')));

  const sim = (i, j) => jaccard_(tokens[i], tokens[j]);
  const simGrupos = (A, B) => {
    let s = 0;
    A.forEach(i => B.forEach(j => { s += sim(i, j); }));
    return s / (A.length * B.length);
  };

  let grupos = alumnos.map((_, i) => [i]);
  while (grupos.length > objetivo) {
    let mejor = null;
    for (let a = 0; a < grupos.length; a++) {
      for (let b = a + 1; b < grupos.length; b++) {
        if (grupos[a].length + grupos[b].length > maximo) continue;
        const s = simGrupos(grupos[a], grupos[b]);
        // Desempate: preferir unir grupos chicos.
        const puntaje = s - 0.001 * (grupos[a].length + grupos[b].length);
        if (!mejor || puntaje > mejor.puntaje) mejor = { a, b, puntaje };
      }
    }
    if (!mejor) break;
    grupos[mejor.a] = grupos[mejor.a].concat(grupos[mejor.b]);
    grupos.splice(mejor.b, 1);
  }

  // Repartir los grupos demasiado chicos entre los que tienen espacio.
  const minimo = Math.max(2, T - 1);
  const chicos = grupos.filter(g => g.length < minimo);
  grupos = grupos.filter(g => g.length >= minimo);
  if (!grupos.length) grupos = [chicos.shift()];
  chicos.flat().forEach(i => {
    const conEspacio = grupos.filter(g => g.length < maximo);
    const candidatos = conEspacio.length ? conEspacio : grupos;
    let destino = candidatos[0], mejorS = -1;
    candidatos.forEach(g => {
      const s = simGrupos([i], g) - 0.01 * g.length;
      if (s > mejorS) { mejorS = s; destino = g; }
    });
    destino.push(i);
  });

  escribirGrupos_(grupos.map(g => ({
    tema: etiquetaGrupo_(g.map(i => tokens[i])),
    integrantes: g.map(i => alumnos[i].nombre),
    razon: g.map(i => `${alumnos[i].nombre}: ${alumnos[i].temas.join(', ')}`).join('\n'),
  })));
}

// ───────────────── Opción 2: Claude vía API ─────────────────

/**
 * Requiere una API key de https://platform.claude.com (es distinta de la
 * suscripción de claude.ai y se cobra por uso; para 30 estudiantes son centavos).
 * Guárdala en Configuración del proyecto → Propiedades del script → ANTHROPIC_API_KEY.
 */
function armarGruposConClaude() {
  const apiKey = PropertiesService.getScriptProperties().getProperty('ANTHROPIC_API_KEY');
  if (!apiKey) return aviso_('Falta la propiedad ANTHROPIC_API_KEY (ver README).');

  const alumnos = leerRespuestas_();
  if (alumnos.length < 2) return aviso_('Aún no hay suficientes respuestas.');

  const schema = {
    type: 'object',
    properties: {
      grupos: {
        type: 'array',
        items: {
          type: 'object',
          properties: {
            tema: { type: 'string' },
            integrantes: { type: 'array', items: { type: 'integer' } },
            razon: { type: 'string' },
          },
          required: ['tema', 'integrantes', 'razon'],
          additionalProperties: false,
        },
      },
    },
    required: ['grupos'],
    additionalProperties: false,
  };

  const respuesta = UrlFetchApp.fetch('https://api.anthropic.com/v1/messages', {
    method: 'post',
    contentType: 'application/json',
    muteHttpExceptions: true,
    headers: {
      'x-api-key': apiKey,
      'anthropic-version': '2023-06-01',
      'anthropic-beta': 'server-side-fallback-2026-07-01',
    },
    payload: JSON.stringify({
      model: 'claude-opus-5-5',
      max_tokens: 16000,
      fallbacks: 'default',
      output_config: {
        effort: 'low',
        format: { type: 'json_schema', schema: schema },
      },
      messages: [{ role: 'user', content: construirPrompt_(alumnos, false) }],
    }),
  });

  const cuerpo = JSON.parse(respuesta.getContentText());
  if (respuesta.getResponseCode() !== 200) {
    return aviso_('Error de la API: ' + (cuerpo.error ? cuerpo.error.message : respuesta.getContentText()));
  }
  if (cuerpo.stop_reason === 'refusal') return aviso_('Claude no pudo procesar la solicitud.');
  if (cuerpo.stop_reason === 'max_tokens') return aviso_('La respuesta quedó cortada; intenta de nuevo.');

  const texto = cuerpo.content.filter(b => b.type === 'text').map(b => b.text).join('');
  escribirGrupos_(convertirIds_(JSON.parse(texto).grupos, alumnos));
}

// ─────────────── Opción 3: Claude sin API (copiar/pegar) ───────────────

function mostrarPrompt() {
  const alumnos = leerRespuestas_();
  if (alumnos.length < 2) return SpreadsheetApp.getUi().alert('Aún no hay suficientes respuestas.');
  const prompt = construirPrompt_(alumnos, true)
    .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  const html = HtmlService.createHtmlOutput(
    '<p style="font-family:sans-serif">Copia este texto, pégalo en claude.ai y luego usa ' +
    '<b>Grupos → Importar grupos desde claude.ai</b> con el JSON que te devuelva.</p>' +
    '<textarea style="width:100%;height:340px" onclick="this.select()">' + prompt + '</textarea>'
  ).setWidth(600).setHeight(440);
  SpreadsheetApp.getUi().showModalDialog(html, 'Prompt para Claude');
}

function importarGrupos() {
  const ui = SpreadsheetApp.getUi();
  const r = ui.prompt('Importar grupos', 'Pega aquí el JSON que respondió Claude:', ui.ButtonSet.OK_CANCEL);
  if (r.getSelectedButton() !== ui.Button.OK) return;
  const texto = r.getResponseText();
  const json = texto.slice(texto.indexOf('{'), texto.lastIndexOf('}') + 1);
  try {
    escribirGrupos_(convertirIds_(JSON.parse(json).grupos, leerRespuestas_()));
  } catch (e) {
    ui.alert('No pude leer el JSON: ' + e.message);
  }
}

// ───────────────────────────── Utilidades ─────────────────────────────

function construirPrompt_(alumnos, paraChat) {
  const lista = alumnos
    .map((a, i) => `${i + 1}. ${a.nombre} — temas: ${a.temas.map(t => `"${t}"`).join(', ')}`)
    .join('\n');
  return [
    `Tengo ${alumnos.length} estudiantes de diseño; cada uno indicó 3 temas que le interesan.`,
    `Agrúpalos en grupos de aproximadamente ${tamanoGrupo_()} personas según afinidad temática.`,
    'Cada estudiante debe quedar en exactamente un grupo. Intenta que los grupos tengan tamaños parecidos;',
    'si alguien no calza con nadie, ponlo en el grupo más cercano conceptualmente.',
    'Para cada grupo da un nombre de tema corto que los una y una razón breve.',
    'En "integrantes" usa los números de la lista.',
    paraChat ? '\nResponde SOLO con JSON con esta forma: {"grupos":[{"tema":"...","integrantes":[1,2,3],"razon":"..."}]}' : '',
    '\nEstudiantes:',
    lista,
  ].join('\n');
}

/** Convierte los números de la lista a nombres y agrega a quien Claude haya omitido. */
function convertirIds_(grupos, alumnos) {
  const asignados = new Set();
  const salida = grupos.map(g => ({
    tema: g.tema,
    razon: g.razon || '',
    integrantes: g.integrantes
      .map(n => alumnos[n - 1])
      .filter(a => a && !asignados.has(a.nombre) && asignados.add(a.nombre))
      .map(a => a.nombre),
  }));
  const faltan = alumnos.filter(a => !asignados.has(a.nombre)).map(a => a.nombre);
  if (faltan.length) salida.push({ tema: '(sin asignar)', razon: 'Revisar a mano', integrantes: faltan });
  return salida;
}

function escribirGrupos_(grupos) {
  const ss = planilla_();
  const hoja = ss.getSheetByName(HOJA_GRUPOS) || ss.insertSheet(HOJA_GRUPOS);
  hoja.clear();
  const filas = grupos.map((g, i) => [i + 1, g.tema, g.integrantes.length, g.integrantes.join(', '), g.razon]);
  hoja.getRange(1, 1, 1, 5).setValues([['Grupo', 'Tema', 'N°', 'Integrantes', 'Por qué']]).setFontWeight('bold');
  if (filas.length) hoja.getRange(2, 1, filas.length, 5).setValues(filas);
  hoja.setColumnWidth(4, 360).setColumnWidth(5, 360);
  hoja.getRange('D:E').setWrap(true);
  aviso_(`Listo: ${grupos.length} grupos en la hoja "${HOJA_GRUPOS}".`);
}

function planilla_() {
  return SPREADSHEET_ID ? SpreadsheetApp.openById(SPREADSHEET_ID) : SpreadsheetApp.getActive();
}

/** Muestra un aviso en la planilla, o en el registro si se ejecuta desde el editor. */
function aviso_(mensaje) {
  try {
    SpreadsheetApp.getUi().alert(mensaje);
  } catch (e) {
    Logger.log(mensaje);
  }
}

function leerRespuestas_() {
  const valores = obtenerHojaRespuestas_().getDataRange().getValues().slice(1);
  return valores
    .filter(f => f[1] && f[2])
    .map(f => ({
      nombre: String(f[1]).trim(),
      temas: [f[2], f[3], f[4]].map(t => String(t || '').trim()).filter(Boolean),
    }));
}

function obtenerHojaRespuestas_() {
  const ss = planilla_();
  let hoja = ss.getSheetByName(HOJA_RESPUESTAS);
  if (!hoja) {
    hoja = ss.insertSheet(HOJA_RESPUESTAS);
    hoja.appendRow(['Fecha', 'Nombre', 'Tema 1', 'Tema 2', 'Tema 3']);
    hoja.getRange('1:1').setFontWeight('bold');
  }
  return hoja;
}

function tamanoGrupo_() {
  const v = PropertiesService.getScriptProperties().getProperty('TAMANO_GRUPO');
  return parseInt(v, 10) || TAMANO_GRUPO_DEFAULT;
}

function normalizar_(texto) {
  return String(texto).toLowerCase().normalize('NFD').replace(/[̀-ͯ]/g, '').trim();
}

/** Palabras sin tildes, sin stopwords y con un "stemming" mínimo (plurales). */
function tokenizar_(texto) {
  const palabras = normalizar_(texto).split(/[^a-z0-9ñ]+/)
    .filter(p => p.length > 2 && !STOPWORDS.has(p))
    .map(p => p.length > 5 && p.endsWith('es') ? p.slice(0, -2)
            : p.length > 4 && p.endsWith('s') ? p.slice(0, -1) : p);
  return new Set(palabras);
}

function jaccard_(A, B) {
  if (!A.size || !B.size) return 0;
  let inter = 0;
  A.forEach(p => { if (B.has(p)) inter++; });
  return inter / (A.size + B.size - inter);
}

/** Las palabras más repetidas del grupo sirven como nombre provisorio. */
function etiquetaGrupo_(conjuntos) {
  const cuenta = {};
  conjuntos.forEach(s => s.forEach(p => { cuenta[p] = (cuenta[p] || 0) + 1; }));
  const top = Object.keys(cuenta).sort((a, b) => cuenta[b] - cuenta[a]).slice(0, 3);
  return top.join(' / ') || '(variado)';
}
