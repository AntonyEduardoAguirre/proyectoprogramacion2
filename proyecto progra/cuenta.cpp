#include "clases/Cuenta.h"
#include "clases/Inicio.h"
#include "clases/datos.h"
#include "clases/Cambio.h"
#include "clases/Seguro.h"
#include "Historial.h"

#include <QCloseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

cuenta::cuenta(QWidget* parent)
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

	actualizarDatos();

	connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
		inicio* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);
		ventanaInicio->show();
		this->close();
		});

	connect(ui.btnDesactivar, &QPushButton::clicked, this, [this]() {
		seguro* ventanaSeguro = new seguro();
		ventanaSeguro->setWindowModality(Qt::ApplicationModal);
		ventanaSeguro->show();
		});

	connect(ui.btnCambiar, &QPushButton::clicked, this, [this]() {
		cambio* ventanaCambio = new cambio();
		ventanaCambio->setAttribute(Qt::WA_DeleteOnClose);
		ventanaCambio->show();
		this->close();
		});

	connect(ui.btnHistorial, &QPushButton::clicked, this, [this]() {
		historial* ventanaHistorial = new historial();
		ventanaHistorial->setAttribute(Qt::WA_DeleteOnClose);
		ventanaHistorial->show();
		});
}

cuenta::~cuenta()
{
}

void cuenta::actualizarDatos() {
	ui.lblUsuario->setText("Nombre de usuario: " + usuarioActual.nombre);
	ui.lblSaldo->setText("Saldo actual: " + QString::number(usuarioActual.saldo, 'f', 2));
	ui.lblId->setText("Mi id: " + QString::number(usuarioActual.id));
}

