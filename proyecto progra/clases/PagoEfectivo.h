#pragma once

#include <QWidget>

// Ventana para pagar el carrito en efectivo: se ingresa el dinero
// que entrega el cliente, se calcula el vuelto en vivo y se confirma
// el pago solo si el efectivo alcanza.
class PagoEfectivo : public QWidget
{
    Q_OBJECT

public:
    explicit PagoEfectivo(double total, QWidget* parent = nullptr);

signals:
    // se emite al confirmar el pago, con el efectivo recibido y el vuelto
    void pagoConfirmado(double efectivoRecibido, double vuelto);
};
