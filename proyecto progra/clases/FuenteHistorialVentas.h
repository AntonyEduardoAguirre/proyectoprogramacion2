#pragma once

#include "FuenteVentasDia.h"

// Fuente que lee las ventas de datos/historial.json y filtra por método.
// Se usa dos veces: una para "Efectivo" y otra para "Tarjeta".
class FuenteHistorialVentas : public FuenteVentasDia
{
public:
    FuenteHistorialVentas(QString tipo, QString metodo);

    QString tipo() const override;
    bool ventasDelDia(const QDate& fecha, const QString& caja,
                      QList<VentaDia>* ventas, QString* error) const override;

private:
    QString tipo_;
    QString metodo_;
};
