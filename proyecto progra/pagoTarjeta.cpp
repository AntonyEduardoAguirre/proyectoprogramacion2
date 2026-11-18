#include "clases/PagoTarjeta.h"
#include "clases/datos.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

PagoTarjeta::PagoTarjeta(double total, int meses, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Pago de tarjetas"));
    setWindowState(Qt::WindowMaximized);
    setMinimumSize(460, 420);
    setObjectName(QStringLiteral("ventanaPagoTarjeta"));
    setStyleSheet(QStringLiteral(R"(
QWidget#ventanaPagoTarjeta {
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
)"));

    auto* tarjeta = new QFrame(this);
    tarjeta->setObjectName(QStringLiteral("tarjeta"));
    auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
    sombra->setBlurRadius(28);
    sombra->setOffset(0, 8);
    sombra->setColor(QColor(41, 42, 40, 22));
    tarjeta->setGraphicsEffect(sombra);

    Q_UNUSED(total);
    Q_UNUSED(meses);
    auto* btnPagar = new QPushButton(QStringLiteral("Pagar"), tarjeta);
    btnPagar->setProperty("rol", "principal");

    connect(btnPagar, &QPushButton::clicked, this, [this]() {
        // el pago con tarjeta se aprueba de inmediato
        emit pagoAceptado();
        close();
    });

    auto* contenido = new QVBoxLayout(this);
    contenido->setContentsMargins(24, 24, 24, 24);
    contenido->addWidget(tarjeta);

    auto* contenidoTarjeta = new QVBoxLayout(tarjeta);
    contenidoTarjeta->setContentsMargins(42, 34, 42, 30);
    contenidoTarjeta->setSpacing(0);
    contenidoTarjeta->addWidget(btnPagar);
}
