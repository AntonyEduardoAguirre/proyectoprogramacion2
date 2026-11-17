#pragma once

#include "CierreDia.h"
#include "FuenteVentasDia.h"

#include <memory>
#include <vector>

// Genera el cierre del día a partir de una o varias fuentes. Cada fuente
// se consulta por separado; si una falla, no se devuelve un cierre parcial.
class ServicioCierre
{
public:
    void agregarFuente(std::unique_ptr<FuenteVentasDia> fuente);
    bool generar(const QDate& fecha, const QString& caja,
                 CierreDia* cierre, QString* error) const;

private:
    std::vector<std::unique_ptr<FuenteVentasDia>> fuentes_;
};
