#include "clases/MenuPrincipal.h"

#include "ui_MenuPrincipal.h"
#include "clases/menuPaginacion.h"

#include <QMessageBox>

MenuPrincipal::MenuPrincipal(const QString& usuario, bool esAdmin, QWidget* parent)
	: QWidget(parent), usuario_(usuario),
	  ui(new Ui::MenuPrincipalClass)
{
	ui->setupUi(this);
	Q_UNUSED(esAdmin);
	// TODO: usar esAdmin para mostrar el botón de administrador.

	ui->lblUsuario->setText(QStringLiteral("Usuario: %1").arg(usuario_));
	conectarSenales();
}

void MenuPrincipal::conectarSenales()
{
	connect(ui->btnCatalogo, &QPushButton::clicked, this, &MenuPrincipal::abrirCatalogo);
	connect(ui->btnSalir, &QPushButton::clicked, this, &MenuPrincipal::salir);
}

MenuPrincipal::~MenuPrincipal()
{
	delete ui;
}

void MenuPrincipal::abrirCatalogo()
{
	abrirSubmenu(new MenuPaginacion());
}

void MenuPrincipal::salir()
{
	const auto respuesta = QMessageBox::question(
		this,
		QStringLiteral("Cerrar sesión"),
		QStringLiteral("¿Desea cerrar la sesión?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);

	if (respuesta == QMessageBox::Yes) {
		emit sesionCerrada();
		close();
	}
}

void MenuPrincipal::abrirSubmenu(QWidget* ventana)
{
	// Pendientes: Carrito y Administrador; el catálogo ya existe como MenuPaginacion.
	if (ventana == nullptr)
		return;

	setVisible(false);
	ventana->setAttribute(Qt::WA_DeleteOnClose);
	connect(ventana, &QObject::destroyed, this, [this]() {
		show();
	});
	ventana->show();
}
