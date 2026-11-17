#include "clases/PagoCuotas.h"

#include "clases/datos.h"

#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace {

bool guardarRegistroCuotas(const QJsonObject& registro, QString* error)
{
    return agregarRegistroCuotas(registro, error);
}

} // namespace

PagoCuotas::PagoCuotas(double totalSinAumento, int mesesIniciales, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Pago a cuotas"));
    setMinimumSize(460, 560);
    setObjectName(QStringLiteral("ventanaPagoCuotas"));
    setStyleSheet(QStringLiteral(R"(
QWidget#ventanaPagoCuotas {
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
QLabel#lblResumen {
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
QComboBox {
	background: #FAFAF8;
	border: 1px solid #E8E7E1;
	border-radius: 8px;
	font-family: "Segoe UI";
	font-size: 13px;
	padding: 0 13px;
	min-height: 43px;
	color: #292A28;
}
QComboBox::drop-down {
	border: none;
}
QComboBox QAbstractItemView {
	background: #FFFFFF;
	color: #292A28;
	selection-background-color: #E5EAE2;
	selection-color: #292A28;
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

    auto* eyebrow = new QLabel(QStringLiteral("PAGO A CUOTAS"), tarjeta);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    auto* titulo = new QLabel(QStringLiteral("Elige tu plazo."), tarjeta);
    titulo->setObjectName(QStringLiteral("titulo"));

    // precio total de los productos sin ningún aumento
    auto* lblTotalSin = new QLabel(
        QStringLiteral("Precio total sin aumento: $%1")
            .arg(QString::number(totalSinAumento, 'f', 2)),
        tarjeta);
    lblTotalSin->setObjectName(QStringLiteral("lblResumen"));

    // selección del plazo: 3, 6, 12 o 24 meses
    const QList<int> plazos = { 3, 6, 12, 24 };
    auto* comboPlazo = new QComboBox(tarjeta);
    for (int meses : plazos)
        comboPlazo->addItem(QStringLiteral("%1 meses").arg(meses));
    const int indiceInicial = plazos.indexOf(mesesIniciales);
    comboPlazo->setCurrentIndex(indiceInicial >= 0 ? indiceInicial : 0);

    // líneas que se actualizan al cambiar el plazo
    auto* lblAumento = new QLabel(tarjeta);
    lblAumento->setObjectName(QStringLiteral("ayuda"));
    auto* lblTotalCon = new QLabel(tarjeta);
    lblTotalCon->setObjectName(QStringLiteral("lblResumen"));
    lblTotalCon->setWordWrap(true);
    auto* lblPagoMensual = new QLabel(tarjeta);
    lblPagoMensual->setObjectName(QStringLiteral("lblResumen"));
    lblPagoMensual->setWordWrap(true);

    // nombre del cliente
    auto* nombre = new QLineEdit(tarjeta);
    nombre->setPlaceholderText(QStringLiteral("Nombre del cliente"));
    auto* avisoNombre = new QLabel(tarjeta);
    avisoNombre->setObjectName(QStringLiteral("ayuda"));

    auto* btnCompletar = new QPushButton(QStringLiteral("Completar cuota"), tarjeta);
    btnCompletar->setProperty("rol", "principal");

    auto* btnCancelar = new QPushButton(QStringLiteral("Cancelar"), tarjeta);
    btnCancelar->setProperty("rol", "secundario");

    // recalcula los montos con cada cambio de plazo y valida el nombre
    auto recalcular = [totalSinAumento, plazos, comboPlazo,
                       lblAumento, lblTotalCon, lblPagoMensual,
                       nombre, avisoNombre, btnCompletar]() {
        const int meses = plazos.value(comboPlazo->currentIndex(), 3);

        // recargo: 0.5% por cada mes del plazo sobre el precio total
        const double totalConAumento = totalSinAumento * (1.0 + 0.005 * meses);
        // pago mensual: 10% del total con aumento
        const double pagoMensual = totalConAumento * 0.10;
        // la primera cuota se paga hoy; quedan los meses restantes
        const int mesesRestantes = meses - 1;

        lblAumento->setText(QStringLiteral("Aumento: 0.5% por cada mes → %1% de recargo")
                                .arg(QString::number(0.5 * meses, 'f', 1)));
        lblTotalCon->setText(QStringLiteral("Precio total con aumento: $%1")
                                 .arg(QString::number(totalConAumento, 'f', 2)));
        lblPagoMensual->setText(
            QStringLiteral("Pago mensual (10%): $%1  —  quedan %2 pagos mensuales")
                .arg(QString::number(pagoMensual, 'f', 2))
                .arg(mesesRestantes));

        const bool nombreValido = !nombre->text().trimmed().isEmpty();
        btnCompletar->setEnabled(nombreValido);
        avisoNombre->setText(nombreValido
            ? QStringLiteral("Listo para completar la cuota.")
            : QStringLiteral("Escribe el nombre del cliente para continuar."));
    };

    connect(comboPlazo, &QComboBox::currentIndexChanged, this, recalcular);
    connect(nombre, &QLineEdit::textChanged, this, recalcular);
    recalcular();

    connect(btnCompletar, &QPushButton::clicked, this,
            [this, plazos, comboPlazo, nombre, totalSinAumento]() {
        const QString cliente = nombre->text().trimmed();
        if (cliente.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Pago a cuotas"),
                                 QStringLiteral("El nombre del cliente no puede estar vacío."));
            return;
        }

        const int meses = plazos.value(comboPlazo->currentIndex(), 3);
        const double porcentajeAumento = 0.5 * meses;   // 0.5% por cada mes
        const double totalConAumento = totalSinAumento * (1.0 + 0.005 * meses);
        const double pagoMensual = totalConAumento * 0.10;
        const int mesesRestantes = meses - 1;

        // registro que se guarda en datos/cuotas.json
        const QString fecha = QDateTime::currentDateTime().toString(
            QStringLiteral("dd/MM/yyyy hh:mm:ss"));

        QJsonObject registro;
        registro["cliente"] = cliente;
        registro["meses"] = meses;
        registro["total_sin_aumento"] = totalSinAumento;
        registro["porcentaje_aumento"] = porcentajeAumento;
        registro["total_con_aumento"] = totalConAumento;
        registro["pago_mensual"] = pagoMensual;
        registro["meses_restantes"] = mesesRestantes;
        registro["fecha"] = fecha;

        QString error;
        if (!guardarRegistroCuotas(registro, &error)) {
            QMessageBox::warning(this, QStringLiteral("Pago a cuotas"),
                                 QStringLiteral("No se pudo guardar el registro en cuotas.json:\n%1")
                                     .arg(error));
            return;
        }

        emit pagoCompletado(meses, cliente, fecha, totalSinAumento,
                            totalConAumento, pagoMensual, mesesRestantes);
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
    contenidoTarjeta->addWidget(lblTotalSin);
    contenidoTarjeta->addSpacing(18);

    auto* filaPlazo = new QHBoxLayout;
    auto* etiquetaPlazo = new QLabel(QStringLiteral("Plazo:"), tarjeta);
    etiquetaPlazo->setObjectName(QStringLiteral("etiquetaFormulario"));
    filaPlazo->addWidget(etiquetaPlazo);
    filaPlazo->addWidget(comboPlazo, 1);
    contenidoTarjeta->addLayout(filaPlazo);
    contenidoTarjeta->addSpacing(6);
    contenidoTarjeta->addWidget(lblAumento);
    contenidoTarjeta->addSpacing(10);
    contenidoTarjeta->addWidget(lblTotalCon);
    contenidoTarjeta->addSpacing(10);
    contenidoTarjeta->addWidget(lblPagoMensual);
    contenidoTarjeta->addSpacing(18);

    auto* etiquetaNombre = new QLabel(QStringLiteral("Nombre del cliente:"), tarjeta);
    etiquetaNombre->setObjectName(QStringLiteral("etiquetaFormulario"));
    contenidoTarjeta->addWidget(etiquetaNombre);
    contenidoTarjeta->addSpacing(6);
    contenidoTarjeta->addWidget(nombre);
    contenidoTarjeta->addSpacing(4);
    contenidoTarjeta->addWidget(avisoNombre);
    contenidoTarjeta->addSpacing(18);

    auto* botones = new QHBoxLayout;
    botones->addWidget(btnCancelar);
    botones->addWidget(btnCompletar);
    contenidoTarjeta->addLayout(botones);
    contenidoTarjeta->addStretch();
}
