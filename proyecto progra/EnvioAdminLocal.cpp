#include "clases/EnvioAdminLocal.h"

#include "clases/datos.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

bool EnvioAdminLocal::enviarCuotaCompletada(const FacturaCuota& factura,
                                            QString* error)
{
    if (error)
        error->clear();

    const QString ruta = rutaArchivo(QStringLiteral("pendiente_admin.json"));

    QJsonArray facturas;
    QFile archivo(ruta);
    if (archivo.exists()) {
        if (!archivo.open(QIODevice::ReadOnly)) {
            if (error)
                *error = QStringLiteral("No se pudo leer pendiente_admin.json: %1")
                             .arg(archivo.errorString());
            return false;
        }
        const QByteArray contenido = archivo.readAll();
        archivo.close();

        QJsonParseError parseError;
        const QJsonDocument documento = QJsonDocument::fromJson(contenido, &parseError);
        if (parseError.error != QJsonParseError::NoError || !documento.isArray()) {
            if (error)
                *error = QStringLiteral("pendiente_admin.json está dañado; no se puede encolar la factura.");
            return false;
        }
        facturas = documento.array();

        // No encolar dos veces la misma factura.
        for (const QJsonValue& valor : facturas) {
            if (valor.toObject().value(QStringLiteral("numero_factura")).toString()
                == factura.numeroTexto()) {
                return true;
            }
        }
    }

    facturas.append(factura.toJson());

    QSaveFile salida(ruta);
    if (!salida.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo abrir pendiente_admin.json: %1")
                         .arg(salida.errorString());
        return false;
    }
    if (salida.write(QJsonDocument(facturas).toJson(QJsonDocument::Indented)) == -1) {
        if (error)
            *error = QStringLiteral("No se pudo escribir pendiente_admin.json: %1")
                         .arg(salida.errorString());
        salida.cancelWriting();
        return false;
    }
    if (!salida.commit()) {
        if (error)
            *error = QStringLiteral("No se pudo guardar pendiente_admin.json: %1")
                         .arg(salida.errorString());
        return false;
    }

    return true;
}
