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

        // Las cuotas completadas con factura pendiente se envían como
        // "Cuota completada", también aunque sus pagos ya estén marcados.
        if (cuota.factura.numero > 0 && !cuota.factura.enviadaAdmin) {
            const QDateTime cuando = QDateTime::fromString(
                cuota.factura.fechaFinalizacion, QStringLiteral("dd/MM/yyyy hh:mm:ss"));
            if (cuando.isValid() && cuando.date() == fecha) {
                VentaDia venta;
                venta.tipo = QStringLiteral("Cuota completada");
                venta.caja = cuota.factura.caja.isEmpty() ? caja : cuota.factura.caja;
                venta.fecha = cuota.factura.fechaFinalizacion;
                venta.cliente = cuota.cliente;
                venta.monto = cuota.factura.totalPagado;
                venta.referencia = cuota.factura.numeroTexto();
                ventas->append(venta);
            }
        }
    }

    return true;
}
