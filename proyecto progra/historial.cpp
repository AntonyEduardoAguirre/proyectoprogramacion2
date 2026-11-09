#include "Historial.h"
#include "datos.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QColor>

historial::historial(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true); // para que el QSS pinte el fondo

    // La sombra suave no se puede definir en el .ui
    auto* sombra = new QGraphicsDropShadowEffect(ui.softCard);
    sombra->setBlurRadius(28);
    sombra->setOffset(0, 8);
    sombra->setColor(QColor(41, 42, 40, 22));
    ui.softCard->setGraphicsEffect(sombra);

    cargarHistorial();

    connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
        this->close();
        });

    actualizarHistorial();
}

historial::~historial()
{
}

void historial::actualizarHistorial()
{
    // limpia las líneas anteriores
    while (QLayoutItem* hijo = ui.layoutCompras->takeAt(0)) {
        if (QWidget* widget = hijo->widget())
            widget->deleteLater();
        delete hijo;
    }

    bool hayCompras = false;

    // se muestran de la más reciente a la más antigua
    for (int i = historialCompras.size() - 1; i >= 0; --i) {
        const Compra& compra = historialCompras[i];
        if (compra.clienteId != usuarioActual.getId())
            continue;

        hayCompras = true;

        QString texto = QString("%1  —  Total: $%2")
            .arg(compra.fecha)
            .arg(compra.total, 0, 'f', 2);

        for (const ItemCompra& item : compra.items)
            texto += QString("\n   %1 × %2  —  $%3")
            .arg(item.cantidad)
            .arg(item.nombre)
            .arg(item.precio * item.cantidad, 0, 'f', 2);

        QLabel* fila = new QLabel(texto, this);
        fila->setObjectName("lblCompra");
        ui.layoutCompras->addWidget(fila);
    }

    if (!hayCompras) {
        QLabel* vacio = new QLabel("Aún no has realizado ninguna compra.", this);
        vacio->setObjectName("lblCompra");
        ui.layoutCompras->addWidget(vacio);
    }

    ui.layoutCompras->addStretch();
}
