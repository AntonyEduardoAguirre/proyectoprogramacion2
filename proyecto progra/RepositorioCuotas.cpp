#include "clases/RepositorioCuotas.h"

#include "clases/datos.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

#include <algorithm>
#include <cmath>

namespace {

// Los registros viejos no guardaban la lista de pagos. El primer pago
// se hizo el día de la compra, así que se reconstruye si hace falta.
void asegurarPrimerPago(Cuota* cuota)
{
    if (cuota == nullptr || cuota->pagoMensual <= 0.0 || cuota->fecha.isEmpty())
        return;

    const QString diaCompra = cuota->fecha.left(10);
    for (const PagoCuota& pago : cuota->pagos) {
        if (pago.fecha.left(10) == diaCompra
            && std::abs(pago.monto - cuota->pagoMensual) < 0.01) {
            return;
        }
    }

    PagoCuota primero;
    primero.fecha = cuota->fecha;
    primero.monto = cuota->pagoMensual;
    cuota->pagos.prepend(primero);
}

QJsonObject cuotaAJson(Cuota cuota)
{
    if (cuota.saldoPendiente() <= 0.004)
        cuota.completada = true;

    QJsonObject obj;
    obj["cliente"] = cuota.cliente;
    obj["caja"] = cuota.caja;
    obj["fecha"] = cuota.fecha;
    obj["meses"] = cuota.meses;
    obj["meses_restantes"] = cuota.mesesRestantes;
    obj["pago_mensual"] = cuota.pagoMensual;
    obj["porcentaje_aumento"] = cuota.porcentajeAumento;
    obj["total_con_aumento"] = cuota.totalConAumento;
    obj["total_sin_aumento"] = cuota.totalSinAumento;
    obj["completada"] = cuota.completada;

    QJsonArray pagos;
    for (const PagoCuota& pago : cuota.pagos) {
        QJsonObject pagoJson;
        pagoJson["fecha"] = pago.fecha;
        pagoJson["monto"] = pago.monto;
        pagoJson["enviado_admin"] = pago.enviadoAdmin;
        pagos.append(pagoJson);
    }
    obj["pagos"] = pagos;
    if (cuota.completada && cuota.factura.numero > 0)
        obj["factura"] = cuota.factura.toJson();
    return obj;
}

} // namespace

double Cuota::totalPagado() const
{
    double total = 0.0;
    for (const PagoCuota& pago : pagos)
        total += pago.monto;
    return total;
}

double Cuota::saldoPendiente() const
{
    const double saldo = totalConAumento - totalPagado();
    return saldo > 0.0 ? saldo : 0.0;
}

int Cuota::cuotasPagadas() const
{
    return pagos.size();
}

int Cuota::cuotasPendientes() const
{
    if (saldoPendiente() <= 0.004 || pagoMensual <= 0.0)
        return 0;
    return static_cast<int>(std::ceil(saldoPendiente() / pagoMensual));
}

bool Cuota::activa() const
{
    return !completada && saldoPendiente() > 0.004;
}

bool Cuota::pagoHechoEn(const QDate& dia) const
{
    for (const PagoCuota& pago : pagos) {
        const QDate fechaPago = QDate::fromString(pago.fecha.left(10),
                                                  QStringLiteral("dd/MM/yyyy"));
        if (fechaPago.isValid() && fechaPago.year() == dia.year()
            && fechaPago.month() == dia.month()) {
            return true;
        }
    }
    return false;
}

QString RepositorioCuotas::rutaArchivoCuotas()
{
    return rutaArchivo(QStringLiteral("cuotas.json"));
}

bool RepositorioCuotas::cargar(QList<Cuota>* cuotas, QString* error) const
{
    if (error)
        error->clear();
    if (cuotas == nullptr)
        return false;

    cuotas->clear();
    QFile archivo(rutaArchivoCuotas());
    if (!archivo.exists())
        return true;

    if (!archivo.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo leer cuotas.json: %1").arg(archivo.errorString());
        return false;
    }

    const QByteArray contenido = archivo.readAll();
    archivo.close();

    QJsonParseError parseError;
    const QJsonDocument documento = QJsonDocument::fromJson(contenido, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error)
            *error = QStringLiteral("cuotas.json está dañado: %1").arg(parseError.errorString());
        return false;
    }
    if (!documento.isArray()) {
        if (error)
            *error = QStringLiteral("cuotas.json no contiene una lista válida.");
        return false;
    }

    for (const QJsonValue& valor : documento.array()) {
        if (!valor.isObject()) {
            if (error)
                *error = QStringLiteral("cuotas.json contiene un registro que no es válido.");
            return false;
        }

        const QJsonObject obj = valor.toObject();
        Cuota cuota;
        cuota.cliente = obj["cliente"].toString();
        cuota.caja = obj["caja"].toString();
        cuota.fecha = obj["fecha"].toString();
        cuota.meses = obj["meses"].toInt(0);
        cuota.mesesRestantes = obj["meses_restantes"].toInt(0);
        cuota.pagoMensual = obj["pago_mensual"].toDouble(0.0);
        cuota.porcentajeAumento = obj["porcentaje_aumento"].toDouble(0.0);
        cuota.totalConAumento = obj["total_con_aumento"].toDouble(0.0);
        cuota.totalSinAumento = obj["total_sin_aumento"].toDouble(0.0);
        cuota.completada = obj["completada"].toBool(false);

        for (const QJsonValue& pagoValor : obj["pagos"].toArray()) {
            const QJsonObject pagoObj = pagoValor.toObject();
            PagoCuota pago;
            pago.fecha = pagoObj["fecha"].toString();
            pago.monto = pagoObj["monto"].toDouble(0.0);
            pago.enviadoAdmin = pagoObj["enviado_admin"].toBool(false);
            cuota.pagos.append(pago);
        }

        if (obj.contains(QStringLiteral("factura")))
            cuota.factura.fromJson(obj["factura"].toObject());

        asegurarPrimerPago(&cuota);
        if (cuota.saldoPendiente() <= 0.004)
            cuota.completada = true;
        cuotas->append(cuota);
    }

    return true;
}

bool RepositorioCuotas::guardar(const QList<Cuota>& cuotas, QString* error) const
{
    if (error)
        error->clear();

    QJsonArray arreglo;
    for (const Cuota& cuota : cuotas)
        arreglo.append(cuotaAJson(cuota));

    QSaveFile archivo(rutaArchivoCuotas());
    if (!archivo.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo abrir cuotas.json para escribir: %1")
                         .arg(archivo.errorString());
        return false;
    }

    if (archivo.write(QJsonDocument(arreglo).toJson(QJsonDocument::Indented)) == -1) {
        if (error)
            *error = QStringLiteral("No se pudo escribir cuotas.json: %1")
                         .arg(archivo.errorString());
        archivo.cancelWriting();
        return false;
    }

    if (!archivo.commit()) {
        if (error)
            *error = QStringLiteral("No se pudo guardar cuotas.json: %1")
                         .arg(archivo.errorString());
        return false;
    }

    return true;
}

bool RepositorioCuotas::agregar(Cuota cuota, QString* error) const
{
    if (error)
        error->clear();

    QList<Cuota> cuotas;
    if (!cargar(&cuotas, error))
        return false;

    asegurarPrimerPago(&cuota);
    if (cuota.saldoPendiente() <= 0.004)
        cuota.completada = true;

    cuotas.append(cuota);
    return guardar(cuotas, error);
}

bool RepositorioCuotas::registrarPago(int indice, const QString& fecha, double monto,
                                      QString* error) const
{
    if (error)
        error->clear();

    QList<Cuota> cuotas;
    if (!cargar(&cuotas, error))
        return false;

    if (indice < 0 || indice >= cuotas.size()) {
        if (error)
            *error = QStringLiteral("La cuota seleccionada ya no existe.");
        return false;
    }

    Cuota& cuota = cuotas[indice];
    const double saldo = cuota.saldoPendiente();
    if (saldo <= 0.004) {
        if (error)
            *error = QStringLiteral("Esta cuota ya está completada.");
        return false;
    }

    // Se pueden registrar varios pagos el mismo día: cada clic cuenta como un pago.
    const double cobro = std::min(monto, saldo);
    if (cobro <= 0.0) {
        if (error)
            *error = QStringLiteral("El monto del pago no es válido.");
        return false;
    }

    PagoCuota pago;
    pago.fecha = fecha;
    pago.monto = cobro;
    cuota.pagos.append(pago);
    cuota.mesesRestantes = std::max(0, cuota.mesesRestantes - 1);

    if (cuota.saldoPendiente() <= 0.004) {
        cuota.completada = true;
        cuota.mesesRestantes = 0;
    }

    return guardar(cuotas, error);
}

int RepositorioCuotas::siguienteNumeroFactura(QString* error) const
{
    QList<Cuota> cuotas;
    if (!cargar(&cuotas, error))
        return 0;

    int maximo = 0;
    for (const Cuota& cuota : cuotas)
        maximo = std::max(maximo, cuota.factura.numero);
    return maximo + 1;
}

bool RepositorioCuotas::completar(int indice, const FacturaCuota& factura,
                                  QString* error) const
{
    if (error)
        error->clear();

    QList<Cuota> cuotas;
    if (!cargar(&cuotas, error))
        return false;

    if (indice < 0 || indice >= cuotas.size()) {
        if (error)
            *error = QStringLiteral("La cuota seleccionada ya no existe.");
        return false;
    }

    Cuota& cuota = cuotas[indice];
    cuota.completada = true;
    cuota.mesesRestantes = 0;
    cuota.factura = factura;
    cuota.factura.enviadaAdmin = false;   // pendiente de confirmación del admin

    return guardar(cuotas, error);        // QSaveFile, antes de intentar el envío
}

QString RepositorioCuotas::idPago(const QString& cliente, const QString& fecha,
                                  double monto)
{
    return QStringLiteral("%1|%2|%3")
        .arg(cliente, fecha, QString::number(monto, 'f', 2));
}

bool RepositorioCuotas::marcarEnviados(const QList<MarcaEnvioCuotas>& marcas,
                                       const QString& fechaEnvio, QString* error) const
{
    if (error)
        error->clear();

    QList<Cuota> cuotas;
    if (!cargar(&cuotas, error))
        return false;

    for (const MarcaEnvioCuotas& marca : marcas) {
        if (marca.indiceCuota < 0 || marca.indiceCuota >= cuotas.size())
            continue;
        Cuota& cuota = cuotas[marca.indiceCuota];

        for (int indicePago : marca.pagos) {
            if (indicePago >= 0 && indicePago < cuota.pagos.size())
                cuota.pagos[indicePago].enviadoAdmin = true;
        }

        if (marca.factura && cuota.factura.numero > 0) {
            cuota.factura.enviadaAdmin = true;
            cuota.factura.fechaEnvioAdmin = fechaEnvio;
            for (PagoCuota& pago : cuota.factura.pagos)
                pago.enviadoAdmin = true;
        }
    }

    return guardar(cuotas, error);   // QSaveFile: todo o nada
}
