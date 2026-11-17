#include "clases/RepositorioBitacora.h"

#include "clases/datos.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStringList>

namespace {

QString formatoFecha()
{
    return QStringLiteral("dd/MM/yyyy");
}

QString formatoHora()
{
    return QStringLiteral("dd/MM/yyyy hh:mm:ss");
}

QJsonObject cierreAJson(const CierreDia& cierre)
{
    QJsonObject obj;
    obj["fecha"] = cierre.fecha.toString(formatoFecha());
    obj["hora_generacion"] = cierre.horaGeneracion.toString(formatoHora());
    obj["total_general"] = cierre.totalGeneral;

    QJsonArray resumen;
    for (const ResumenTipo& tipo : cierre.resumen) {
        QJsonObject fila;
        fila["tipo"] = tipo.tipo;
        fila["cantidad"] = tipo.cantidad;
        fila["total"] = tipo.total;
        resumen.append(fila);
    }
    obj["resumen"] = resumen;

    QJsonArray detalle;
    for (const VentaDia& venta : cierre.detalle) {
        QJsonObject fila;
        fila["tipo"] = venta.tipo;
        fila["caja"] = venta.caja;
        fila["fecha"] = venta.fecha;
        fila["cliente"] = venta.cliente;
        fila["monto"] = venta.monto;
        fila["ultimos4"] = venta.ultimos4;
        fila["referencia"] = venta.referencia;
        detalle.append(fila);
    }
    obj["detalle"] = detalle;

    return obj;
}

bool cierreDesdeJson(const QJsonObject& obj, CierreDia* cierre, QString* error)
{
    const QDate fecha = QDate::fromString(obj["fecha"].toString(), formatoFecha());
    if (!fecha.isValid()) {
        if (error)
            *error = QStringLiteral("bitacora.json tiene un cierre con fecha inválida.");
        return false;
    }

    const QDateTime hora = QDateTime::fromString(obj["hora_generacion"].toString(),
                                                formatoHora());
    if (!hora.isValid()) {
        if (error)
            *error = QStringLiteral("bitacora.json tiene un cierre con hora inválida.");
        return false;
    }

    CierreDia leido;
    leido.fecha = fecha;
    leido.horaGeneracion = hora;
    leido.totalGeneral = obj["total_general"].toDouble(0.0);

    for (const QJsonValue& valor : obj["resumen"].toArray()) {
        if (!valor.isObject()) {
            if (error)
                *error = QStringLiteral("bitacora.json tiene un resumen inválido.");
            return false;
        }

        const QJsonObject fila = valor.toObject();
        ResumenTipo tipo;
        tipo.tipo = fila["tipo"].toString();
        tipo.cantidad = fila["cantidad"].toInt(0);
        tipo.total = fila["total"].toDouble(0.0);
        leido.resumen.append(tipo);
    }

    for (const QJsonValue& valor : obj["detalle"].toArray()) {
        if (!valor.isObject()) {
            if (error)
                *error = QStringLiteral("bitacora.json tiene un detalle inválido.");
            return false;
        }

        const QJsonObject fila = valor.toObject();
        VentaDia venta;
        venta.tipo = fila["tipo"].toString();
        venta.caja = fila["caja"].toString();
        venta.fecha = fila["fecha"].toString();
        venta.cliente = fila["cliente"].toString();
        venta.monto = fila["monto"].toDouble(0.0);
        venta.ultimos4 = fila["ultimos4"].toString();
        venta.referencia = fila["referencia"].toString();
        leido.detalle.append(venta);
    }

    *cierre = leido;
    return true;
}

// Dos ventas se consideran la misma si coinciden todos sus datos.
bool mismaVenta(const VentaDia& a, const VentaDia& b)
{
    return a.tipo == b.tipo && a.caja == b.caja && a.fecha == b.fecha
        && a.cliente == b.cliente && qAbs(a.monto - b.monto) < 0.005
        && a.ultimos4 == b.ultimos4 && a.referencia == b.referencia;
}

// Recalcula el resumen y el total general a partir del detalle ya fusionado.
void recalcularCierre(CierreDia* cierre)
{
    cierre->resumen.clear();
    cierre->totalGeneral = 0.0;

    const QStringList tipos = { QStringLiteral("Efectivo"),
                                QStringLiteral("Tarjeta"),
                                QStringLiteral("Cuotas"),
                                QStringLiteral("Cuota completada") };
    for (const QString& tipo : tipos) {
        ResumenTipo fila;
        fila.tipo = tipo;
        cierre->resumen.append(fila);
    }

    for (const VentaDia& venta : cierre->detalle) {
        cierre->totalGeneral += venta.monto;
        bool encontrado = false;
        for (ResumenTipo& fila : cierre->resumen) {
            if (fila.tipo == venta.tipo) {
                fila.cantidad += 1;
                fila.total += venta.monto;
                encontrado = true;
                break;
            }
        }
        if (!encontrado) {
            ResumenTipo fila;
            fila.tipo = venta.tipo;
            fila.cantidad = 1;
            fila.total = venta.monto;
            cierre->resumen.append(fila);
        }
    }
}

} // namespace

QString RepositorioBitacora::rutaArchivoBitacora()
{
    return rutaArchivo(QStringLiteral("bitacora.json"));
}

bool RepositorioBitacora::todos(QList<CierreDia>* cierres, QString* error) const
{
    if (error)
        error->clear();
    if (cierres == nullptr)
        return false;

    cierres->clear();
    QFile archivo(rutaArchivoBitacora());
    if (!archivo.exists())
        return true;

    if (!archivo.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo leer bitacora.json: %1").arg(archivo.errorString());
        return false;
    }

    const QByteArray contenido = archivo.readAll();
    archivo.close();

    QJsonParseError parseError;
    const QJsonDocument documento = QJsonDocument::fromJson(contenido, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error)
            *error = QStringLiteral("bitacora.json está dañado: %1").arg(parseError.errorString());
        return false;
    }
    if (!documento.isArray()) {
        if (error)
            *error = QStringLiteral("bitacora.json no contiene una lista válida.");
        return false;
    }

    for (const QJsonValue& valor : documento.array()) {
        if (!valor.isObject()) {
            if (error)
                *error = QStringLiteral("bitacora.json contiene un cierre que no es válido.");
            return false;
        }

        CierreDia cierre;
        if (!cierreDesdeJson(valor.toObject(), &cierre, error))
            return false;
        cierres->append(cierre);
    }

    return true;
}

bool RepositorioBitacora::guardar(const CierreDia& cierre, QString* error) const
{
    if (error)
        error->clear();

    QList<CierreDia> cierres;
    if (!todos(&cierres, error))
        return false;

    bool agregado = false;
    for (CierreDia& existente : cierres) {
        if (existente.fecha != cierre.fecha)
            continue;

        // No se reemplaza: se agregan los registros nuevos y se recalculan totales.
        for (const VentaDia& venta : cierre.detalle) {
            bool duplicado = false;
            for (const VentaDia& guardada : existente.detalle) {
                if (mismaVenta(venta, guardada)) {
                    duplicado = true;
                    break;
                }
            }
            if (!duplicado)
                existente.detalle.append(venta);
        }
        existente.horaGeneracion = cierre.horaGeneracion;
        recalcularCierre(&existente);
        agregado = true;
        break;
    }
    if (!agregado)
        cierres.append(cierre);

    QJsonArray arreglo;
    for (const CierreDia& guardado : cierres)
        arreglo.append(cierreAJson(guardado));

    QSaveFile archivo(rutaArchivoBitacora());
    if (!archivo.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo abrir bitacora.json para escribir: %1")
                         .arg(archivo.errorString());
        return false;
    }

    if (archivo.write(QJsonDocument(arreglo).toJson(QJsonDocument::Indented)) == -1) {
        if (error)
            *error = QStringLiteral("No se pudo escribir bitacora.json: %1")
                         .arg(archivo.errorString());
        archivo.cancelWriting();
        return false;
    }

    if (!archivo.commit()) {
        if (error)
            *error = QStringLiteral("No se pudo guardar bitacora.json: %1")
                         .arg(archivo.errorString());
        return false;
    }

    return true;
}

bool RepositorioBitacora::existeCierre(const QDate& fecha, QString* error) const
{
    if (error)
        error->clear();

    QList<CierreDia> cierres;
    if (!todos(&cierres, error))
        return false;

    for (const CierreDia& cierre : cierres) {
        if (cierre.fecha == fecha)
            return true;
    }
    return false;
}
