#pragma once
#include <QList>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include "Cliente.h"
#include "Producto.h"
#include "compra.h"

inline QList<Cliente> clientes;
inline QList<Producto> productos;
inline QList<Compra> historialCompras;   // compras de todos los clientes
inline QList<ItemCompra> carrito;        // carrito de compras del usuario actual
inline Cliente usuarioActual;  // variable global para almacenar el usuario actual

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

// cierra la compra: descuenta saldo y stock, guarda el historial
// si meses > 0, la compra se hace a plazos (12 o 24 meses):
//   solo se cobra la primera cuota (total / meses, sin interés)
inline bool pagarCarrito(int meses = 0)
{
    if (carrito.isEmpty())
        return false;

    const double total = carritoTotal();

    // el cobro es el total de contado, o solo la primera cuota
    double cobro = total;
    if (meses > 0)
        cobro = total / meses;

    if (usuarioActual.saldo < cobro)
        return false;   // saldo insuficiente

    // descuenta el saldo del usuario actual (y de la lista de clientes)
    usuarioActual.saldo -= static_cast<float>(cobro);
    for (Cliente& cliente : clientes) {
        if (cliente.getId() == usuarioActual.getId()) {
            cliente.setSaldo(usuarioActual.saldo);
            break;
        }
    }
    guardarClientes();

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
    compra.items = carrito;
    historialCompras.append(compra);
    guardarHistorial();

    vaciarCarrito();
    return true;
}