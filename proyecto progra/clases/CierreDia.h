#pragma once

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QString>

// Una venta o pago individual que entra en el cierre del día.
struct VentaDia
{
    QString tipo;     // "Efectivo", "Tarjeta" o "Cuotas"
    QString caja;     // usuario que tiene la sesión iniciada
    QString fecha;
    QString cliente;
    double monto = 0.0;
    QString ultimos4;    // se deja vacío si no se conoce
    QString referencia;  // id estable: número de factura o id del pago
};

// Cantidad y total acumulados por tipo de pago.
struct ResumenTipo
{
    QString tipo;
    int cantidad = 0;
    double total = 0.0;
};

// Resultado completo de un cierre diario.
struct CierreDia
{
    QDate fecha;
    QDateTime horaGeneracion;
    QList<ResumenTipo> resumen;
    double totalGeneral = 0.0;
    QList<VentaDia> detalle;
};
