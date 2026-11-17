#pragma once
#pragma once
#include <QList>
#include <QSet>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QPointer>
#include <QObject>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QJsonParseError>
#include <functional>
#include <utility>
#include "Cliente.h"
#include "Producto.h"
#include "compra.h"

inline QList<Cliente> clientes;
inline QList<Producto> productos;
inline QList<Compra> historialCompras;   // compras de todos los clientes
inline QList<ItemCompra> carrito;        // carrito de compras del usuario actual
inline Cliente usuarioActual;  // variable global para almacenar el usuario actual

struct ObservadorActualizacionPagos
{
    QPointer<QObject> objeto;
    std::function<void()> actualizar;
};

inline QList<ObservadorActualizacionPagos>& observadoresActualizacionPagos()
{
    static QList<ObservadorActualizacionPagos> observadores;
    return observadores;
}

inline void registrarActualizacionPagos(QObject* objeto, std::function<void()> actualizar)
{
    auto& observadores = observadoresActualizacionPagos();
    for (ObservadorActualizacionPagos& observador : observadores) {
        if (observador.objeto == objeto) {
            observador.actualizar = std::move(actualizar);
            return;
        }
    }
    observadores.append({ objeto, std::move(actualizar) });
}

inline void notificarActualizacionPagos()
{
    auto& observadores = observadoresActualizacionPagos();
    for (int i = observadores.size() - 1; i >= 0; --i) {
        if (observadores[i].objeto.isNull()) {
            observadores.removeAt(i);
            continue;
        }
        observadores[i].actualizar();
    }
}

inline QString carpetaDatos()
{
    static QString ruta;
    if (!ruta.isEmpty())
        return ruta;

    QDir dir(QCoreApplication::applicationDirPath());
    do {
        if (!dir.entryList({ "*.slnx", "*.sln" }, QDir::Files).isEmpty()) {
            ruta = dir.filePath("datos");
            break;
        }
    } while (dir.cdUp());

    if (ruta.isEmpty())   // respaldo: programa ejecutado fuera del árbol del proyecto
        ruta = QCoreApplication::applicationDirPath() + "/datos";

    QDir().mkpath(ruta);  // crea la carpeta si no existe
    return ruta;
}

inline QString rutaArchivo(const QString& nombre)
{
    return carpetaDatos() + "/" + nombre;
}
// ---------------------------------------------------------------
// Funciones genéricas: sirven para cualquier clase T que tenga
//   QJsonObject T::toJson() const
//   void        T::fromJson(const QJsonObject&)
//   y constructor por defecto
// ---------------------------------------------------------------
template <typename T>
inline void guardarLista(const QList<T>& lista, const QString& nombreArchivo)
{
    QJsonArray arreglo;
    for (const T& elemento : lista)
        arreglo.append(elemento.toJson());

    QFile archivo(rutaArchivo(nombreArchivo));
    if (archivo.open(QIODevice::WriteOnly)) {
        archivo.write(QJsonDocument(arreglo).toJson());
        archivo.close();
    }
}

template <typename T>
inline void cargarLista(QList<T>& lista, const QString& nombreArchivo)
{
    lista.clear();
    QFile archivo(rutaArchivo(nombreArchivo));
    if (!archivo.open(QIODevice::ReadOnly))
        return;   // aún no existe el archivo: primera ejecución

    QJsonDocument doc = QJsonDocument::fromJson(archivo.readAll());
    archivo.close();

    for (const QJsonValue& valor : doc.array()) {
        T elemento;
        elemento.fromJson(valor.toObject());
        lista.append(elemento);
    }
}

// ---------------------------------------------------------------
// Atajos por clase (misma interfaz que antes)
// ---------------------------------------------------------------
inline void guardarClientes() { guardarLista(clientes, "clientes.json"); }
inline void cargarClientes() { cargarLista(clientes, "clientes.json"); }

inline void guardarProductos() { guardarLista(productos, "productos.json"); }
inline void cargarProductos() { cargarLista(productos, "productos.json"); }

inline void guardarHistorial() { guardarLista(historialCompras, "historial.json"); }
inline void cargarHistorial() { cargarLista(historialCompras, "historial.json"); }

inline bool cargarRegistrosCuotas(QJsonArray& registros, QString* error = nullptr)
{
    QFile archivo(rutaArchivo(QStringLiteral("cuotas.json")));
    if (!archivo.exists()) {
        registros = QJsonArray();
        return true;
    }
    if (!archivo.open(QIODevice::ReadOnly)) {
        if (error)
            *error = archivo.errorString();
        return false;
    }
    QJsonParseError errorJson;
    const QJsonDocument documento = QJsonDocument::fromJson(archivo.readAll(), &errorJson);
    archivo.close();
    if (errorJson.error != QJsonParseError::NoError || !documento.isArray()) {
        if (error)
            *error = errorJson.errorString().isEmpty()
                ? QStringLiteral("El archivo de cuotas no contiene una lista válida.")
                : errorJson.errorString();
        return false;
    }
    registros = documento.array();
    return true;
}

inline bool guardarRegistrosCuotas(const QJsonArray& registros, QString* error = nullptr)
{
    QSaveFile archivo(rutaArchivo(QStringLiteral("cuotas.json")));
    if (!archivo.open(QIODevice::WriteOnly)) {
        if (error)
            *error = archivo.errorString();
        return false;
    }
    const QByteArray datos = QJsonDocument(registros).toJson();
    if (archivo.write(datos) != datos.size()) {
        if (error)
            *error = archivo.errorString();
        archivo.cancelWriting();
        return false;
    }
    if (!archivo.commit()) {
        if (error)
            *error = archivo.errorString();
        return false;
    }
    return true;
}

inline bool agregarRegistroCuotas(const QJsonObject& registro, QString* error = nullptr)
{
    QJsonArray registros;
    if (!cargarRegistrosCuotas(registros, error))
        return false;
    registros.append(registro);
    return guardarRegistrosCuotas(registros, error);
}

// Opcional: para guardar/cargar todo de una vez
inline void guardarTodo() { guardarClientes(); guardarProductos(); guardarHistorial(); }
inline void cargarTodo() { cargarClientes();  cargarProductos(); cargarHistorial(); }

// ---------------------------------------------------------------
// Carrito de compras (sesión, no se guarda en disco)
// ---------------------------------------------------------------
inline int cantidadEnCarrito(int idProducto)
{
    int cantidad = 0;
    for (const ItemCompra& item : carrito)
        if (item.idProducto == idProducto)
            cantidad += item.cantidad;
    return cantidad;
}

inline bool agregarAlCarrito(int idProducto, int cantidad)
{
    if (cantidad <= 0)
        return false;

    const Producto* producto = nullptr;
    for (const Producto& p : productos) {
        if (p.id == idProducto) { producto = &p; break; }
    }
    if (!producto)
        return false;

    if (producto->stock < cantidadEnCarrito(idProducto) + cantidad)
        return false;   // no hay stock suficiente

    // si ya está, suma cantidades; si no, agrega la línea
    for (ItemCompra& item : carrito) {
        if (item.idProducto == idProducto) {
            item.cantidad += cantidad;
            return true;
        }
    }

    ItemCompra item;
    item.idProducto = idProducto;
    item.nombre = producto->nombre;
    item.precio = producto->precio;
    item.cantidad = cantidad;
    carrito.append(item);
    return true;
}

inline bool quitarDelCarrito(int idProducto)
{
    for (int i = 0; i < carrito.size(); ++i) {
        if (carrito[i].idProducto == idProducto) {
            carrito.removeAt(i);
            return true;
        }
    }
    return false;
}

inline double carritoTotal()
{
    double total = 0.0;
    for (const ItemCompra& item : carrito)
        total += item.precio * item.cantidad;
    return total;
}

inline int carritoCantidadTotal()
{
    int cantidad = 0;
    for (const ItemCompra& item : carrito)
        cantidad += item.cantidad;
    return cantidad;
}

inline void vaciarCarrito() { carrito.clear(); }

// meses del plazo según el índice del combo de método de pago del carrito
inline int mesesDelMetodo(int indiceMetodo)
{
    switch (indiceMetodo) {
    case 2: return 3;
    case 3: return 6;
    case 4: return 12;
    case 5: return 24;
    default: return 0;
    }
}

// total a pagar con el recargo del 0.5% por cada mes de plazo
// sobre el precio original (3 meses = 1.5% más, 12 meses = 6% más...)
inline double totalConInteres(double total, int meses)
{
    return (meses > 0) ? total * (1.0 + 0.005 * meses) : total;
}

// sincroniza usuarioActual con la lista de clientes para que el saldo
// mostrado y cobrado esté siempre al día (p. ej. tras un depósito)
inline void refrescarUsuarioActual()
{
    for (const Cliente& cliente : clientes) {
        if (cliente.getId() == usuarioActual.getId()) {
            usuarioActual = cliente;
            break;
        }
    }
}

// agrega dinero ficticio al saldo del usuario actual y lo guarda en clientes.json
inline bool depositarSaldo(double monto)
{
    if (monto <= 0.0)
        return false;

    usuarioActual.saldo += static_cast<float>(monto);
    for (Cliente& cliente : clientes) {
        if (cliente.getId() == usuarioActual.getId()) {
            cliente.setSaldo(usuarioActual.saldo);
            break;
        }
    }
    guardarClientes();
    return true;
}

// cierra la compra: descuenta stock, guarda el historial y vacía el
// carrito (no toca el saldo del usuario). Devuelve la compra registrada.
inline Compra finalizarCompra(double total, int meses, const QString& metodo)
{
    // descuenta el stock de cada producto
    for (const ItemCompra& item : carrito) {
        for (Producto& p : productos) {
            if (p.id == item.idProducto) {
                p.stock -= item.cantidad;
                break;
            }
        }
    }
    guardarProductos();

    // registra la compra en el historial
    Compra compra;
    compra.clienteId = usuarioActual.getId();
    compra.fecha = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");
    compra.total = total;
    compra.meses = (meses > 0) ? meses : 0;
    compra.cuotaMensual = (meses > 0) ? total / meses : 0.0;
    compra.metodo = metodo;
    compra.items = carrito;
    historialCompras.append(compra);
    guardarHistorial();

    vaciarCarrito();
    notificarActualizacionPagos();
    return compra;
}

// pago con tarjeta (de contado o a plazos): el pago es inmediato.
// si meses > 0, la compra se hace a plazos (3, 6, 12 o 24 meses):
//   el precio sube 0.5% por cada mes de plazo y se cobra la primera
//   cuota (total con aumento / meses) el día de la compra
// si recibo != nullptr, devuelve la compra registrada para mostrarla en el recibo
inline bool pagarCarrito(int meses = 0, Compra* recibo = nullptr)
{
    if (carrito.isEmpty())
        return false;

    const double total = carritoTotal();
    const double totalFinal = totalConInteres(total, meses);

    QString metodo = (meses > 0) ? "Tarjeta (cuotas)" : "Tarjeta";

    if (recibo)
        *recibo = finalizarCompra(totalFinal, meses, metodo);
    else
        finalizarCompra(totalFinal, meses, metodo);
    return true;
}

// pago en efectivo: el cliente entrega dinero físico y se calcula el
// vuelto (efectivo dado - total). No usa el saldo de la cuenta.
// devuelve false si el efectivo no alcanza (saldo insuficiente).
// si recibo != nullptr, devuelve la compra registrada para mostrarla en el recibo
inline bool pagarCarritoEfectivo(double efectivoRecibido, double* vuelto = nullptr, Compra* recibo = nullptr)
{
    if (carrito.isEmpty())
        return false;
    if (vuelto)
        *vuelto = 0.0;

    const double total = carritoTotal();
    const qint64 diferencia = qRound64(efectivoRecibido * 100.0) - qRound64(total * 100.0);

    if (diferencia < 0)
        return false;   // efectivo insuficiente

    if (vuelto)
        *vuelto = diferencia / 100.0;

    if (recibo)
        *recibo = finalizarCompra(total, 0, "Efectivo");
    else
        finalizarCompra(total, 0, "Efectivo");
    return true;
}

// ---------------------------------------------------------------
// Códigos de producto (6 dígitos, únicos)
// ---------------------------------------------------------------

// Devuelve un código de 6 dígitos que puede comenzar con cero.
inline QString generarCodigoUnico(const QSet<QString>& usados)
{
    QString codigo;
    do {
        codigo = QStringLiteral("%1").arg(QRandomGenerator::global()->bounded(1000000), 6, 10, QChar('0'));
    } while (usados.contains(codigo));
    return codigo;
}

// Código nuevo para un producto que se vaya a agregar a "productos".
inline QString nuevoCodigoProducto()
{
    QSet<QString> usados;
    for (const Producto& p : productos)
        usados.insert(p.codigo);
    return generarCodigoUnico(usados);
}

// Se asegura de que TODOS los productos tengan un código válido y no repetido.
// Los productos que no lo tengan (por ejemplo, un productos.json anterior) reciben uno
// y el archivo se guarda. Devuelve true si algún código fue asignado.
inline bool asegurarCodigos()
{
    QSet<QString> usados;
    QList<int> pendientes;

    for (int i = 0; i < productos.size(); ++i) {
        const QString& c = productos[i].codigo;
        if (Producto::codigoValido(c) && !usados.contains(c))
            usados.insert(c);
        else
            pendientes.append(i);
    }

    for (int i : pendientes) {
        const QString nuevo = generarCodigoUnico(usados);
        usados.insert(nuevo);
        productos[i].codigo = nuevo;
    }

    if (!pendientes.isEmpty())
        guardarProductos();

    return !pendientes.isEmpty();
}