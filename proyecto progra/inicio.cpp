#include "clases/Inicio.h"
#include "clases/cliente.h"
#include "clases/datos.h"
#include "clases/MenuPrincipal.h"
#include "clases/Administracion.h"

#include <QPushButton>
#include <QString>
#include <QCheckBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <QList>
using namespace std;

inicio::inicio(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    auto* sombra = new QGraphicsDropShadowEffect(ui.loginCard);
    sombra->setBlurRadius(28);
    sombra->setOffset(0, 8);
    sombra->setColor(QColor(41, 42, 40, 22));
    ui.loginCard->setGraphicsEffect(sombra);

    connect(ui.btnIniciarSesion, &QPushButton::clicked,
        this, &inicio::validarSesion);
    connect(ui.btnCuentaAdmin, &QPushButton::clicked, this, [this]() {
        auto* ventanaAdmin = new AccesoAdmin();
        ventanaAdmin->setAttribute(Qt::WA_DeleteOnClose);
        ventanaAdmin->show();
        close();
        });

    connect(ui.btnSalir, &QPushButton::clicked, this, [this]() {
        this->close();
        });

}

inicio::~inicio()
{
}
string convertirMinuscula(string texto) {//FUNCION AUXILIAR
    for (int i = 0;i < texto.size(); i++) {
        texto[i] = tolower(texto[i]);
    }
    return texto;
}

void inicio::validarSesion() {//FUNCION AUXILIAR
    string nombre = ui.txtNombre->text().toStdString();
    string contrasenna = ui.txtContrasenna->text().toStdString();

    for (const auto& usuario : clientes) {
        if (convertirMinuscula(usuario.getNombre()) == convertirMinuscula(nombre) && contrasenna == usuario.getContrasenna()) {
            usuarioActual = usuario; // Almacena el usuario actual en la variable global
            // TODO: cambiar según el rol del usuario cuando se implemente la validación de administradores.
            MenuPrincipal* ventanaMenu = new MenuPrincipal(usuarioActual.nombre, false);
            ventanaMenu->setAttribute(Qt::WA_DeleteOnClose); // libera memoria al cerrarla
            ventanaMenu->show();
            this->close();   // cierra inicio
            return;
        }
    }

    ui.lblError->setText("El nombre o la contraseña son incorrectos");
}