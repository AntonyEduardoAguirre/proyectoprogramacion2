#pragma once

#include <QList>
#include <QWidget>

#include "RepositorioCuotas.h"
#include "ui_VentanaCuotas.h"

class QKeyEvent;
class QListWidgetItem;

// Lista de compras a cuotas activas. Solo muestra las que todavía tienen
// saldo pendiente y permite abrir el detalle de cada una.
class VentanaCuotas : public QWidget
{
    Q_OBJECT

public:
    explicit VentanaCuotas(QWidget* parent = nullptr);
    ~VentanaCuotas() override;

protected:
    void keyPressEvent(QKeyEvent* evento) override;

private:
    void cargarCuotas();
    void filtrar();
    void abrirDetalle(int indice, bool soloLectura = false);
    void mostrarEstado(const QString& texto, bool error);

    Ui::VentanaCuotasClass ui;
    QList<Cuota> cuotas_;
    QString errorCarga_;
    bool detalleAbierto_ = false;
};
