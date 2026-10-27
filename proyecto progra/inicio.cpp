#include "inicio.h"
#include "Registro.h"
#include "cliente.h"
#include "datos.h"
#include "Cuenta.h"

#include <QPushButton>
#include <QString>
#include <vector>
#include <QList>
using namespace std;


inicio::inicio(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	connect(ui.btnIniciarSesion, &QPushButton::clicked,
		this, &inicio::validarSesion);

	connect(ui.btnRegistrarse, &QPushButton::clicked, this, [this]() {
		registro* ventanaRegistro = new registro();
		ventanaRegistro->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
		ventanaRegistro->show();
		this->close();   // cierra inicio
		});

	connect(ui.btnSalir, &QPushButton::clicked, this, [this]() {
		this->close();   // cierra inicio
		});

}

inicio::~inicio()
{}


string convertirMinuscula(string texto) {//FUNCION AUXILIAR
	for (int i = 0;i < texto.size(); i++) {
		texto[i] = tolower(texto[i]);
	}
	return texto;
}

void inicio::validarSesion(){//FUNCION AUXILIAR
	string nombre = ui.txtNombre->text().toStdString();
	string contrasenna = ui.txtContrasenna->text().toStdString();

	for (const auto& usuario : clientes) {
		if (convertirMinuscula(usuario.getNombre()) == convertirMinuscula(nombre) && contrasenna == usuario.getContrasenna()) {
			usuarioActual = usuario; // Almacena el usuario actual en la variable global
			cuenta* ventanaMenu = new cuenta();
			ventanaMenu->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
			ventanaMenu->show();
			this->close();   // cierra inicio
			return;
		}
	}

	ui.lblError->setText("El nombre o la contraseña son incorrectos");
}