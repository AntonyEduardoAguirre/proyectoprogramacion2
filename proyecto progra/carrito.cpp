#include "Carrito.h"
#include "datos.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QColor>

Carrito::Carrito(QWidget* parent)
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

    // meses de cada opción del combo (contado / 12 / 24)
    ui.cmbCuotas->setItemData(0, 0);
    ui.cmbCuotas->setItemData(1, 12);
    ui.cmbCuotas->setItemData(2, 24);

    connect(ui.btnPagar, &QPushButton::clicked, this, [this]() {
        if (carrito.isEmpty()) {
            ui.lblMensaje->setText("Tu carrito está vacío.");
            return;
        }
        const int meses = ui.cmbCuotas->currentData().toInt();
        if (pagarCarrito(meses)) {
            ui.lblMensaje->setText("");
            emit carritoModificado();
            actualizarCarrito();
        } else if (meses > 0) {
            ui.lblMensaje->setText("No tienes saldo suficiente para la primera cuota.");
        } else {
            ui.lblMensaje->setText("No tienes saldo suficiente para pagar todo el carrito.");
        }
        });

    connect(ui.btnQuitar, &QPushButton::clicked, this, [this]() {
        if (carrito.isEmpty()) {
            ui.lblMensaje->setText("Tu carrito está vacío.");
            return;
        }
        quitarDelCarrito(carrito.last().idProducto);
        ui.lblMensaje->setText("");
        emit carritoModificado();
        actualizarCarrito();
        });

    connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
        emit carritoModificado();
        this->close();
        });

    // la flecha de arriba hace lo mismo que el botón de volver
    connect(ui.btnFlecha, &QPushButton::clicked, ui.btnRegresar, &QPushButton::click);

    connect(ui.cmbCuotas, &QComboBox::currentIndexChanged, this, [this](int) {
        mostrarTotal();
        });

    actualizarCarrito();
}

Carrito::~Carrito()
{
}

void Carrito::actualizarCarrito()
{
    // limpia las líneas anteriores
    while (QLayoutItem* hijo = ui.layoutItems->takeAt(0)) {
        if (QWidget* widget = hijo->widget())
            widget->deleteLater();
        delete hijo;
    }

    if (carrito.isEmpty()) {
        QLabel* vacio = new QLabel("Tu carrito está vacío.", this);
        vacio->setObjectName("lblItem");
        ui.layoutItems->addWidget(vacio);
    } else {
        for (const ItemCompra& item : carrito) {
            QLabel* fila = new QLabel(
                QString("%1 × %2\n$%3 c/u  —  $%4")
                .arg(item.cantidad)
                .arg(item.nombre)
                .arg(item.precio, 0, 'f', 2)
                .arg(item.precio * item.cantidad, 0, 'f', 2),
                this);
            fila->setObjectName("lblItem");
            ui.layoutItems->addWidget(fila);
        }
    }
    ui.layoutItems->addStretch();

    mostrarTotal();
}

void Carrito::mostrarTotal()
{
    const double total = carritoTotal();
    const int meses = ui.cmbCuotas->currentData().toInt();

    if (!carrito.isEmpty() && meses > 0) {
        // a plazos: el total no cambia, pero se cobra por cuotas mensuales
        ui.lblTotal->setText(QString("Total: $%1  —  %2 cuotas de $%3  (Saldo: $%4)")
            .arg(total, 0, 'f', 2)
            .arg(meses)
            .arg(total / meses, 0, 'f', 2)
            .arg(usuarioActual.saldo, 0, 'f', 2));
    } else {
        ui.lblTotal->setText("Total: $" + QString::number(total, 'f', 2)
            + "   (Saldo: $" + QString::number(usuarioActual.saldo, 'f', 2) + ")");
    }
}
