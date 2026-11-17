#pragma once

#include <QWidget>
#include "ui_CuotaCompletada.h"
#include "compra.h"

// Ventana de "Cuota completada": recibo completo de la compra a cuotas
// con el mismo diseño del resto de las pantallas (tarjeta suave).
// Muestra los productos comprados y toda la información de la cuota.
class CuotaCompletada : public QWidget
{
    Q_OBJECT

public:
    CuotaCompletada(const Compra& compra, const QString& cliente, const QString& fecha,
                    double totalSinAumento, double totalConAumento,
                    double pagoMensual, int mesesRestantes,
                    QWidget* parent = nullptr);
    ~CuotaCompletada();

signals:
    // se emite al presionar "Listo"; el carrito se cierra al recibirla
    void cuotaCerrada();

private:
    Ui::CuotaCompletadaClass ui;
};
