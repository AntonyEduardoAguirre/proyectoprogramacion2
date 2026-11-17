#pragma once

#include "EnvioAdmin.h"

// Implementación temporal: escribe las facturas en datos/pendiente_admin.json
// como un arreglo JSON. Una factura nunca se encola dos veces: el número de
// factura se usa como identificador.
class EnvioAdminLocal : public EnvioAdmin
{
public:
    bool enviarCuotaCompletada(const FacturaCuota& factura,
                               QString* error = nullptr) override;
};
