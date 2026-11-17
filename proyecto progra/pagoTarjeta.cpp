#include "clases/PagoTarjeta.h"
#include "clases/datos.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStyle>
#include <QVBoxLayout>
#include <QWidget>

PagoTarjeta::PagoTarjeta(double total, int meses, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Pago de tarjetas"));
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
QLabel#lblTotal {
	color: #292A28;
	background: #FAFAF8;
	border: 1px solid #EAE8E2;
	border-radius: 8px;
	padding: 14px;
	font-size: 14px;
	font-weight: 600;
}
QLabel#ayuda {
	color: #92918B;
	font-family: "Segoe UI";
	font-size: 11px;
}
QLineEdit {
	background-color: #FAFAF8;
	color: #292A28;
	border: 1px solid #E8E7E1;
	border-radius: 8px;
	font-family: "Segoe UI";
	font-size: 15px;
	padding: 0 13px;
	min-height: 43px;
}
QLineEdit:focus {
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
)"));

    auto* tarjeta = new QFrame(this);
    tarjeta->setObjectName(QStringLiteral("tarjeta"));
    auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
    sombra->setBlurRadius(28);
    sombra->setOffset(0, 8);
    sombra->setColor(QColor(41, 42, 40, 22));
    tarjeta->setGraphicsEffect(sombra);

    auto* eyebrow = new QLabel(QStringLiteral("PAGO DE TARJETAS"), tarjeta);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));

    const QString tituloTexto = (meses > 0)
        ? QStringLiteral("Compra a %1 cuotas.").arg(meses)
        : QStringLiteral("Pago de tarjetas.");
    auto* titulo = new QLabel(tituloTexto, tarjeta);
    titulo->setObjectName(QStringLiteral("titulo"));

    // monto a cobrar: el total de contado, o la primera cuota del
    // total con el interés del 5% por cada mes de plazo
    const double totalFinal = totalConInteres(total, meses);
    const double cobro = (meses > 0) ? totalFinal / meses : total;
    auto* lblTotal = new QLabel(
        (meses > 0)
            ? QStringLiteral("Primera cuota: $%1  —  %2 cuotas mensuales de $%3")
                  .arg(cobro, 0, 'f', 2).arg(meses).arg(cobro, 0, 'f', 2)
            : QStringLiteral("Total a pagar: $%1").arg(total, 0, 'f', 2),
        tarjeta);
    lblTotal->setObjectName(QStringLiteral("lblTotal"));
    lblTotal->setWordWrap(true);

    // número de tarjeta: solo dígitos y espacios; se piden 16 dígitos
    auto* numero = new QLineEdit(tarjeta);
    numero->setPlaceholderText(QStringLiteral("1234 5678 9012 3456"));
    numero->setAlignment(Qt::AlignRight);
    auto* validador = new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9 ]{0,19}")), numero);
    numero->setValidator(validador);

    auto* ayuda = new QLabel(QStringLiteral("Digite los 16 dígitos de la tarjeta."), tarjeta);
    ayuda->setObjectName(QStringLiteral("ayuda"));

    auto* btnPagar = new QPushButton(
        (meses > 0)
            ? QStringLiteral("Pagar primera cuota: $%1").arg(cobro, 0, 'f', 2)
            : QStringLiteral("Pagar: $%1").arg(total, 0, 'f', 2),
        tarjeta);
    btnPagar->setProperty("rol", "principal");
    btnPagar->setEnabled(false);

    auto* btnCancelar = new QPushButton(QStringLiteral("Cancelar"), tarjeta);
    btnCancelar->setProperty("rol", "secundario");

    // habilita el pago cuando el número tiene exactamente 16 dígitos
    auto numeroValido = [numero]() -> bool {
        const QString limpio = numero->text().remove(' ');
        return limpio.length() == 16;
    };

    auto actualizar = [numeroValido, numero, btnPagar, ayuda]() {
        btnPagar->setEnabled(numeroValido());
        if (numero->text().isEmpty())
            ayuda->setText(QStringLiteral("Digite los 16 dígitos de la tarjeta."));
        else if (numeroValido())
            ayuda->setText(QStringLiteral("Tarjeta lista para pagar."));
        else
            ayuda->setText(QStringLiteral("Faltan dígitos..."));
    };

    connect(numero, &QLineEdit::textChanged, this, actualizar);
    actualizar();

    connect(btnPagar, &QPushButton::clicked, this, [this]() {
        // el pago con tarjeta se aprueba de inmediato
        emit pagoAceptado();
        close();
    });

    connect(btnCancelar, &QPushButton::clicked, this, &QWidget::close);

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
    contenidoTarjeta->addWidget(lblTotal);
    contenidoTarjeta->addSpacing(18);
    contenidoTarjeta->addWidget(new QLabel(QStringLiteral("Número de tarjeta:"), tarjeta));
    contenidoTarjeta->addSpacing(6);
    contenidoTarjeta->addWidget(numero);
    contenidoTarjeta->addSpacing(4);
    contenidoTarjeta->addWidget(ayuda);
    contenidoTarjeta->addSpacing(18);

    auto* botones = new QHBoxLayout;
    botones->addWidget(btnCancelar);
    botones->addWidget(btnPagar);
    contenidoTarjeta->addLayout(botones);
    contenidoTarjeta->addStretch();

    numero->setFocus();
}
