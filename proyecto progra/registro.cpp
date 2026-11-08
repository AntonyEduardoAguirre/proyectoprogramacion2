// registro.cpp
#include "Registro.h"
#include "Inicio.h"
#include "cliente.h"
#include "datos.h"

#include <QCloseEvent>
#include <QPushButton>
#include <QList>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

registro::registro(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// La sombra suave no se puede definir en el .ui
	auto* sombra = new QGraphicsDropShadowEffect(ui.softCard);
	sombra->setBlurRadius(28);
	sombra->setOffset(0, 8);
	sombra->setColor(QColor(41, 42, 40, 22));
	ui.softCard->setGraphicsEffect(sombra);

	connect(ui.btnRegistrarse, &QPushButton::clicked,
		this, &registro::crearUsuario);

	connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
		inicio* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);
		ventanaInicio->show();
		this->close();
		});
}

registro::~registro()
{}

void registro::crearUsuario() {
	string nombre = ui.txtUsuario->text().toStdString();
	string contrasenna = ui.txtContrasenna->text().toStdString();

	if (nombre.empty() || contrasenna.empty()) { // Verificar si los campos están vacíos
		ui.lblError->setText("Por favor, complete todos los campos.");
		return;
	}
	else if (contrasenna.length() < 6 || contrasenna.length() > 20) {
		ui.lblError->setText("La nueva contraseña debe tener entre 6 y 20 caracteres");
		return;
	}
	else if (nombre.length() < 3 || nombre.length() > 20) {
		ui.lblError->setText("El nuevo nombre debe tener entre 3 y 20 caracteres");
		return;
	}
	for (const auto& usuario : clientes) { // Verificar si el usuario ya existe
		if (usuario.getNombre() == nombre) {
			ui.lblError->setText("El usuario ya existe.");	// Mostrar mensaje de error si el usuario ya existe
			return;
		}
	}


	Cliente nuevo;
	int idMaximo = 0;
	for (const auto& usuario : clientes) {
		if (usuario.getId() > idMaximo) {
			idMaximo = usuario.getId();
		}
	}

	nuevo.setNombre(ui.txtUsuario->text());
	nuevo.setContrasenna(ui.txtContrasenna->text());
	nuevo.setId(idMaximo + 1);
	nuevo.setSaldo(0.0f);


	clientes.push_back(nuevo);

	guardarClientes();

	ui.lblMensaje->setText("Usuario creado exitosamente.");

	
}