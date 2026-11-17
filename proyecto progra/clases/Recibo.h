#pragma once

#include <QWidget>
#include "compra.h"

// Ventana que muestra el recibo de una compra terminada:
// fecha, cliente, productos, total y detalles de la forma de pago
// (vuelto del efectivo o cuotas de la tarjeta).
class Recibo : public QWidget
{
    Q_OBJECT

public:
    explicit Recibo(const Compra& compra, double efectivoRecibido = 0.0,
                    double vuelto = 0.0, QWidget* parent = nullptr);

signals:
    // se emite al cerrar el recibo con el botón principal
    void reciboCerrado();
};
