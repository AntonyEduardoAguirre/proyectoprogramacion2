#pragma once

#include "FuenteVentasDia.h"

// Fuente que lee los pagos de cuotas registrados en datos/cuotas.json.
class FuenteCuotas : public FuenteVentasDia
{
public:
    QString tipo() const override;
    bool ventasDelDia(const QDate& fecha, const QString& caja,
                      QList<VentaDia>* ventas, QString* error) const override;
};
