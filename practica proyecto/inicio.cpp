#include "inicio.h"
#include "Registro.h"
#include "cliente.h"
#include "datos.h"

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

	for (int i = 0; i < clientes.size(); i++) {
		if (convertirMinuscula(clientes[i].getNombre()) == convertirMinuscula(nombre) && contrasenna == clientes[i].getContrasenna()) {
			ui.lblNombre->setText("funciono");
			return;
		}
	}

	ui.lblNombre->setText("error");
}