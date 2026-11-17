#pragma once

#include "FacturaCuota.h"

class QString;

// Contrato para entregar la factura de una cuota completada al admin.
// Hoy se usa una implementación local; cuando exista el menú admin se
// reemplaza por el envío real sin tocar quienes llaman a este método.
class EnvioAdmin
{
public:
    virtual ~EnvioAdmin() = default;

    virtual bool enviarCuotaCompletada(const FacturaCuota& factura,
                                       QString* error = nullptr) = 0;
};
