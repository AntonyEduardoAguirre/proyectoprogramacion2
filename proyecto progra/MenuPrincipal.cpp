#include "MenuPrincipal.h"

#include "EstiloPulpe.h"
#include "VentanaCobro.h"
#include "Cuenta.h"
#include "placeholders/VentanaAdmin.h"
#include "placeholders/VentanaCarrito.h"
#include "placeholders/VentanaCatalogo.h"

#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

MenuPrincipal::MenuPrincipal(const QString& usuario, bool esAdmin, QWidget* parent)
	: QWidget(parent), usuario_(usuario), esAdmin_(esAdmin)
{
	setWindowTitle(QStringLiteral("La Pulpe - Menú principal"));
	setMinimumSize(560, 520);
	setObjectName(QStringLiteral("menuPrincipal"));
	setStyleSheet(EstiloPulpe::hojaEstilo());

	auto* tarjeta = new QFrame(this);
	tarjeta->setObjectName(QStringLiteral("tarjeta"));
	auto* sombra = new QGraphicsDropShadowEffect(tarjeta);
	sombra->setBlurRadius(28);
	sombra->setOffset(0, 8);
	sombra->setColor(QColor(41, 42, 40, 22));
	tarjeta->setGraphicsEffect(sombra);

	auto* titulo = new QLabel(QStringLiteral("La Pulpe"), tarjeta);
	titulo->setObjectName(QStringLiteral("tituloPrincipal"));

	auto* cuenta = new QPushButton(QStringLiteral("Cuenta"), tarjeta);
	cuenta->setProperty("rol", "secundario");
	auto* cobro = new QPushButton(QStringLiteral("Cobro / Vuelto"), tarjeta);
	cobro->setProperty("rol", "principal");
	auto* barraSuperior = new QHBoxLayout;
	barraSuperior->addWidget(cuenta);
	barraSuperior->addStretch();
	barraSuperior->addWidget(cobro);

	auto* catalogo = new QPushButton(QStringLiteral("Ver catálogo"), tarjeta);
	auto* carrito = new QPushButton(QStringLiteral("Ver carrito de compras"), tarjeta);
	auto* admin = new QPushButton(QStringLiteral("Página de administrador"), tarjeta);
	auto* salirButton = new QPushButton(QStringLiteral("Salir"), tarjeta);
	catalogo->setProperty("rol", "principal");
	carrito->setProperty("rol", "principal");
	admin->setProperty("rol", "principal");
	salirButton->setProperty("rol", "peligro");

	admin->setVisible(esAdmin_);

	auto* botones = new QVBoxLayout;
	botones->setSpacing(10);
	botones->addWidget(catalogo);
	botones->addWidget(carrito);
	botones->addWidget(admin);
	botones->addWidget(salirButton);

	auto* etiquetaUsuario = new QLabel(QStringLiteral("Usuario: %1").arg(usuario_), tarjeta);
	etiquetaUsuario->setObjectName(QStringLiteral("etiquetaUsuario"));

	auto* contenido = new QVBoxLayout(this);
	contenido->setContentsMargins(24, 24, 24, 24);
	contenido->addWidget(tarjeta);

	auto* contenidoTarjeta = new QVBoxLayout(tarjeta);
	contenidoTarjeta->setContentsMargins(42, 34, 42, 30);
	contenidoTarjeta->setSpacing(0);
	contenidoTarjeta->addWidget(titulo);
	contenidoTarjeta->addSpacing(22);
	contenidoTarjeta->addLayout(barraSuperior);
	contenidoTarjeta->addSpacing(30);
	contenidoTarjeta->addLayout(botones);
	contenidoTarjeta->addStretch();
	contenidoTarjeta->addWidget(etiquetaUsuario);

	connect(cuenta, &QPushButton::clicked, this, &MenuPrincipal::abrirCuenta);
	connect(cobro, &QPushButton::clicked, this, &MenuPrincipal::abrirCobro);
	connect(catalogo, &QPushButton::clicked, this, &MenuPrincipal::abrirCatalogo);
	connect(carrito, &QPushButton::clicked, this, &MenuPrincipal::abrirCarrito);
	connect(admin, &QPushButton::clicked, this, &MenuPrincipal::abrirAdmin);
	connect(salirButton, &QPushButton::clicked, this, &MenuPrincipal::salir);
}

void MenuPrincipal::abrirCuenta()
{
	abrirSubmenu(new cuenta());
}

void MenuPrincipal::abrirCobro()
{
	abrirSubmenu(new VentanaCobro(usuario_));
}

void MenuPrincipal::abrirCatalogo()
{
	abrirSubmenu(new VentanaCatalogo(usuario_));
}

void MenuPrincipal::abrirCarrito()
{
	abrirSubmenu(new VentanaCarrito(usuario_));
}

void MenuPrincipal::abrirAdmin()
{
	if (esAdmin_)
		abrirSubmenu(new VentanaAdmin(usuario_));
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
	if (ventana == nullptr)
		return;

	setVisible(false);
	ventana->setAttribute(Qt::WA_DeleteOnClose);
	connect(ventana, &QObject::destroyed, this, [this]() {
		show();
	});
	ventana->show();
}
