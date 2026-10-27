#include "seguro.h"
#include "inicio.h"
#include "datos.h"
#include "cambio.h"

#include <QCloseEvent>

seguro::seguro(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.btnCancelar, &QPushButton::clicked, this, [this]() {
		this->close();   // cierra seguro
		});

	connect(ui.btnConfirmar, &QPushButton::clicked, this, [this]() {
		desactivarCuenta();
		});

}

seguro::~seguro()
{
}


void seguro::desactivarCuenta() {
	// Eliminar el usuario actual de la lista de clientes
	for (int i = 0; i < clientes.size(); ++i) {
		if (clientes[i].getId() == usuarioActual.getId()) {
			clientes.erase(clientes.begin() + i);
			break;
		}
	}
	guardarClientes();
	inicio* ventanaInicio = new inicio();
	ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);

	for (QWidget* w : QApplication::topLevelWidgets())
		if (w != ventanaInicio)
			w->close();

	ventanaInicio->show();
}