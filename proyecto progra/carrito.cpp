#include "Carrito.h"
#include "clases/datos.h"
#include "clases/PagoCuotas.h"
#include "clases/CuotaCompletada.h"
#include "clases/PagoEfectivo.h"
#include "clases/PagoTarjeta.h"
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

    // el método de pago (efectivo / tarjeta de contado / a cuotas)
    connect(ui.cmbMetodo, &QComboBox::currentIndexChanged, this, [this](int) {
        mostrarTotal();
        });

    connect(ui.btnPagar, &QPushButton::clicked, this, [this]() {
        if (carrito.isEmpty()) {
            ui.lblMensaje->setText("Tu carrito está vacío.");
            return;
        }
        const int metodo = ui.cmbMetodo->currentIndex();

        if (metodo == 0) {
            // efectivo: se abre la caja registradora del vuelto y el pago
            // se confirma ahí mismo; el recibo sale después de confirmar
            abrirPagoEfectivo();
        } else if (metodo == 1) {
            // tarjeta de contado: se pide el número de tarjeta y el pago
            // se procesa al instante; el recibo sale después de confirmar
            abrirPagoTarjeta(0);
        } else {
            // cuotas: ventana propia del pago a cuotas (plazo, nombre y registro)
            abrirPagoCuotas(mesesDelMetodo(metodo));
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

    actualizarCarrito();
}

Carrito::~Carrito()
{
}

// abre la calculadora del pago en efectivo sobre el carrito
void Carrito::abrirPagoEfectivo()
{
    PagoEfectivo* pago = new PagoEfectivo(carritoTotal(), carrito);
    pago->setAttribute(Qt::WA_DeleteOnClose);
    pago->setWindowModality(Qt::ApplicationModal);

    connect(pago, &PagoEfectivo::pagoConfirmado, this,
            [this](double efectivoRecibido, double vuelto) {
        Compra recibo;
        if (pagarCarritoEfectivo(efectivoRecibido, &vuelto, &recibo)) {
            ui.lblMensaje->setText("");
            emit carritoModificado();
            mostrarRecibo(recibo, efectivoRecibido, vuelto);
        }
        });

    pago->show();
}

// abre la ventana del pago a cuotas sobre el carrito
void Carrito::abrirPagoCuotas(int mesesIniciales)
{
    PagoCuotas* pago = new PagoCuotas(carritoTotal(), mesesIniciales);
    pago->setAttribute(Qt::WA_DeleteOnClose);
    pago->setWindowModality(Qt::ApplicationModal);

    connect(pago, &PagoCuotas::pagoCompletado, this,
            [this](int meses, const QString& cliente, const QString& fecha,
                   double totalSinAumento, double totalConAumento,
                   double pagoMensual, int mesesRestantes) {
        // registra la compra (stock, historial y carrito) con el plazo elegido
        Compra recibo;
        pagarCarrito(meses, &recibo);
        ui.lblMensaje->setText("");
        emit carritoModificado();

        // recibo de la cuota completada; al cerrarlo se cierra el carrito
        CuotaCompletada* completada = new CuotaCompletada(
            recibo, cliente, fecha, totalSinAumento, totalConAumento,
            pagoMensual, mesesRestantes);
        completada->setAttribute(Qt::WA_DeleteOnClose);
        completada->setWindowModality(Qt::ApplicationModal);
        connect(completada, &CuotaCompletada::cuotaCerrada, this, [this]() {
            actualizarCarrito();
            this->close();
            });
        completada->show();
        });

    pago->show();
}

// abre la ventana del pago con tarjeta sobre el carrito
void Carrito::abrirPagoTarjeta(int meses)
{
    PagoTarjeta* pago = new PagoTarjeta(carritoTotal(), meses);
    pago->setAttribute(Qt::WA_DeleteOnClose);
    pago->setWindowModality(Qt::ApplicationModal);

    connect(pago, &PagoTarjeta::pagoAceptado, this,
            [this, meses]() {
        Compra recibo;
        pagarCarrito(meses, &recibo);
        ui.lblMensaje->setText("");
        emit carritoModificado();
        mostrarRecibo(recibo, 0.0, 0.0);
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
    const int metodo = ui.cmbMetodo->currentIndex();
    const int meses = mesesDelMetodo(metodo);

    if (!carrito.isEmpty() && meses > 0) {
        // a plazos: el precio sube 0.5% por cada mes de plazo
        const double totalFinal = totalConInteres(total, meses);
        ui.lblTotal->setText(QString("Total: $%1  —  %2 cuotas de $%3")
            .arg(totalFinal, 0, 'f', 2)
            .arg(meses)
            .arg(totalFinal / meses, 0, 'f', 2));
    } else if (metodo >= 1) {
        // tarjeta de contado: pago inmediato
        ui.lblTotal->setText("Total: $" + QString::number(total, 'f', 2));
    } else {
        // efectivo: el vuelto se calcula en la caja registradora
        ui.lblTotal->setText("Total: $" + QString::number(total, 'f', 2)
            + "   (El vuelto se calcula al pagar en efectivo)");
    }
}
