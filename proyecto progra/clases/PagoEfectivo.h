#pragma once

#include <QWidget>
#include "compra.h"

// Ventana de cobro en efectivo tipo caja registradora: muestra el
// ticket con los artículos seleccionados y su total, se ingresa el
// dinero que entrega el cliente y se calcula el vuelto en vivo.
// Si el efectivo no alcanza pero el saldo de la cuenta del cliente
// cubre el total, el pago también se permite (se descuenta del saldo).
class PagoEfectivo : public QWidget
{
    Q_OBJECT

public:
    explicit PagoEfectivo(double total, double saldoCuenta,
                          const QList<ItemCompra>& items, QWidget* parent = nullptr);

signals:
    // se emite al confirmar el pago, con el efectivo recibido, el vuelto
    // y si el pago se hace con el saldo de la cuenta en lugar de efectivo
    void pagoConfirmado(double efectivoRecibido, double vuelto, bool pagadoConSaldo);
};
