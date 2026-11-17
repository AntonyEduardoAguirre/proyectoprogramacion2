#pragma once

#include <QWidget>

// Ventana del pago a cuotas: se elige el plazo (3, 6, 12 o 24 meses),
// se muestra el precio total sin aumento y con el aumento (0.5% por
// cada mes de plazo), el pago mensual (10% del total con aumento) y
// se pide el nombre del cliente. Al presionar "Completar cuota" se
// muestra el resumen y se guarda el registro en datos/cuotas.json.
class PagoCuotas : public QWidget
{
    Q_OBJECT

public:
    explicit PagoCuotas(double totalSinAumento, int mesesIniciales = 3,
                        QWidget* parent = nullptr);

signals:
    // se emite al completar la cuota, con toda la información del registro
    void pagoCompletado(int meses, const QString& cliente, const QString& fecha,
                        double totalSinAumento, double totalConAumento,
                        double pagoMensual, int mesesRestantes);
};
