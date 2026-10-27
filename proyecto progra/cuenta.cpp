#include "Cuenta.h"
#include "inicio.h"
#include "datos.h"
#include "cambio.h"
#include "seguro.h"

#include <QCloseEvent>

cuenta::cuenta(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	actualizarDatos();

	connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
		inicio* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
		ventanaInicio->show();
		this->close();   // cierra registro
		});

	connect(ui.btnDesactivar, &QPushButton::clicked, this, [this]() {
		seguro* ventanaSeguro = new seguro();
		ventanaSeguro->setWindowModality(Qt::ApplicationModal);
		ventanaSeguro->show();
	});

	connect(ui.btnCambiar, &QPushButton::clicked, this, [this]() {
		cambio* ventanaCambio = new cambio();
		ventanaCambio->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
		ventanaCambio->show();
		this->close();   // cierra cuenta
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

