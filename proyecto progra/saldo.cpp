#include "clases/Saldo.h"
#include "clases/datos.h"

#include <QPushButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QGraphicsDropShadowEffect>
#include <QColor>

Saldo::Saldo(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowMaximized);
	setAttribute(Qt::WA_StyledBackground, true); // para que el QSS pinte el fondo

	// La sombra suave no se puede definir en el .ui
	auto* sombra = new QGraphicsDropShadowEffect(ui.softCard);
	sombra->setBlurRadius(28);
	sombra->setOffset(0, 8);
	sombra->setColor(QColor(41, 42, 40, 22));
	ui.softCard->setGraphicsEffect(sombra);

	actualizarDatos();

	connect(ui.btnDepositar, &QPushButton::clicked, this, [this]() {
		const double monto = ui.spnMonto->value();
		if (monto <= 0.0) {
			ui.lblMensaje->setText("Ingresa un monto mayor a $0.00.");
			return;
		}
		if (depositarSaldo(monto)) {
			ui.lblMensaje->setText("");
			actualizarDatos();
		} else {
			ui.lblMensaje->setText("No se pudo realizar el depósito.");
		}
		});

	// la flecha de arriba hace lo mismo que el botón de volver
	connect(ui.btnFlecha, &QPushButton::clicked, ui.btnRegresar, &QPushButton::click);

	connect(ui.btnRegresar, &QPushButton::clicked, this, &QWidget::close);
}

Saldo::~Saldo()
{
}

void Saldo::actualizarDatos()
{
	ui.lblSaldoActual->setText("Saldo actual: $" + QString::number(usuarioActual.saldo, 'f', 2));
}
