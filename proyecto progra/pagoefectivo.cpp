#include "clases/PagoEfectivo.h"

#include <QDoubleSpinBox>
#include <QFrame>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

PagoEfectivo::PagoEfectivo(double total, double saldoCuenta,
                           const QList<ItemCompra>& items, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Cobro en efectivo"));
    setMinimumSize(460, 620);
    setObjectName(QStringLiteral("ventanaPagoEfectivo"));
    setStyleSheet(QStringLiteral(R"(
QWidget#ventanaPagoEfectivo {
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
QLabel#etiquetaFormulario {
	color: #292A28;
	font-family: "Segoe UI";
	font-size: 12px;
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
QScrollArea {
	background: transparent;
	border: none;
}
QDoubleSpinBox {
	background-color: #FAFAF8;
	color: #292A28;
	border: 1px solid #E8E7E1;
	border-radius: 8px;
	font-family: "Segoe UI";
	font-size: 13px;
	padding: 0 13px;
	min-height: 43px;
}
QDoubleSpinBox:focus {
	border: 1px solid #899B83;
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
QPushButton[rol="principal"]:disabled {
	background-color: #D5DAD2;
	color: #FAFAF8;
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
QLabel#resultado[estado="exito"] {
	color: #899B83;
	font-family: "Segoe UI";
	font-size: 24px;
	font-weight: 600;
}
QLabel#resultado[estado="error"] {
	color: #A9503F;
	font-family: "Segoe UI";
	font-size: 12px;
	font-weight: 600;
}
QLabel#resultado {
	color: #92918B;
	font-family: "Segoe UI";
	font-size: 12px;
}
)"));

    auto* tarjeta = new QFrame(this);
    tarjeta->setObjectName(QStringLiteral("tarjeta"));
    auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
    sombra->setBlurRadius(28);
    sombra->setOffset(0, 8);
    sombra->setColor(QColor(41, 42, 40, 22));
    tarjeta->setGraphicsEffect(sombra);

    auto* eyebrow = new QLabel(QStringLiteral("CAJA REGISTRADORA"), tarjeta);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    auto* titulo = new QLabel(QStringLiteral("Cobro en efectivo."), tarjeta);
    titulo->setObjectName(QStringLiteral("titulo"));

    // ticket con los artículos seleccionados, como en una caja registradora
    auto* areaItems = new QScrollArea(tarjeta);
    areaItems->setWidgetResizable(true);
    areaItems->setFrameShape(QFrame::NoFrame);
    auto* contenedorItems = new QWidget(areaItems);
    auto* layoutItems = new QVBoxLayout(contenedorItems);
    layoutItems->setContentsMargins(0, 0, 6, 0);
    layoutItems->setSpacing(10);
    for (const ItemCompra& item : items) {
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

    auto* lblTotal = new QLabel(this);
    lblTotal->setObjectName(QStringLiteral("lblTotal"));
    lblTotal->setText(QStringLiteral("Total a pagar: $%1   (Saldo de la cuenta: $%2)")
                          .arg(total, 0, 'f', 2)
                          .arg(saldoCuenta, 0, 'f', 2));

    auto* efectivo = new QDoubleSpinBox(tarjeta);
    efectivo->setDecimals(2);
    efectivo->setMinimum(0.0);
    efectivo->setMaximum(999999999.99);
    efectivo->setPrefix(QStringLiteral("$ "));
    efectivo->setButtonSymbols(QAbstractSpinBox::NoButtons);
    efectivo->setAlignment(Qt::AlignRight);

    auto* resultado = new QLabel(tarjeta);
    resultado->setObjectName(QStringLiteral("resultado"));
    resultado->setAlignment(Qt::AlignCenter);
    resultado->setMinimumHeight(60);
    resultado->setWordWrap(true);

    auto* btnConfirmar = new QPushButton(QStringLiteral("Confirmar pago"), tarjeta);
    btnConfirmar->setProperty("rol", "principal");
    btnConfirmar->setEnabled(false);

    auto* btnCancelar = new QPushButton(QStringLiteral("Cancelar"), tarjeta);
    btnCancelar->setProperty("rol", "secundario");

    // recalcula el vuelto con cada cambio del efectivo recibido
    auto actualizar = [this, total, saldoCuenta, efectivo, resultado, btnConfirmar]() {
        // se compara en centavos para evitar errores de redondeo
        const qint64 diferencia = qRound64(efectivo->value() * 100.0)
                                 - qRound64(total * 100.0);

        if (diferencia < 0) {
            if (qRound64(saldoCuenta * 100.0) >= qRound64(total * 100.0)) {
                // el efectivo físico no alcanza, pero el saldo de la
                // cuenta del cliente sí cubre el total: se permite pagar
                resultado->setProperty("estado", "exito");
                resultado->setText(
                    QStringLiteral("El efectivo no alcanza, pero tu saldo de $%1 cubre el pago.")
                        .arg(saldoCuenta, 0, 'f', 2));
                btnConfirmar->setEnabled(true);
            } else {
                const double faltante = -diferencia / 100.0;
                resultado->setProperty("estado", "error");
                resultado->setText(QStringLiteral("Saldo insuficiente. Faltan $%1")
                                       .arg(faltante, 0, 'f', 2));
                btnConfirmar->setEnabled(false);
            }
        } else if (diferencia == 0) {
            resultado->setProperty("estado", "exito");
            resultado->setText(QStringLiteral("Pago completo. Sin vuelto."));
            btnConfirmar->setEnabled(true);
        } else {
            const double cambio = diferencia / 100.0;
            resultado->setProperty("estado", "exito");
            resultado->setText(QStringLiteral("Vuelto: $%1").arg(cambio, 0, 'f', 2));
            btnConfirmar->setEnabled(true);
        }
        // el QSS se recalcula al cambiar una propiedad dinámica
        style()->unpolish(resultado);
        style()->polish(resultado);
    };

    connect(efectivo, &QDoubleSpinBox::valueChanged, this, actualizar);
    actualizar();   // muestra desde el arranque por qué se puede (o no) confirmar

    connect(btnConfirmar, &QPushButton::clicked, this,
            [this, total, saldoCuenta, efectivo]() {
        const qint64 diferencia = qRound64(efectivo->value() * 100.0)
                                 - qRound64(total * 100.0);
        // si el efectivo no alcanza, el pago se hace con el saldo de la cuenta
        const bool conSaldo = diferencia < 0;
        emit pagoConfirmado(efectivo->value(),
                            conSaldo ? 0.0 : diferencia / 100.0,
                            conSaldo);
        close();
    });

    connect(btnCancelar, &QPushButton::clicked, this, &QWidget::close);

    auto* formulario = new QFormLayout;
    formulario->setVerticalSpacing(12);
    auto* etiquetaEfectivo = new QLabel(QStringLiteral("Efectivo recibido:"), tarjeta);
    etiquetaEfectivo->setObjectName(QStringLiteral("etiquetaFormulario"));
    formulario->addRow(etiquetaEfectivo, efectivo);

    auto* botones = new QHBoxLayout;
    botones->addWidget(btnCancelar);
    botones->addWidget(btnConfirmar);

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
    contenidoTarjeta->addWidget(areaItems, 1);
    contenidoTarjeta->addSpacing(14);
    contenidoTarjeta->addWidget(lblTotal);
    contenidoTarjeta->addSpacing(18);
    contenidoTarjeta->addLayout(formulario);
    contenidoTarjeta->addSpacing(12);
    contenidoTarjeta->addWidget(resultado);
    contenidoTarjeta->addSpacing(18);
    contenidoTarjeta->addLayout(botones);
    contenidoTarjeta->addStretch();
}
