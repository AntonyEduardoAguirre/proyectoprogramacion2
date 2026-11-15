#include "clases/Administracion.h"
#include "clases/datos.h"
#include "clases/Inicio.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace
{
const QString estilo = QStringLiteral(
	"QWidget { background-color: #F5F3EE; color: #292A28; font-family: 'Segoe UI'; }"
	"QFrame, QLineEdit { background-color: #FFFFFF; border: 1px solid #EAE8E2; border-radius: 8px; padding: 6px; }"
	"QLineEdit { min-height: 30px; }"
	"QPushButton { background-color: #899B83; color: white; border: none; border-radius: 8px; padding: 8px 14px; }"
	"QPushButton:hover { background-color: #788B72; }"
	"QPushButton[flat='true'] { background: transparent; color: #899B83; }"
	"QLabel#titulo { font-family: Georgia; font-size: 26px; }"
	"QLabel#error { color: #B05E5E; }");

QDialog* dialogoProducto(QWidget* parent, const Producto* producto, QString& nombre, double& precio)
{
	auto* dialogo = new QDialog(parent);
	dialogo->setWindowTitle(producto == nullptr ? QStringLiteral("Agregar producto") : QStringLiteral("Editar producto"));
	dialogo->setStyleSheet(estilo);

	auto* nombreEdit = new QLineEdit(producto == nullptr ? QString() : producto->nombre, dialogo);
	auto* precioEdit = new QLineEdit(producto == nullptr ? QString() : QString::number(producto->precio, 'f', 2), dialogo);
	precioEdit->setValidator(new QDoubleValidator(0.0, 1000000000.0, 2, precioEdit));
	auto* error = new QLabel(dialogo);
	error->setObjectName(QStringLiteral("error"));

	auto* formulario = new QFormLayout(dialogo);
	formulario->addRow(QStringLiteral("Nombre"), nombreEdit);
	formulario->addRow(QStringLiteral("Precio"), precioEdit);
	formulario->addRow(error);
	auto* botones = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, dialogo);
	formulario->addRow(botones);

	QObject::connect(botones, &QDialogButtonBox::rejected, dialogo, &QDialog::reject);
	QObject::connect(botones, &QDialogButtonBox::accepted, dialogo, [dialogo, nombreEdit, precioEdit, error, &nombre, &precio]() {
		bool valido = false;
		const double valor = precioEdit->text().toDouble(&valido);
		if (nombreEdit->text().trimmed().isEmpty() || !valido || valor <= 0) {
			error->setText(QStringLiteral("El nombre no puede estar vacío y el precio debe ser mayor que 0."));
			return;
		}
		nombre = nombreEdit->text().trimmed();
		precio = valor;
		dialogo->accept();
	});
	return dialogo;
}
}

AccesoAdmin::AccesoAdmin(QWidget* parent)
	: QWidget(parent), contrasenna_(new QLineEdit(this)), error_(new QLabel(this))
{
	setWindowTitle(QStringLiteral("Cuenta admin"));
	resize(430, 270);
	setStyleSheet(estilo);

	auto* titulo = new QLabel(QStringLiteral("Cuenta admin"), this);
	titulo->setObjectName(QStringLiteral("titulo"));
	auto* indicacion = new QLabel(QStringLiteral("Ingrese la contraseña para continuar."), this);
	contrasenna_->setEchoMode(QLineEdit::Password);
	contrasenna_->setPlaceholderText(QStringLiteral("Contraseña"));
	error_->setObjectName(QStringLiteral("error"));

	auto* entrar = new QPushButton(QStringLiteral("Entrar"), this);
	auto* regresar = new QPushButton(QStringLiteral("Regresar"), this);
	regresar->setProperty("flat", true);

	auto* contenido = new QVBoxLayout(this);
	contenido->setContentsMargins(42, 34, 42, 30);
	contenido->addWidget(titulo);
	contenido->addWidget(indicacion);
	contenido->addSpacing(20);
	contenido->addWidget(contrasenna_);
	contenido->addWidget(error_);
	contenido->addSpacing(12);
	contenido->addWidget(entrar);
	contenido->addWidget(regresar);
	contenido->addStretch();

	connect(entrar, &QPushButton::clicked, this, &AccesoAdmin::validarContrasenna);
	connect(contrasenna_, &QLineEdit::returnPressed, this, &AccesoAdmin::validarContrasenna);
	connect(regresar, &QPushButton::clicked, this, [this]() {
		auto* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);
		ventanaInicio->show();
		close();
	});
}

void AccesoAdmin::validarContrasenna()
{
	if (contrasenna_->text() != QStringLiteral("admin123")) {
		error_->setText(QStringLiteral("La contraseña es incorrecta."));
		return;
	}

	auto* ventana = new Administracion();
	ventana->setAttribute(Qt::WA_DeleteOnClose);
	ventana->show();
	close();
}

Administracion::Administracion(QWidget* parent)
	: QWidget(parent), paginas_(new QStackedWidget(this)), listaProductos_(nullptr), listaCuentas_(nullptr)
{
	setWindowTitle(QStringLiteral("Administración"));
	resize(850, 650);
	setStyleSheet(estilo);

	auto* titulo = new QLabel(QStringLiteral("Administración"), this);
	titulo->setObjectName(QStringLiteral("titulo"));
	auto* editarProductos = new QPushButton(QStringLiteral("Editar productos"), this);
	auto* editarCuentas = new QPushButton(QStringLiteral("Editar cuentas"), this);
	auto* regresar = new QPushButton(QStringLiteral("Cerrar"), this);
	regresar->setProperty("flat", true);

	paginas_->addWidget(crearProductos());
	paginas_->addWidget(crearCuentas());

	auto* menu = new QHBoxLayout;
	menu->addWidget(editarProductos);
	menu->addWidget(editarCuentas);
	menu->addStretch();
	menu->addWidget(regresar);

	auto* contenido = new QVBoxLayout(this);
	contenido->setContentsMargins(32, 28, 32, 28);
	contenido->addWidget(titulo);
	contenido->addLayout(menu);
	contenido->addWidget(paginas_);

	connect(editarProductos, &QPushButton::clicked, this, [this]() {
		actualizarProductos();
		paginas_->setCurrentIndex(0);
	});
	connect(editarCuentas, &QPushButton::clicked, this, [this]() {
		actualizarCuentas();
		paginas_->setCurrentIndex(1);
	});
	connect(regresar, &QPushButton::clicked, this, [this]() {
		auto* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);
		ventanaInicio->show();
		close();
	});
	actualizarProductos();
}

QWidget* Administracion::crearProductos()
{
	auto* pagina = new QWidget(this);
	listaProductos_ = new QVBoxLayout;
	listaProductos_->setAlignment(Qt::AlignTop);

	auto* scrollContenido = new QWidget(pagina);
	scrollContenido->setLayout(listaProductos_);
	auto* scroll = new QScrollArea(pagina);
	scroll->setWidgetResizable(true);
	scroll->setWidget(scrollContenido);

	auto* nombre = new QLineEdit(pagina);
	nombre->setPlaceholderText(QStringLiteral("Nombre del producto"));
	auto* precio = new QLineEdit(pagina);
	precio->setPlaceholderText(QStringLiteral("Precio"));
	precio->setValidator(new QDoubleValidator(0.0, 1000000000.0, 2, precio));
	auto* error = new QLabel(pagina);
	error->setObjectName(QStringLiteral("error"));
	auto* agregar = new QPushButton(QStringLiteral("Agregar producto"), pagina);

	auto* formulario = new QHBoxLayout;
	formulario->addWidget(nombre);
	formulario->addWidget(precio);
	formulario->addWidget(agregar);
	auto* layout = new QVBoxLayout(pagina);
	layout->addWidget(new QLabel(QStringLiteral("Productos existentes"), pagina));
	layout->addWidget(scroll);
	layout->addWidget(new QLabel(QStringLiteral("Nuevo producto"), pagina));
	layout->addLayout(formulario);
	layout->addWidget(error);

	connect(agregar, &QPushButton::clicked, this, [this, nombre, precio, error]() {
		bool valido = false;
		const double valor = precio->text().toDouble(&valido);
		if (nombre->text().trimmed().isEmpty() || !valido || valor <= 0) {
			error->setText(QStringLiteral("El nombre no puede estar vacío y el precio debe ser mayor que 0."));
			return;
		}
		Producto producto;
		producto.nombre = nombre->text().trimmed();
		producto.precio = valor;
		int idMaximo = 0;
		for (const auto& existente : productos)
			idMaximo = qMax(idMaximo, existente.id);
		producto.id = idMaximo + 1;
		productos.append(producto);
		guardarProductos();
		nombre->clear();
		precio->clear();
		error->clear();
		actualizarProductos();
	});
	return pagina;
}

void Administracion::actualizarProductos()
{
	if (listaProductos_ == nullptr)
		return;
	while (auto* item = listaProductos_->takeAt(0)) {
		if (item->widget() != nullptr)
			delete item->widget();
		delete item;
	}
	for (int i = 0; i < productos.size(); ++i) {
		auto* fila = new QWidget(this);
		auto* layout = new QHBoxLayout(fila);
		layout->addWidget(new QLabel(QStringLiteral("%1 — $%2").arg(productos[i].nombre).arg(productos[i].precio, 0, 'f', 2), fila));
		layout->addStretch();
		auto* editar = new QPushButton(QStringLiteral("Editar"), fila);
		auto* eliminar = new QPushButton(QStringLiteral("Eliminar"), fila);
		layout->addWidget(editar);
		layout->addWidget(eliminar);
		connect(editar, &QPushButton::clicked, this, [this, i]() { editarProducto(i); });
		connect(eliminar, &QPushButton::clicked, this, [this, i]() { eliminarProducto(i); });
		listaProductos_->addWidget(fila);
	}
	listaProductos_->addStretch();
}

void Administracion::editarProducto(int indice)
{
	if (indice < 0 || indice >= productos.size())
		return;
	QString nombre;
	double precio = 0;
	auto* dialogo = dialogoProducto(this, &productos[indice], nombre, precio);
	if (dialogo->exec() == QDialog::Accepted) {
		productos[indice].nombre = nombre;
		productos[indice].precio = precio;
		guardarProductos();
		actualizarProductos();
	}
	delete dialogo;
}

void Administracion::eliminarProducto(int indice)
{
	if (indice < 0 || indice >= productos.size())
		return;
	if (QMessageBox::question(this, QStringLiteral("Eliminar producto"), QStringLiteral("¿Desea eliminar este producto?")) != QMessageBox::Yes)
		return;
	productos.removeAt(indice);
	guardarProductos();
	actualizarProductos();
}

QWidget* Administracion::crearCuentas()
{
    auto* pagina = new QWidget(this);
    listaCuentas_ = new QVBoxLayout;
    listaCuentas_->setAlignment(Qt::AlignTop);

    auto* scrollContenido = new QWidget(pagina);
    scrollContenido->setLayout(listaCuentas_);
    auto* scroll = new QScrollArea(pagina);
    scroll->setWidgetResizable(true);
    scroll->setWidget(scrollContenido);

    auto* layout = new QVBoxLayout(pagina);
    layout->addWidget(new QLabel(QStringLiteral("Usuarios registrados"), pagina));
    layout->addWidget(scroll);
    return pagina;
}

void Administracion::actualizarCuentas()
{
    if (listaCuentas_ == nullptr)
        return;
    while (auto* item = listaCuentas_->takeAt(0)) {
        if (item->widget() != nullptr)
            delete item->widget();
        delete item;
    }
    for (int i = 0; i < clientes.size(); ++i) {
        auto* usuario = new QPushButton(clientes[i].nombre, this);
        usuario->setProperty("flat", true);
        connect(usuario, &QPushButton::clicked, this, [this, i]() { editarCuenta(i); });
        listaCuentas_->addWidget(usuario);
    }
    listaCuentas_->addStretch();
}

void Administracion::editarCuenta(int indice)
{
    if (indice < 0 || indice >= clientes.size())
        return;

    auto* dialogo = new QDialog(this);
    dialogo->setWindowTitle(QStringLiteral("Editar cuenta"));
    dialogo->setStyleSheet(estilo);
    auto* nombre = new QLineEdit(clientes[indice].nombre, dialogo);
    auto* contrasenna = new QLineEdit(clientes[indice].contrasenna, dialogo);
    contrasenna->setEchoMode(QLineEdit::Password);
    auto* error = new QLabel(dialogo);
    error->setObjectName(QStringLiteral("error"));
    auto* botones = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, dialogo);
    auto* formulario = new QFormLayout(dialogo);
    formulario->addRow(QStringLiteral("Nombre de usuario"), nombre);
    formulario->addRow(QStringLiteral("Contraseña"), contrasenna);
    formulario->addRow(error);
    formulario->addRow(botones);

    connect(botones, &QDialogButtonBox::rejected, dialogo, &QDialog::reject);
    connect(botones, &QDialogButtonBox::accepted, dialogo, [this, indice, dialogo, nombre, contrasenna, error]() {
        const QString nuevoNombre = nombre->text().trimmed();
        const QString nuevaContrasenna = contrasenna->text();
        if (nuevoNombre.isEmpty() || nuevaContrasenna.trimmed().isEmpty()) {
            error->setText(QStringLiteral("El nombre y la contraseña no pueden estar vacíos."));
            return;
        }
        for (int i = 0; i < clientes.size(); ++i) {
            if (i != indice && clientes[i].nombre.compare(nuevoNombre, Qt::CaseInsensitive) == 0) {
                error->setText(QStringLiteral("El nuevo nombre de usuario ya está repetido."));
                return;
            }
        }
        clientes[indice].nombre = nuevoNombre;
        clientes[indice].contrasenna = nuevaContrasenna;
        guardarClientes();
        dialogo->accept();
    });

    if (dialogo->exec() == QDialog::Accepted)
        actualizarCuentas();
    delete dialogo;
}
