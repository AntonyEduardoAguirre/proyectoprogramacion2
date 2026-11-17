#include "clases/FuenteCuotas.h"

#include "clases/RepositorioCuotas.h"

#include <QDateTime>

QString FuenteCuotas::tipo() const
{
    return QStringLiteral("Cuotas");
}

bool FuenteCuotas::ventasDelDia(const QDate& fecha, const QString& caja,
                                QList<VentaDia>* ventas, QString* error) const
{
    if (error)
        error->clear();
    if (ventas == nullptr)
        return false;

    ventas->clear();
    QList<Cuota> cuotas;
    RepositorioCuotas repositorio;
    if (!repositorio.cargar(&cuotas, error))
        return false;

    for (const Cuota& cuota : cuotas) {

        for (const PagoCuota& pago : cuota.pagos) {
            // Los pagos ya enviados en un Cierre anterior nunca se repiten.
            if (pago.enviadoAdmin)
                continue;

            const QDateTime cuando = QDateTime::fromString(
                pago.fecha, QStringLiteral("dd/MM/yyyy hh:mm:ss"));
            if (!cuando.isValid()) {
                if (error)
                    *error = QStringLiteral("La fecha '%1' de un pago en cuotas.json no es válida.")
                                 .arg(pago.fecha);
                return false;
            }
            if (cuando.date() != fecha)
                continue;

            VentaDia venta;
            venta.tipo = tipo();
            venta.caja = caja;
            venta.fecha = pago.fecha;
            venta.cliente = cuota.cliente;
            venta.monto = pago.monto;
            venta.referencia = RepositorioCuotas::idPago(
                cuota.cliente, pago.fecha, pago.monto);
            ventas->append(venta);
        }

        // Las cuotas completadas NO viajan en el Cierre: su factura se sube al
        // instante en que se completan (enviada_admin = true). Aquí solo se
        // envían los pagos de los planes que siguen activos.
    }

    return true;
}
