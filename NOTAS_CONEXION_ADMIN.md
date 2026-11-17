# NOTAS PARA LA CONEXIÓN CON EL ADMIN

Lee este archivo antes de tocar nada del flujo de cuotas/cierre. Está escrito para que otro
agente pueda terminar la conexión con el menú admin real **leyendo solo este documento**.

---

## 1) Qué quedó implementado

### Menú principal
- Botones **Ver catálogo**, **Cuotas**, **Cierre** y **Salir** (`MenuPrincipal.ui` / `.cpp`).
- `MenuPrincipal::salir()` ya no cierra el programa: emite `sesionCerrada()` y vuelve al login.

### Cuotas (mis archivos)
- `VentanaCuotas`: pestañas **Activas** y **Completadas** con buscador por nombre.
  - Activas: segunda línea `Caja: X · Fecha: Y · Falta $Z`.
  - Completadas: se muestran **siempre** (también después del Cierre) con estado
    `Pendiente de envío al admin` o `Enviada al admin`.
  - Se refresca al volver del detalle.
- `VentanaDetalleCuota`: labels **"Falta por pagar: $X.XX"** y **"Pagado: $Y.YY"**, definidos
  en el `.ui`; el `.cpp` solo asigna texto. Tras registrar un pago se recalculan en vivo sin
  cerrar la ventana. No hay límite de un pago por mes; el último pago se recorta al saldo.
- Al llegar a cero: cartel **"La cuota ha sido completada con éxito"** con el nombre, se crea
  la factura, se guarda en `cuotas.json` con `QSaveFile`, la ventana se cierra y vuelve a la
  lista. **No se envía nada al admin en ese momento.**
- `RepositorioCuotas`: única clase que lee/escribe `cuotas.json`. Agrega los campos nuevos
  `enviada_admin` (factura), `fecha_envio_admin` (factura) y `enviado_admin` (cada pago),
  sin quitar los campos originales de Iván. Los registros viejos sin esos campos cuentan
  como **no enviados**.

### Cierre
- Fuentes: `FuenteHistorialVentas("Efectivo")`, `FuenteHistorialVentas("Tarjeta")` → leen
  `datos/historial.json`; `FuenteCuotas` → lee `datos/cuotas.json` y envía:
  - pagos con `enviado_admin == false` como `Cuotas` (referencia `cliente|fecha|monto`);
  - cuotas completadas con `enviada_admin == false` como `Cuota completada`
    (referencia = número de factura).
- `ServicioCierre` agrega los tipos `Efectivo`, `Tarjeta`, `Cuotas`, `Cuota completada`.
- **Orden de seguridad obligatorio** en `MenuPrincipal::realizarCierre()`:
  1. generar cierre;
  2. respaldo de `historial.json`, `cuotas.json` y `bitacora.json` en `datos/respaldo/` con sello `yyyyMMdd_HHmmss`;
  3. guardar en `bitacora.json` con `QSaveFile`;
  4. releer la bitácora y verificar que contiene todo lo enviado (comparando `referencia`);
  5. solo si pasó: borrar de `historial.json` **solo Efectivo/Tarjeta** y marcar en `cuotas.json`
     los pagos (`enviado_admin = true`) y facturas (`enviada_admin = true` + `fecha_envio_admin`).
- Cierre repetido el mismo día: `RepositorioBitacora::guardar()` **agrega** registros nuevos
  y recalcula totales; nunca reemplaza. Si no hay nada nuevo: *"No hay datos nuevos para enviar"*.
- Confirmación: cantidad y total por tipo, pagos de cuotas, cuotas completadas, total general y el aviso
  *"Se enviarán a la bitácora y se borrarán los datos locales de tarjeta y efectivo. Las cuotas se conservan en el menú de cuotas."*
  Botones **Confirmar** / **Cancelar**. Al terminar: *"Cierre enviado a la bitácora"*.

---

## 2) Puntos `TODO(ADMIN)` (búsquedas exactas: `TODO(ADMIN)`)

| # | Archivo | Función | Línea | Qué hace hoy |
|---|---|---|---|---|
| 1 | `proyecto progra/MenuPrincipal.cpp` | `MenuPrincipal::realizarCierre()` | 313 | `bitacora.guardar(cierre, &error)` es la bandeja de entrega actual. Reemplazar por el envío real al menú admin, conservando el orden generar → respaldo → guardar → releer/verificar → limpiar/marcar. |

Comentario exacto a buscar:

```cpp
// TODO(ADMIN): reemplazar por el envío real al menú admin
```

No hay otros `TODO(ADMIN)`. `EnvioAdminLocal` ya **no se dispara** al completar una cuota
(antes sí, en `VentanaDetalleCuota::completarPago()`); la clase quedó como implementación de
referencia pero hoy no se usa en el flujo.

---

## 3) Formato exacto de los datos

### 3.1 Cierre (elemento de `datos/bitacora.json`)

```json
{
    "fecha": "16/11/2026",
    "hora_generacion": "16/11/2026 23:10:00",
    "total_general": 1404.94,
    "resumen": [
        { "tipo": "Efectivo", "cantidad": 1, "total": 250.0 },
        { "tipo": "Tarjeta", "cantidad": 1, "total": 243.47 },
        { "tipo": "Cuotas", "cantidad": 10, "total": 911.47 },
        { "tipo": "Cuota completada", "cantidad": 0, "total": 0.0 }
    ],
    "detalle": [
        {
            "tipo": "Efectivo",
            "caja": "anderson",
            "fecha": "16/11/2026 21:37",
            "cliente": "ivan1",
            "monto": 250.0,
            "ultimos4": "",
            "referencia": ""
        },
        {
            "tipo": "Cuotas",
            "caja": "anderson",
            "fecha": "16/11/2026 22:19:21",
            "cliente": "ivan bolivar",
            "monto": 91.15,
            "ultimos4": "",
            "referencia": "ivan bolivar|16/11/2026 22:19:21|91.15"
        },
        {
            "tipo": "Cuota completada",
            "caja": "anderson",
            "fecha": "16/11/2026 22:20:19",
            "cliente": "ivan bolivar",
            "monto": 911.47,
            "ultimos4": "",
            "referencia": "F-000002"
        }
    ]
}
```

`referencia` es el id estable anti-duplicados: factura (`F-000002`) o pago
(`cliente|fecha|monto` con 2 decimales). `RepositorioBitacora::guardar()` y
`verificarBitacora()` la comparan. **No se guardan números de tarjeta, CVV ni vencimiento**;
`ultimos4` queda vacío en cuotas.

### 3.2 Cuota completada (elemento de `datos/cuotas.json`)

```json
{
    "caja": "anderson",
    "cliente": "ivan bolivar",
    "completada": true,
    "fecha": "16/11/2026 22:19:21",
    "meses": 3,
    "meses_restantes": 0,
    "pago_mensual": 91.14699999999999,
    "pagos": [
        { "fecha": "16/11/2026 22:19:21", "monto": 91.14699999999999, "enviado_admin": false }
    ],
    "porcentaje_aumento": 1.5,
    "total_con_aumento": 911.4699999999999,
    "total_sin_aumento": 898,
    "factura": {
        "numero_factura": "F-000002",
        "fecha_finalizacion": "16/11/2026 22:20:19",
        "cliente": "ivan bolivar",
        "caja": "anderson",
        "producto": [
            { "cantidad": 1, "idProducto": 8, "nombre": "Webcam Full HD", "precio": 549 },
            { "cantidad": 1, "idProducto": 5, "nombre": "Mouse inalámbrico Logitech", "precio": 349 }
        ],
        "fecha_compra": "16/11/2026 22:19:21",
        "total_con_aumento": 911.4699999999999,
        "total_pagado": 911.4699999999999,
        "pagos": [
            { "fecha": "16/11/2026 22:19:21", "monto": 91.14699999999999, "enviado_admin": false }
        ],
        "enviada_admin": false,
        "fecha_envio_admin": ""
    }
}
```

Después de un Cierre exitoso, `enviada_admin` pasa a `true`, `fecha_envio_admin` a la fecha
del cierre y cada `enviado_admin` a `true`. Los registros siguen en `cuotas.json` para siempre.

`datos/pendiente_admin.json` existe de la implementación anterior con el mismo esquema de
factura, pero **ya no se escribe** al completar cuotas.

---

## 4) Dónde quedan los datos y cómo releerlos

La ruta la resuelve `carpetaDatos()` en `proyecto progra/clases/datos.h`: sube desde
`QCoreApplication::applicationDirPath()` hasta encontrar `*.slnx`/`*.sln` y usa `<raíz>/datos`.
Si no lo encuentra, usa `<dir del exe>/datos`.

| Archivo | Lector / escritor | Cómo releerlo |
|---|---|---|
| `datos/historial.json` | `cargarHistorial()` / `guardarHistorial()` en `datos.h`; `FuenteHistorialVentas`; `MenuPrincipal::borrarVentasEnviadas()` | `cargarHistorial()` llena la global `historialCompras` |
| `datos/cuotas.json` | `RepositorioCuotas` (única clase) | `RepositorioCuotas repositorio; QList<Cuota> cuotas; repositorio.cargar(&cuotas, &error);` |
| `datos/bitacora.json` | `RepositorioBitacora` (única clase) | `RepositorioBitacora bitacora; QList<CierreDia> cierres; bitacora.todos(&cierres, &error);` |
| `datos/pendiente_admin.json` | `EnvioAdminLocal` (hoy sin uso en el flujo) | `QJsonDocument::fromJson` sobre `rutaArchivo("pendiente_admin.json")` |
| `datos/respaldo/*` | `copiarRespaldo()` en `MenuPrincipal.cpp` | Copias con sello `yyyyMMdd_HHmmss` |

**Quién más lee `historial.json`:** `historial.cpp` (pantalla Historial del cliente) y
`VentanaCuotas::buscarProductos()` (busca la compra `"Tarjeta (cuotas)"` para mostrar los
productos del detalle). Por eso el Cierre **no borra** `"Tarjeta (cuotas)"`: solo Efectivo y
Tarjeta. Borrar esos dos tipos es lo previsto por las reglas del proyecto; la pantalla Historial
simplemente dejará de mostrar esas compras ya enviadas.

---

## 5) Qué falta para conectar el admin real y cómo probarlo

### Falta
1. Un **menú admin receptor** (no existe): hoy `Administracion.cpp` solo edita
   `productos.json` y `clientes.json`; no lee `bitacora.json` ni `pendiente_admin.json`.
2. Reemplazar `bitacora.guardar(cierre, &error)` (punto 1 de la tabla) por el envío real,
   con confirmación de recepción del admin.
3. Ese envío real debe devolver éxito/error; solo entonces ejecutar la limpieza y el marcado
   (pasos 4 y 5 del orden de seguridad). Si falla, no borrar ni marcar y mostrar el error.
4. Confirmar la recepción actualizando `enviada_admin` / `enviado_admin` desde el admin real
   (hoy lo hace el Cierre local).

### Cómo probar sin admin
1. Abrir **Cuotas** → una activa → *Completar pago de este mes* varias veces seguidas:
   "Falta por pagar" baja hasta `$0.00`, sale el cartel de completada, la cuota pasa a
   **Completadas** con *Pendiente de envío al admin* y **no** aparece en `pendiente_admin.json`.
2. Hacer un Cierre: revisar `datos/bitacora.json` (debe contener Efectivo, Tarjeta, Cuotas y
   Cuota completada), ver que `historial.json` perdió solo Efectivo/Tarjeta, y que en
   `cuotas.json` los pagos/facturas enviados quedaron en `true` con `fecha_envio_admin`.
3. Repetir Cierre el mismo día: **no debe repetir** nada ya enviado y debe decir
   *"No hay datos nuevos para enviar"* si no hay pendientes.
4. Dañar un JSON (por ejemplo `cuotas.json`) y correr un Cierre: debe mostrar error y
   **no** borrar ni marcar nada (los respaldos quedan en `datos/respaldo/`).
5. `Select-String -Pattern 'TODO\(ADMIN\)'` debe listar exactamente el punto de la tabla.

---

## 6) Decisiones tomadas

- Se usa la **paleta real actual** (`#F5F3EE` / `#899B83`), no la antigua `#f9f9f6` / `#2d5043`.
- Se dio factura `F-000002` a la cuota ya completada `ivan bolivar` para que aparezca en
  **Completadas** (`josue` ya tenía `F-000001`); las que se completen después siguen la misma
  regla con número consecutivo.
- Se corrigió `FacturaCuota::fromJson()`: antes leía `F-000001` con `mid(1)` dando `-1`, por lo
  que ninguna completada aparecía y cada guardado borraba su factura. Ahora usa `mid(2)`.
- Se trajeron de `main` las versiones nuevas del catálogo (`menuPaginacion.*`) y del carrito
  (`carrito.*`, `Carrito.h`, `CuotaCompletada.*`, `PagoTarjeta.*`, `pagoefectivo.cpp`, `recibo.cpp`,
  `datos.h`, `producto.h`) y se les reaplicó la paleta real al catálogo y al bloque admin.
- Se vistieron también los dos `.ui` huérfanos (`practicaproyecto.ui`, `Widget.ui`).
- El `esAdmin` de `MenuPrincipal` sigue sin usarse (`Q_UNUSED(esAdmin)`): el admin que falta
  es el receptor de datos, no el administrador de catálogo/usuarios que ya existe.

---

## 7) Conexión final con el menú admin (rama `menu-principal-completo`)

Estado actual de esta rama:

- Se copiaron **todos los archivos de `main`** (menú admin con pestañas Productos,
  Cuentas, **Cuotas** y **Cobros por usuario**) y se integraron los botones **Cuotas** y
  **Cierre** de `menu-principal` con el estilo/UI actual de `main`.
- Se conservaron los datos de demostración (`datos/*.json`) que tenían las cuotas
  completadas con factura (`F-000001`…`F-000003`) para poder probar la conexión.

### Cómo quedó conectado

1. **Cierre → Cobros por usuario** (`Administracion.cpp`):
   - `recolectarCobros()` lee ahora `bitacora.json` y recupera los pagos **Efectivo** y
     **Tarjeta** que el Cierre envió (y que borró de `historial.json`), sin duplicar los
     que todavía no se enviaron.
   - Las **facturas** de las cuotas completadas (`factura.numero_factura` en
     `cuotas.json`) se muestran como cobros de tipo `Factura`, con número de factura y
     productos en el detalle. Los planes activos siguen aportando sus pagos.
   - El detalle de cada cobro indica el tipo (`Efectivo` / `Tarjeta` / `Cuotas` / `Factura`),
     el número de factura y los productos.
2. **Cuotas → pestaña Cuotas del admin** (`Administracion.cpp`):
   - `actualizarCuotas()` lee `cuotas.json` (el mismo archivo que escribe el flujo de
     cuotas del cliente) y muestra cada plan con estado **Activa** o
     **Completada (Factura F-…)**, además de su progreso de pagos.
3. `MenuPrincipal.cpp`: los mensajes del Cierre ya indican que los datos se envían al
   menú de administración (Cobros y Cuotas).

El orden de seguridad del Cierre (generar → respaldar → guardar → releer/verificar →
limpiar/marcar) se conserva intacto: `bitacora.json` sigue siendo la bandeja de entrega
que ahora consume el menú admin.

---

## 8) Reglas de subida al admin (ajuste final)

1. **Cobros por usuario** (`recolectarCobros()`): muestra **únicamente Efectivo y Tarjeta**,
   y solo los que ya fueron enviados por el Cierre (lee `bitacora.json`). No aparecen
   cuotas ni facturas aquí. Antes del Cierre no se muestra ningún cobro de contado,
   aunque esté en `historial.json`.
2. **Cuotas activas**: no se suben hasta que el cajero pulse **Cierre**. El Cierre envía
   sus pagos (`tipo = Cuotas`) y los marca con `enviado_admin = true`. La pestaña Cuotas
   del admin solo muestra un plan activo cuando tiene algún pago con esa marca.
3. **Cuotas completadas**: se suben **al instante de completarse**. Al generar la factura
   se guarda con `enviada_admin = true` y `fecha_envio_admin` (`VentanaDetalleCuota` y
   `RepositorioCuotas::completar()` ya no la fuerzan a `false`). `FuenteCuotas` dejó de
   enviar el tipo `Cuota completada`, así que el Cierre no las reenvía.
4. Los datos de prueba (`datos/cuotas.json`) tienen sus 3 facturas marcadas como
   `enviada_admin: true` para reflejar esta regla.
