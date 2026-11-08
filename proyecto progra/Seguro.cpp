#include "seguro.h"
#include "inicio.h"
#include "datos.h"
#include "cambio.h"


#include <algorithm>
#include <QCloseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>


seguro::seguro(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_StyledBackground, true); // para que el QSS pinte el fondo

	connect(ui.btnCancelar, &QPushButton::clicked, this, [this]() {
		this->close();
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
	const int id = usuarioActual.getId();
	clientes.erase(
		std::remove_if(clientes.begin(), clientes.end(),
			[id](const Cliente& c) { return c.getId() == id; }),
		clientes.end());
	guardarClientes();


	inicio* ventanaInicio = new inicio();
	ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);

	for (QWidget* w : QApplication::topLevelWidgets())
		if (w != ventanaInicio)
			w->close();

	ventanaInicio->show();
}