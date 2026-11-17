#pragma once

#include "compra.h"

#include <QJsonObject>
#include <QList>
#include <QString>

// Un pago mensual registrado de una compra a cuotas.
struct PagoCuota
{
    QString fecha;               // "dd/MM/yyyy hh:mm:ss"
    double monto = 0.0;
    bool enviadoAdmin = false;   // true solo cuando el Cierre lo envió
};

// Factura que se genera cuando una cuota se completa. Se guarda dentro del
// registro de la cuota en cuotas.json y se envía a la bandeja de salida
// datos/pendiente_admin.json. No sustituye a ningún campo existente.
struct FacturaCuota
{
    int numero = 0;                    // consecutivo; se muestra como F-000001
    QString fechaFinalizacion;         // "dd/MM/yyyy hh:mm:ss"
    QString cliente;
    QString caja;                      // usuario con la sesión iniciada
    QList<ItemCompra> productos;       // productos de la compra original
    QString fechaCompra;               // fecha en que se realizó la cuota
    double totalConAumento = 0.0;
    double totalPagado = 0.0;
    QList<PagoCuota> pagos;            // lista de pagos realizados
    bool enviadaAdmin = false;         // true solo cuando el Cierre la envió
    QString fechaEnvioAdmin;           // fecha del Cierre en que se envió

    QString numeroTexto() const
    {
        return QStringLiteral("F-%1").arg(numero, 6, 10, QChar('0'));
    }

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};
