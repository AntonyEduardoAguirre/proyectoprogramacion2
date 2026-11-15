#include "Carrito.h"
#include "clases/datos.h"
#include "clases/PagoEfectivo.h"
#include "clases/Recibo.h"

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

    // el método de pago decide si se permiten cuotas:
    // efectivo => solo de contado, tarjeta => contado o a plazos
    connect(ui.cmbMetodo, &QComboBox::currentIndexChanged, this, [this](int indice) {
        const bool esTarjeta = (indice == 1);
        ui.cmbCuotas->setEnabled(esTarjeta);
        mostrarTotal();
        });
    ui.cmbCuotas->setEnabled(false);   // arranca en efectivo: solo contado

    connect(ui.btnPagar, &QPushButton::clicked, this, [this]() {
        if (carrito.isEmpty()) {
            ui.lblMensaje->setText("Tu carrito está vacío.");
            return;
        }
        const bool esTarjeta = (ui.cmbMetodo->currentIndex() == 1);

        if (esTarjeta) {
            // tarjeta: el pago es inmediato (total o primera cuota según el combo)
            const int meses = ui.cmbCuotas->currentData().toInt();
            Compra recibo;
            if (pagarCarrito(meses, &recibo)) {
                ui.lblMensaje->setText("");
                emit carritoModificado();
                actualizarCarrito();
                mostrarRecibo(recibo, 0.0, 0.0);
            } else if (meses > 0) {
                ui.lblMensaje->setText("No tienes saldo suficiente para la primera cuota.");
            } else {
                ui.lblMensaje->setText("No tienes saldo suficiente para pagar todo el carrito.");
            }
        } else {
            // efectivo: se abre la calculadora del vuelto y el pago se
            // confirma ahí mismo; el recibo sale después de confirmar
            abrirPagoEfectivo();
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

// abre la calculadora del pago en efectivo sobre el carrito
void Carrito::abrirPagoEfectivo()
{
    PagoEfectivo* pago = new PagoEfectivo(carritoTotal());
    pago->setAttribute(Qt::WA_DeleteOnClose);
    pago->setWindowModality(Qt::ApplicationModal);

    connect(pago, &PagoEfectivo::pagoConfirmado, this,
            [this](double efectivoRecibido, double) {
        Compra recibo;
        double vuelto = 0.0;
        if (pagarCarritoEfectivo(efectivoRecibido, &vuelto, &recibo)) {
            ui.lblMensaje->setText("");
            emit carritoModificado();
            mostrarRecibo(recibo, efectivoRecibido, vuelto);
        }
        });

    pago->show();
}

// muestra el recibo de la compra; al cerrarlo, se cierra también el carrito
void Carrito::mostrarRecibo(const Compra& compra, double efectivoRecibido, double vuelto)
{
    Recibo* recibo = new Recibo(compra, efectivoRecibido, vuelto);
    recibo->setAttribute(Qt::WA_DeleteOnClose);
    recibo->setWindowModality(Qt::ApplicationModal);

    connect(recibo, &Recibo::reciboCerrado, this, [this]() {
        actualizarCarrito();
        this->close();
        });

    recibo->show();
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
    const bool esTarjeta = (ui.cmbMetodo->currentIndex() == 1);
    const int meses = esTarjeta ? ui.cmbCuotas->currentData().toInt() : 0;

    if (!carrito.isEmpty() && meses > 0) {
        // a plazos: el total no cambia, pero se cobra por cuotas mensuales
        ui.lblTotal->setText(QString("Total: $%1  —  %2 cuotas de $%3  (Saldo: $%4)")
            .arg(total, 0, 'f', 2)
            .arg(meses)
            .arg(total / meses, 0, 'f', 2)
            .arg(usuarioActual.saldo, 0, 'f', 2));
    } else if (esTarjeta) {
        // de contado con tarjeta: pago inmediato con el saldo de la cuenta
        ui.lblTotal->setText("Total: $" + QString::number(total, 'f', 2)
            + "   (Saldo: $" + QString::number(usuarioActual.saldo, 'f', 2) + ")");
    } else {
        // efectivo: el pago no usa el saldo; el vuelto se calcula al pagar
        ui.lblTotal->setText("Total: $" + QString::number(total, 'f', 2)
            + "   (Se calculará el vuelto del efectivo)");
    }
}
