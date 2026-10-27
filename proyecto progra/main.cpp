
#include "inicio.h"
#include "Cliente.h"
#include "Producto.h"
#include "datos.h"

#include <QCloseEvent>
#include <QtWidgets/QApplication>
#include <QList>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Estos dos nombres definen la carpeta de AppData; deben ir ANTES de cargar
    QCoreApplication::setOrganizationName("MiPractica");
    QCoreApplication::setApplicationName("practica proyecto");

    cargarClientes();
    // cargarProductos();

    inicio w;
    w.show();
    return a.exec();

    QApplication app(argc, argv);
    inicio window;
    window.show();
    return app.exec();
}
