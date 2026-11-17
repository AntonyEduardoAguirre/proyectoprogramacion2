#include "clases/ServicioCierre.h"

#include <QDateTime>
#include <QStringList>

void ServicioCierre::agregarFuente(std::unique_ptr<FuenteVentasDia> fuente)
{
    if (fuente)
        fuentes_.push_back(std::move(fuente));
}

bool ServicioCierre::generar(const QDate& fecha, const QString& caja,
                             CierreDia* cierre, QString* error) const
{
    if (error)
        error->clear();
    if (cierre == nullptr)
        return false;

    *cierre = CierreDia();
    cierre->fecha = fecha;
    cierre->horaGeneracion = QDateTime::currentDateTime();

    const QStringList tipos = { QStringLiteral("Efectivo"),
                                QStringLiteral("Tarjeta"),
                                QStringLiteral("Cuotas") };
    for (const QString& tipo : tipos) {
        ResumenTipo resumen;
        resumen.tipo = tipo;
        cierre->resumen.append(resumen);
    }

    for (const auto& fuente : fuentes_) {
        QList<VentaDia> ventas;
        QString errorFuente;
        if (!fuente->ventasDelDia(fecha, caja, &ventas, &errorFuente)) {
            if (error) {
                *error = QStringLiteral("Error en la fuente %1: %2")
                             .arg(fuente->tipo(), errorFuente);
            }
            return false;
        }

        for (const VentaDia& venta : ventas) {
            cierre->detalle.append(venta);
            cierre->totalGeneral += venta.monto;

            bool encontrado = false;
            for (ResumenTipo& resumen : cierre->resumen) {
                if (resumen.tipo == venta.tipo) {
                    resumen.cantidad += 1;
                    resumen.total += venta.monto;
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado) {
                ResumenTipo resumen;
                resumen.tipo = venta.tipo;
                resumen.cantidad = 1;
                resumen.total = venta.monto;
                cierre->resumen.append(resumen);
            }
        }
    }

    return true;
}
