#pragma once

#include "CierreDia.h"

#include <QDate>
#include <QList>
#include <QString>

// Interfaz común de las fuentes del cierre. Para agregar otra fuente
// (por ejemplo, transferencias) basta con crear otra clase que herede
// de esta y registrarla en ServicioCierre, sin tocar el servicio.
class FuenteVentasDia
{
public:
    virtual ~FuenteVentasDia() = default;

    virtual QString tipo() const = 0;
    virtual bool ventasDelDia(const QDate& fecha, const QString& caja,
                              QList<VentaDia>* ventas, QString* error) const = 0;
};
