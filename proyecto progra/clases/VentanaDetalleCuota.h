#pragma once

#include <QList>
#include <QWidget>

#include "RepositorioCuotas.h"
#include "compra.h"
#include "ui_VentanaDetalleCuota.h"

class QKeyEvent;

// Detalle de una compra a cuotas con el mismo diseño del recibo de cuota
// completada. Permite registrar el pago del mes actual.
class VentanaDetalleCuota : public QWidget
{
    Q_OBJECT

public:
    VentanaDetalleCuota(const Cuota& cuota, int indice,
                        const QList<ItemCompra>& productos,
                        QWidget* parent = nullptr, bool soloLectura = false);
    ~VentanaDetalleCuota() override;

protected:
    void keyPressEvent(QKeyEvent* evento) override;

private:
    void mostrarCuota();
    void mostrarEstado(const QString& texto, bool error);
    void completarPago();
    double pagoDeEsteMes() const;

    Ui::VentanaDetalleCuotaClass ui;
    Cuota cuota_;
    int indice_ = -1;
    QList<ItemCompra> productos_;
    bool soloLectura_ = false;   // factura completada: sin sección de pago
};
