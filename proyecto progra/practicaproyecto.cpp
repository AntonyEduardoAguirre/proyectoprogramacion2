#include "practicaproyecto.h"
#include <QPushButton>
#include <QString>
#include  <vector>
using namespace std;

vector<string> nombresRegistrados = { "usuario1", "usuario2", "usuario3" };
vector<string> contrasennasRegistrados = { "contrasenna1", "contrasenna2", "contrasenna3" };

practicaproyecto::practicaproyecto(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	connect(ui.btnIniciarSesion, &QPushButton::clicked,
		this, &practicaproyecto::validarSesion);

}

practicaproyecto::~practicaproyecto()
{}


string convertirMinuscula(string texto) {//FUNCION AUXILIAR
	for (int i = 0;i < texto.size(); i++) {
		texto[i] = tolower(texto[i]);
	}
	return texto;
}

bool verificarUsuario(string nombre, string contrasenna) {//FUNCION AUXILIAR
	for (int i = 0; i < nombresRegistrados.size(); i++) {
		if (nombresRegistrados[i] == convertirMinuscula(nombre) && contrasenna == contrasennasRegistrados[i]) {
			return true;
		}
	}
	return false;
}

bool practicaproyecto::validarSesion(){//FUNCION AUXILIAR
	string nombre = ui.txtNombre->text().toStdString();
	string contrasenna = ui.txtContrasenna->text().toStdString();

	if (verificarUsuario(nombre, contrasenna)){
		return true;
	}


	return false;
}