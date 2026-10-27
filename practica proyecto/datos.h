#pragma once
#include <QList>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include "Cliente.h"
#include "Producto.h"

inline QList<Cliente> clientes;
inline QList<Producto> productos;

inline QString rutaArchivo(const QString& nombre)
{
    QString carpeta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(carpeta);   // crea la carpeta si no existe
    return carpeta + "/" + nombre;
}

inline void guardarClientes()
{
    QJsonArray arreglo;
    for (const Cliente& c : clientes)
        arreglo.append(c.toJson());

    QFile archivo(rutaArchivo("clientes.json"));
    if (archivo.open(QIODevice::WriteOnly)) {
        archivo.write(QJsonDocument(arreglo).toJson());
        archivo.close();
    }
}

inline void cargarClientes()
{
    clientes.clear();
    QFile archivo(rutaArchivo("clientes.json"));
    if (!archivo.open(QIODevice::ReadOnly))
        return;   // aún no existe el archivo: primera ejecución

    QJsonDocument doc = QJsonDocument::fromJson(archivo.readAll());
    archivo.close();

    for (const QJsonValue& valor : doc.array()) {
        Cliente c;
        c.fromJson(valor.toObject());
        clientes.append(c);
    }
}