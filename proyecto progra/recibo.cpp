#include "clases/Recibo.h"
#include "clases/datos.h"

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

Recibo::Recibo(const Compra& compra, double efectivoRecibido,
               double vuelto, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Recibo de compra"));
    setWindowState(Qt::WindowMaximized);
    setMinimumSize(460, 560);
    setObjectName(QStringLiteral("ventanaRecibo"));
    setStyleSheet(QStringLiteral(R"(
QWidget#ventanaRecibo {
	background-color: #F5F3EE;
}
QFrame#tarjeta {
	background-color: #FFFFFF;
	border: 1px solid #EAE8E2;
	border-radius: 18px;
}
QLabel#titulo {
	color: #292A28;
	font-family: Georgia;
	font-size: 29px;
}
QLabel#eyebrow {
	color: #92918B;
	font-family: "Segoe UI";
	font-size: 11px;
	font-weight: 600;
}
QLabel#lblItem {
	color: #292A28;
	background: #FAFAF8;
	border: 1px solid #EAE8E2;
	border-radius: 8px;
	padding: 14px;
	font-size: 13px;
}
QLabel#lblTotal {
	color: #292A28;
	background: #FAFAF8;
	border: 1px solid #EAE8E2;
	border-radius: 8px;
	padding: 14px;
	font-size: 14px;
	font-weight: 600;
}
QLabel#lblPago {
	color: #292A28;
	background: #F0F3EE;
	border: 1px solid #D5DAD2;
	border-radius: 8px;
	padding: 14px;
	font-size: 13px;
}
QLabel#copyright {
	color: #92918B;
	font-family: "Segoe UI";
	font-size: 12px;
}
QPushButton {
	min-height: 43px;
	border-radius: 8px;
	padding: 0 13px;
	font-family: "Segoe UI";
	font-size: 13px;
	font-weight: 600;
}
QPushButton[rol="principal"] {
	background-color: #899B83;
	color: #FFFFFF;
	border: none;
}
QPushButton[rol="principal"]:hover {
	background-color: #788B72;
}
QPushButton[rol="principal"]:pressed {
	background-color: #6D8067;
}
QPushButton[rol="secundario"] {
	background-color: #FFFFFF;
	color: #899B83;
	border: 1px solid #899B83;
}
QPushButton[rol="secundario"]:hover {
	background-color: #F0F3EE;
	color: #788B72;
}
QScrollArea {
	background: transparent;
	border: none;
}
QScrollArea > QWidget > QWidget {
	background: transparent;
}
QScrollBar:vertical {
	background: transparent;
	width: 8px;
	margin: 0;
}
QScrollBar::handle:vertical {
	background: #D8DED5;
	border-radius: 4px;
	min-height: 30px;
}
QScrollBar::handle:vertical:hover {
	background: #B9C2B4;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
	height: 0;
}
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
	background: transparent;
}
QScrollBar:horizontal {
	background: transparent;
	height: 0;
}
)"));

    auto* tarjeta = new QFrame(this);
    tarjeta->setObjectName(QStringLiteral("tarjeta"));

    auto* eyebrow = new QLabel(QStringLiteral("RECIBO DE COMPRA"), tarjeta);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    auto* titulo = new QLabel(QStringLiteral("¡Gracias por tu compra!"), tarjeta);
    titulo->setObjectName(QStringLiteral("titulo"));

    auto* lblCliente = new QLabel(
        QStringLiteral("Cliente: %1   —   Fecha: %2")
            .arg(usuarioActual.nombre, compra.fecha),
        tarjeta);
    lblCliente->setObjectName(QStringLiteral("lblPago"));

    // lista de productos comprados
    auto* areaItems = new QScrollArea(tarjeta);
    areaItems->setWidgetResizable(true);
    areaItems->setFrameShape(QFrame::NoFrame);
    auto* contenedorItems = new QWidget(areaItems);
    auto* layoutItems = new QVBoxLayout(contenedorItems);
    layoutItems->setContentsMargins(0, 0, 6, 0);
    layoutItems->setSpacing(10);
    for (const ItemCompra& item : compra.items) {
        auto* fila = new QLabel(
            QStringLiteral("%1 × %2\n$%3 c/u  —  $%4")
                .arg(item.cantidad)
                .arg(item.nombre)
                .arg(item.precio, 0, 'f', 2)
                .arg(item.precio * item.cantidad, 0, 'f', 2),
            contenedorItems);
        fila->setObjectName(QStringLiteral("lblItem"));
        layoutItems->addWidget(fila);
    }
    layoutItems->addStretch();
    areaItems->setWidget(contenedorItems);

    auto* lblTotal = new QLabel(
        QStringLiteral("Total: $%1").arg(compra.total, 0, 'f', 2),
        tarjeta);
    lblTotal->setObjectName(QStringLiteral("lblTotal"));

    // detalles de la forma de pago
    QString detallesPago = QStringLiteral("Método de pago: %1").arg(compra.metodo);
    if (compra.metodo == QStringLiteral("Efectivo")) {
        detallesPago += QStringLiteral("\nEfectivo recibido: $%1   —   Vuelto: $%2")
                            .arg(efectivoRecibido, 0, 'f', 2)
                            .arg(vuelto, 0, 'f', 2);
    } else if (compra.meses > 0) {
        detallesPago += QStringLiteral("\n%1 cuotas mensuales de $%2 (primera cuota cobrada hoy)")
                            .arg(compra.meses)
                            .arg(compra.cuotaMensual, 0, 'f', 2);
    } else {
        detallesPago += QStringLiteral("\nPago inmediato de contado.");
    }
    auto* lblPago = new QLabel(detallesPago, tarjeta);
    lblPago->setObjectName(QStringLiteral("lblPago"));
    lblPago->setWordWrap(true);

    auto* btnListo = new QPushButton(QStringLiteral("Listo"), tarjeta);
    btnListo->setProperty("rol", "principal");
    connect(btnListo, &QPushButton::clicked, this, [this]() {
        emit reciboCerrado();
        close();
    });

    auto* copyright = new QLabel(QStringLiteral("© 2026 UNA Electros"), tarjeta);
    copyright->setObjectName(QStringLiteral("copyright"));
    copyright->setAlignment(Qt::AlignCenter);

    auto* contenido = new QVBoxLayout(this);
    contenido->setContentsMargins(24, 24, 24, 24);
    contenido->addWidget(tarjeta);

    auto* contenidoTarjeta = new QVBoxLayout(tarjeta);
    contenidoTarjeta->setContentsMargins(42, 34, 42, 30);
    contenidoTarjeta->setSpacing(0);
    contenidoTarjeta->addWidget(eyebrow);
    contenidoTarjeta->addSpacing(8);
    contenidoTarjeta->addWidget(titulo);
    contenidoTarjeta->addSpacing(18);
    contenidoTarjeta->addWidget(lblCliente);
    contenidoTarjeta->addSpacing(14);
    contenidoTarjeta->addWidget(areaItems, 1);
    contenidoTarjeta->addSpacing(14);
    contenidoTarjeta->addWidget(lblTotal);
    contenidoTarjeta->addSpacing(10);
    contenidoTarjeta->addWidget(lblPago);
    contenidoTarjeta->addSpacing(18);
    contenidoTarjeta->addWidget(btnListo);
    contenidoTarjeta->addSpacing(10);
    contenidoTarjeta->addWidget(copyright);
}
