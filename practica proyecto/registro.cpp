// registro.cpp
#include "Registro.h"
#include "Inicio.h"
#include "cliente.h"
#include "datos.h"

#include <QCloseEvent>
#include <QPushButton>
#include <QList>

registro::registro(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	connect(ui.btnRegistrarse, &QPushButton::clicked,
		this, &registro::crearUsuario);

	connect(ui.btnRegresar, &QPushButton::clicked, this, [this]() {
		inicio* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
		ventanaInicio->show();
		ui.lblMensaje->setText("Usuario creado exitosamente.");
		this->close();   // cierra registro
		});
}

registro::~registro()
{}

void registro::crearUsuario() {
	string nombre = ui.txtUsuario->text().toStdString();
	string contrasenna = ui.txtContrasenna->text().toStdString();
	
	if (nombre.empty() || contrasenna.empty()) { // Verificar si los campos están vacíos
		ui.lblMensaje->setText("Por favor, complete todos los campos.");
		return;}
	for (const auto& usuario : clientes) { // Verificar si el usuario ya existe
		if (usuario.getNombre() == nombre) {
			ui.lblMensaje->setText("El usuario ya existe.");	// Mostrar mensaje de error si el usuario ya existe
			return;}
	}
	Cliente nuevo;
	nuevo.setNombre(ui.txtUsuario->text());
	nuevo.setContrasenna(ui.txtContrasenna->text());
	clientes.push_back(nuevo);
	guardarClientes();

	ui.lblMensaje->setText("Usuario creado exitosamente.");

	
}