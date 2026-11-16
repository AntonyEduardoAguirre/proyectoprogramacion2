#pragma once

#include <QWidget>

// Ventana de pago con tarjeta: se digita el número de tarjeta (16
// dígitos) y al confirmar el pago se procesa de inmediato; la compra
// se registra y sale el recibo.
class PagoTarjeta : public QWidget
{
    Q_OBJECT

public:
    // meses = 0 pago de contado, o 12/24 si la compra es a plazos
    explicit PagoTarjeta(double total, int meses, QWidget* parent = nullptr);

signals:
    // se emite cuando el pago con tarjeta fue aceptado
    void pagoAceptado();
};
