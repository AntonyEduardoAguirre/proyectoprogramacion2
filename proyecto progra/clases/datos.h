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
#include <QCoreApplication>
#include <QRandomGenerator>
#include "Cliente.h"
#include "Producto.h"

inline QList<Cliente> clientes;
inline QList<Producto> productos;
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


// Opcional: para guardar/cargar todo de una vez
inline void guardarTodo() { guardarClientes(); guardarProductos(); }
inline void cargarTodo() { cargarClientes();  cargarProductos(); }

// ---------------------------------------------------------------
// Códigos de producto (6 dígitos, únicos)
// ---------------------------------------------------------------

// Devuelve un código de 6 dígitos que no esté en "usados".
inline QString generarCodigoUnico(const QSet<QString>& usados)
{
    QString codigo;
    do {
        codigo = QString::number(QRandomGenerator::global()->bounded(100000, 1000000));
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