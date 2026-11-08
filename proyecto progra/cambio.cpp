#include "cambio.h"
#include "cuenta.h"
#include "datos.h"
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

	if (usuarioActual.getContrasenna() == nuevaContrasenna.toStdString()) {
		ui.lblError->setText("La nueva contraseña no puede ser igual a la actual");
		return;
	}
	else if(usuarioActual.getContrasenna() != antiguaContrasenna.toStdString()) {
		ui.lblError->setText("La contraseña actual es incorrecta");
		return;
	}
	else {
		if (!nuevoNombre.isEmpty()) {
			usuarioActual.setNombre(nuevoNombre);
		}
		if (!nuevaContrasenna.isEmpty()) {
			usuarioActual.setContrasenna(nuevaContrasenna);
		}
		if (nuevaContrasenna.isEmpty() && nuevoNombre.isEmpty()){
			ui.lblError->setText("No se han realizado cambios");
			return;
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

	
}