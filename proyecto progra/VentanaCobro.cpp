#include "VentanaCobro.h"

#include "CalculadoraCambio.h"
#include "EstiloPulpe.h"

#include <QDoubleSpinBox>
#include <QFrame>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

VentanaCobro::VentanaCobro(const QString& usuario, double total, QWidget* parent)
	: QWidget(parent), usuario_(usuario), total_(new QDoubleSpinBox(this)),
	  efectivo_(new QDoubleSpinBox(this)), resultado_(new QLabel(this))
{
	setWindowTitle(QStringLiteral("Cobro / Vuelto"));
	setMinimumSize(520, 430);
	setObjectName(QStringLiteral("ventanaCobro"));
	setStyleSheet(EstiloPulpe::hojaEstilo());

	auto* tarjeta = new QFrame(this);
	tarjeta->setObjectName(QStringLiteral("tarjeta"));
	auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
	sombra->setBlurRadius(28);
	sombra->setOffset(0, 8);
	sombra->setColor(QColor(41, 42, 40, 22));
	tarjeta->setGraphicsEffect(sombra);

	auto* titulo = new QLabel(QStringLiteral("Cobro y cálculo de vuelto"), tarjeta);
	titulo->setObjectName(QStringLiteral("tituloCobro"));

	auto* btnVolver = new QPushButton(QStringLiteral("Volver"), tarjeta);
	btnVolver->setProperty("rol", "secundario");
	btnVolver->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
	btnVolver->setShortcut(QKeySequence(Qt::Key_Escape));
	btnVolver->setToolTip(QStringLiteral("Volver al menú (Esc)"));

	auto* barraSuperior = new QHBoxLayout;
	barraSuperior->addWidget(btnVolver);
	barraSuperior->addStretch();

	auto configurarDinero = [](QDoubleSpinBox* campo) {
		campo->setDecimals(2);
		campo->setMinimum(0.0);
		campo->setMaximum(999999999.99);
		campo->setPrefix(QStringLiteral("₡ "));
		campo->setButtonSymbols(QAbstractSpinBox::NoButtons);
		campo->setAlignment(Qt::AlignRight);
	};

	configurarDinero(total_);
	configurarDinero(efectivo_);
	total_->setReadOnly(true);
	setTotal(total);

	auto* formulario = new QFormLayout;
	formulario->setVerticalSpacing(12);
	auto* etiquetaTotal = new QLabel(QStringLiteral("Total a pagar:"), tarjeta);
	auto* etiquetaEfectivo = new QLabel(QStringLiteral("Efectivo recibido:"), tarjeta);
	etiquetaTotal->setObjectName(QStringLiteral("etiquetaFormulario"));
	etiquetaEfectivo->setObjectName(QStringLiteral("etiquetaFormulario"));
	formulario->addRow(etiquetaTotal, total_);
	formulario->addRow(etiquetaEfectivo, efectivo_);

	auto* calcular = new QPushButton(QStringLiteral("Calcular vuelto"), tarjeta);
	calcular->setProperty("rol", "principal");
	auto* limpiar = new QPushButton(QStringLiteral("Limpiar"), tarjeta);
	limpiar->setProperty("rol", "secundario");
	auto* botones = new QHBoxLayout;
	botones->addWidget(calcular);
	botones->addWidget(limpiar);

	resultado_->setAlignment(Qt::AlignCenter);
	resultado_->setMinimumHeight(60);
	resultado_->setWordWrap(true);
	resultado_->setObjectName(QStringLiteral("resultado"));

	auto* contenido = new QVBoxLayout(this);
	contenido->setContentsMargins(24, 24, 24, 24);
	contenido->addWidget(tarjeta);

	auto* contenidoTarjeta = new QVBoxLayout(tarjeta);
	contenidoTarjeta->setContentsMargins(42, 34, 42, 30);
	contenidoTarjeta->setSpacing(0);
	contenidoTarjeta->addLayout(barraSuperior);
	contenidoTarjeta->addSpacing(18);
	contenidoTarjeta->addWidget(titulo);
	contenidoTarjeta->addSpacing(24);
	contenidoTarjeta->addLayout(formulario);
	contenidoTarjeta->addSpacing(20);
	contenidoTarjeta->addLayout(botones);
	contenidoTarjeta->addSpacing(12);
	contenidoTarjeta->addWidget(resultado_);
	contenidoTarjeta->addStretch();

	connect(calcular, &QPushButton::clicked, this, &VentanaCobro::calcularVuelto);
	connect(limpiar, &QPushButton::clicked, this, &VentanaCobro::limpiar);
	connect(btnVolver, &QPushButton::clicked, this, &QWidget::close);
	connect(efectivo_, &QDoubleSpinBox::editingFinished,
			this, &VentanaCobro::calcularVuelto);
}

void VentanaCobro::setTotal(double total)
{
	total_->setValue(total > 0.0 ? total : 0.0);
	resultado_->clear();
	resultado_->setProperty("estado", QVariant());
	style()->unpolish(resultado_);
	style()->polish(resultado_);
}

void VentanaCobro::calcularVuelto()
{
	const CalculadoraCambio::Resultado resultado =
		CalculadoraCambio::calcular(total_->value(), efectivo_->value());

	if (!resultado.valido) {
		mostrarError(resultado.error);
		return;
	}

	resultado_->setProperty("estado", "exito");
	style()->unpolish(resultado_);
	style()->polish(resultado_);
	resultado_->setText(QStringLiteral("Vuelto: ₡ %1")
							.arg(resultado.cambio, 0, 'f', 2));
}

void VentanaCobro::limpiar()
{
	efectivo_->setValue(0.0);
	resultado_->clear();
	resultado_->setProperty("estado", QVariant());
	style()->unpolish(resultado_);
	style()->polish(resultado_);
}

void VentanaCobro::mostrarError(const QString& mensaje)
{
	resultado_->setProperty("estado", "error");
	style()->unpolish(resultado_);
	style()->polish(resultado_);
	resultado_->setText(mensaje);
}
