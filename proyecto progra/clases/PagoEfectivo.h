#pragma once

#include <QWidget>
#include "compra.h"

// Ventana de cobro en efectivo tipo caja registradora: muestra el
// ticket con los artículos seleccionados y su total, se ingresa el
// dinero que entrega el cliente y se calcula el vuelto en vivo.
class PagoEfectivo : public QWidget
{
    Q_OBJECT

public:
    explicit PagoEfectivo(double total,
                          const QList<ItemCompra>& items, QWidget* parent = nullptr);

signals:
    // se emite al confirmar el pago, con el efectivo recibido y el vuelto
    void pagoConfirmado(double efectivoRecibido, double vuelto);
};
