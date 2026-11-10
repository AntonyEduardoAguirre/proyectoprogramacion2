#include "clases/Cambio.h"
#include "clases/Cuenta.h"
#include "clases/datos.h"
#include "iostream"
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

cambio::cambio(QWidget* parent)
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

	connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
		cuenta* ventanaCuenta = new cuenta();
		ventanaCuenta->setAttribute(Qt::WA_DeleteOnClose);
		ventanaCuenta->show();
		this->close();
		});

	connect(ui.btnConfirmar, &QPushButton::clicked, this, &cambio::cambiarDatos);
}

cambio::~cambio()
{
}

void cambio::cambiarDatos() {
	QString nuevoNombre = ui.txtNombre->text();
	QString nuevaContrasenna = ui.txtContrasenna->text();
	QString antiguaContrasenna = ui.txtOldContrasenna->text();



	if (usuarioActual.getContrasenna() != antiguaContrasenna.toStdString()) {
		ui.lblError->setText("La contraseña actual es incorrecta");
		return;
	}
	else if (usuarioActual.getContrasenna() == nuevaContrasenna.toStdString()) {
		ui.lblError->setText("La nueva contraseña no puede ser igual a la actual");
		return;
	}
	else if(nuevaContrasenna.length() < 6 || nuevaContrasenna.length() > 20) {
		ui.lblError->setText("La nueva contraseña debe tener entre 6 y 20 caracteres");
		return;
	}
	else if (nuevoNombre.length() < 3 || nuevoNombre.length() > 20) {
		ui.lblError->setText("El nuevo nombre debe tener entre 3 y 20 caracteres");
		return;
	}
	else if(nuevaContrasenna.isEmpty() && nuevoNombre.isEmpty()) {
		ui.lblError->setText("No se han realizado cambios");
		return;
	}

	for (const auto& usuario : clientes) { // Verificar si el usuario ya existe
		if (usuario.getNombre() == nuevoNombre) {
			ui.lblError->setText("Este nombre de usuario ya esta en uso");	// Mostrar mensaje de error si el usuario ya existe
			return;
		}
	}

	if (!nuevoNombre.isEmpty()) {
		usuarioActual.setNombre(nuevoNombre);
	}
	else if(!nuevaContrasenna.isEmpty()) {
		usuarioActual.setContrasenna(nuevaContrasenna);
	}

	else {
		for (int i = 0; i < clientes.size(); ++i) {
			if (clientes[i].getId() == usuarioActual.getId()) {
				clientes[i] = usuarioActual; // Actualiza el usuario en la lista de clientes
				guardarClientes();
				break;
			}
		}
		cuenta* ventanaCuenta = new cuenta();
		ventanaCuenta->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
		ventanaCuenta->show();
		this->close();   // cierra cambio
	}

	
}