#include "clases/FuenteHistorialVentas.h"

#include "clases/cliente.h"
#include "clases/datos.h"
#include "compra.h"

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <utility>

namespace {

QString nombreCliente(int clienteId)
{
    for (const Cliente& cliente : clientes) {
        if (cliente.getId() == clienteId)
            return cliente.nombre;
    }
    return QStringLiteral("Cliente #%1").arg(clienteId);
}

} // namespace

FuenteHistorialVentas::FuenteHistorialVentas(QString tipo, QString metodo)
    : tipo_(std::move(tipo)), metodo_(std::move(metodo))
{
}

QString FuenteHistorialVentas::tipo() const
{
    return tipo_;
}

bool FuenteHistorialVentas::ventasDelDia(const QDate& fecha, const QString& caja,
                                         QList<VentaDia>* ventas, QString* error) const
{
    if (error)
        error->clear();
    if (ventas == nullptr)
        return false;

    ventas->clear();
    QFile archivo(rutaArchivo(QStringLiteral("historial.json")));
    if (!archivo.exists())
        return true;

    if (!archivo.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo leer historial.json: %1").arg(archivo.errorString());
        return false;
    }

    const QByteArray contenido = archivo.readAll();
    archivo.close();

    QJsonParseError parseError;
    const QJsonDocument documento = QJsonDocument::fromJson(contenido, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error)
            *error = QStringLiteral("historial.json está dañado: %1").arg(parseError.errorString());
        return false;
    }
    if (!documento.isArray()) {
        if (error)
            *error = QStringLiteral("historial.json no contiene una lista válida.");
        return false;
    }

    for (const QJsonValue& valor : documento.array()) {
        if (!valor.isObject()) {
            if (error)
                *error = QStringLiteral("historial.json contiene un registro que no es válido.");
            return false;
        }

        Compra compra;
        compra.fromJson(valor.toObject());
        if (compra.metodo != metodo_)
            continue;

        const QDateTime cuando = QDateTime::fromString(compra.fecha,
                                                       QStringLiteral("dd/MM/yyyy hh:mm"));
        if (!cuando.isValid()) {
            if (error)
                *error = QStringLiteral("La fecha '%1' de historial.json no es válida.")
                             .arg(compra.fecha);
            return false;
        }
        if (cuando.date() != fecha)
            continue;

        VentaDia venta;
        venta.tipo = tipo_;
        venta.caja = caja;
        venta.fecha = compra.fecha;
        venta.cliente = nombreCliente(compra.clienteId);
        venta.monto = compra.total;
        ventas->append(venta);
    }

    return true;
}
