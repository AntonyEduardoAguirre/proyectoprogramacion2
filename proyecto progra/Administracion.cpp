#include "clases/Administracion.h"
#include "clases/datos.h"
#include "clases/Inicio.h"
#include "ui_AdminCobros.h"
#include "ui_AdminCuentas.h"
#include "ui_AdminCuotas.h"
#include "ui_AdminProductos.h"
#include "ui_Administracion.h"
#include "ui_DetalleCobros.h"
#include "ui_DialogoCuenta.h"
#include "ui_DialogoProducto.h"

#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QSet>
#include <QStyle>
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

struct CobroRegistro
{
	QString cliente;
	QString fecha;
	double monto = 0.0;
	bool esCuota = false;
	int indiceRegistro = -1;
	int indicePago = -1;
};

QString nombreCliente(int id)
{
	for (const Cliente& cliente : clientes)
		if (cliente.getId() == id)
			return cliente.nombre;
	return QStringLiteral("Cliente %1").arg(id);
}

QSet<int> cuotasPagadas(const QJsonObject& registro)
{
	QSet<int> pagadas;
	const int meses = registro.value(QStringLiteral("meses")).toInt();
	if (!registro.value(QStringLiteral("pago_inicial_eliminado")).toBool())
		pagadas.insert(0);
	const QJsonArray pagos = registro.value(QStringLiteral("pagos")).toArray();
	for (int i = 0; i < pagos.size(); ++i) {
		const QJsonObject pago = pagos.at(i).toObject();
		const int numero = pago.contains(QStringLiteral("cuota"))
			? pago.value(QStringLiteral("cuota")).toInt(-1) : i + 1;
		if (numero >= 0 && numero < meses)
			pagadas.insert(numero);
	}
	return pagadas;
}

void normalizarPagosCuotas(QJsonObject& registro)
{
	QJsonArray pagos = registro.value(QStringLiteral("pagos")).toArray();
	for (int i = 0; i < pagos.size(); ++i) {
		QJsonObject pago = pagos.at(i).toObject();
		if (!pago.contains(QStringLiteral("cuota")))
			pago.insert(QStringLiteral("cuota"), i + 1);
		pagos.replace(i, pago);
	}
	registro.insert(QStringLiteral("pagos"), pagos);
}

QList<CobroRegistro> recolectarCobros(QString* error = nullptr)
{
	QList<CobroRegistro> cobros;
	for (int i = 0; i < historialCompras.size(); ++i) {
		const Compra& compra = historialCompras[i];
		if (compra.meses == 0)
			cobros.append({ nombreCliente(compra.clienteId), compra.fecha, compra.total, false, i, -1 });
	}

	QJsonArray registros;
	if (!cargarRegistrosCuotas(registros, error))
		return cobros;
	for (int i = 0; i < registros.size(); ++i) {
		const QJsonObject registro = registros.at(i).toObject();
		const QString cliente = registro.value(QStringLiteral("cliente")).toString().trimmed();
		const QJsonArray pagos = registro.value(QStringLiteral("pagos")).toArray();
		bool contieneCuotaInicial = false;
		for (int j = 0; j < pagos.size(); ++j) {
			const QJsonObject pago = pagos.at(j).toObject();
			const int cuota = pago.contains(QStringLiteral("cuota"))
				? pago.value(QStringLiteral("cuota")).toInt(-1) : j + 1;
			if (cuota == 0)
				contieneCuotaInicial = true;
		}
		if (!cliente.isEmpty() && !registro.value(QStringLiteral("pago_inicial_eliminado")).toBool()
			&& !contieneCuotaInicial) {
			cobros.append({ cliente, registro.value(QStringLiteral("fecha")).toString(),
				registro.value(QStringLiteral("pago_mensual")).toDouble(), true, i, -1 });
		}
		for (int j = 0; j < pagos.size(); ++j) {
			const QJsonObject pago = pagos.at(j).toObject();
			if (!cliente.isEmpty())
				cobros.append({ cliente, pago.value(QStringLiteral("fecha")).toString(),
					pago.value(QStringLiteral("monto")).toDouble(), true, i, j });
		}
	}
	return cobros;
}

QDateTime fechaDesdeRegistro(const QString& fecha)
{
	QDateTime fechaHora = QDateTime::fromString(fecha, QStringLiteral("dd/MM/yyyy hh:mm"));
	if (!fechaHora.isValid())
		fechaHora = QDateTime::fromString(fecha, QStringLiteral("dd/MM/yyyy hh:mm:ss"));
	return fechaHora;
}

void limpiarLayout(QVBoxLayout* layout)
{
	while (QLayoutItem* item = layout->takeAt(0)) {
		if (QWidget* widget = item->widget())
			delete widget;
		delete item;
	}
}

QDialog* dialogoProducto(QWidget* parent, int indice, QString& codigo, QString& nombre, double& precio)
{
	auto* dialogo = new QDialog(parent);
	Ui::DialogoProducto formulario;
	formulario.setupUi(dialogo);
	dialogo->setWindowTitle(indice < 0 ? QStringLiteral("Agregar producto") : QStringLiteral("Editar producto"));
	dialogo->setStyleSheet(estilo);
	const Producto* producto = indice >= 0 ? &productos[indice] : nullptr;
	formulario.codigoProducto->setText(producto == nullptr ? QString() : producto->codigo);
	formulario.nombreProducto->setText(producto == nullptr ? QString() : producto->nombre);
	formulario.precioProducto->setText(producto == nullptr ? QString() : QString::number(producto->precio, 'f', 2));
	formulario.codigoProducto->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]*")), formulario.codigoProducto));
	formulario.precioProducto->setValidator(new QDoubleValidator(0.0, 1000000000.0, 2, formulario.precioProducto));
	auto* codigoEdit = formulario.codigoProducto;
	auto* nombreEdit = formulario.nombreProducto;
	auto* precioEdit = formulario.precioProducto;
	auto* error = formulario.error;
	QObject::connect(formulario.botones, &QDialogButtonBox::rejected, dialogo, &QDialog::reject);
	QObject::connect(formulario.botones, &QDialogButtonBox::accepted, dialogo,
		[dialogo, indice, codigoEdit, nombreEdit, precioEdit, error, &codigo, &nombre, &precio]() {
			const QString nuevoCodigo = codigoEdit->text().trimmed();
			if (!Producto::codigoValido(nuevoCodigo)) {
				error->setText(QStringLiteral("El código debe contener al menos 3 dígitos."));
				return;
			}
			for (int i = 0; i < productos.size(); ++i) {
				if (i != indice && productos[i].codigo == nuevoCodigo) {
					error->setText(QStringLiteral("El código ya está asignado a otro producto."));
					return;
				}
			}
			bool valido = false;
			const double valor = precioEdit->text().toDouble(&valido);
			if (nombreEdit->text().trimmed().isEmpty() || !valido || valor <= 0) {
				error->setText(QStringLiteral("El nombre no puede estar vacío y el precio debe ser mayor que 0."));
				return;
			}
			codigo = nuevoCodigo;
			nombre = nombreEdit->text().trimmed();
			precio = valor;
			dialogo->accept();
		});
	return dialogo;
}
}

Administracion::Administracion(QWidget* parent)
	: QWidget(parent),
	  ui(new Ui::Administracion),
	  uiProductos_(new Ui::AdminProductos),
	  uiCuentas_(new Ui::AdminCuentas),
	  uiCuotas_(new Ui::AdminCuotas),
	  uiCobros_(new Ui::AdminCobros),
	  listaProductos_(nullptr), listaCuentas_(nullptr), listaCuotas_(nullptr), listaCobros_(nullptr)
{
	ui->setupUi(this);
	setStyleSheet(estilo);
	ui->regresar->setProperty("flat", true);
	ui->paginas->addWidget(crearProductos());
	ui->paginas->addWidget(crearCuentas());
	ui->paginas->addWidget(crearCuotas());
	ui->paginas->addWidget(crearCobros());
	registrarActualizacionPagos(this, [this]() {
		actualizarCuotas();
		actualizarCobros();
	});
	connect(ui->editarProductos, &QPushButton::clicked, this, [this]() {
		actualizarProductos();
		ui->paginas->setCurrentIndex(0);
	});
	connect(ui->editarCuentas, &QPushButton::clicked, this, [this]() {
		actualizarCuentas();
		ui->paginas->setCurrentIndex(1);
	});
	connect(ui->verCuotas, &QPushButton::clicked, this, [this]() {
		actualizarCuotas();
		ui->paginas->setCurrentIndex(2);
	});
	connect(ui->verCobros, &QPushButton::clicked, this, [this]() {
		actualizarCobros();
		ui->paginas->setCurrentIndex(3);
	});
	connect(ui->regresar, &QPushButton::clicked, this, [this]() {
		auto* ventanaInicio = new inicio();
		ventanaInicio->setAttribute(Qt::WA_DeleteOnClose);
		ventanaInicio->show();
		close();
	});
	actualizarProductos();
}

Administracion::~Administracion()
{
	delete ui;
	delete uiProductos_;
	delete uiCuentas_;
	delete uiCuotas_;
	delete uiCobros_;
}

QWidget* Administracion::crearProductos()
{
	auto* pagina = new QWidget(this);
	uiProductos_->setupUi(pagina);
	listaProductos_ = uiProductos_->listaProductosLayout;
	uiProductos_->codigoProducto->setValidator(new QRegularExpressionValidator(
		QRegularExpression(QStringLiteral("[0-9]*")), uiProductos_->codigoProducto));
	uiProductos_->precioProducto->setValidator(new QDoubleValidator(0.0, 1000000000.0, 2, uiProductos_->precioProducto));
	connect(uiProductos_->agregarProducto, &QPushButton::clicked, this, [this]() {
		const QString codigo = uiProductos_->codigoProducto->text().trimmed();
		if (!Producto::codigoValido(codigo)) {
			uiProductos_->errorProductos->setText(QStringLiteral("El código debe contener al menos 3 dígitos."));
			return;
		}
		for (const Producto& existente : productos) {
			if (existente.codigo == codigo) {
				uiProductos_->errorProductos->setText(QStringLiteral("El código ya está asignado a otro producto."));
				return;
			}
		}
		bool valido = false;
		const double valor = uiProductos_->precioProducto->text().toDouble(&valido);
		if (uiProductos_->nombreProducto->text().trimmed().isEmpty() || !valido || valor <= 0) {
			uiProductos_->errorProductos->setText(QStringLiteral("El nombre no puede estar vacío y el precio debe ser mayor que 0."));
			return;
		}
		Producto producto;
		producto.codigo = codigo;
		producto.nombre = uiProductos_->nombreProducto->text().trimmed();
		producto.precio = valor;
		for (const Producto& existente : productos)
			producto.id = qMax(producto.id, existente.id);
		++producto.id;
		productos.append(producto);
		guardarProductos();
		uiProductos_->codigoProducto->clear();
		uiProductos_->nombreProducto->clear();
		uiProductos_->precioProducto->clear();
		uiProductos_->errorProductos->clear();
		actualizarProductos();
	});
	return pagina;
}

void Administracion::actualizarProductos()
{
	if (!listaProductos_)
		return;
	limpiarLayout(listaProductos_);
	for (int i = 0; i < productos.size(); ++i) {
		auto* fila = new QFrame(uiProductos_->contenidoListaProductos);
		auto* layout = new QHBoxLayout(fila);
		auto* etiqueta = new QLabel(QStringLiteral("%1 — Cód. %2 — $%3")
			.arg(productos[i].nombre, productos[i].codigo)
			.arg(productos[i].precio, 0, 'f', 2), fila);
		etiqueta->setWordWrap(true);
		layout->addWidget(etiqueta, 1);
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
	QString codigo;
	QString nombre;
	double precio = 0.0;
	QDialog* dialogo = dialogoProducto(this, indice, codigo, nombre, precio);
	if (dialogo->exec() == QDialog::Accepted) {
		productos[indice].codigo = codigo;
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
    uiCuentas_->setupUi(pagina);
    listaCuentas_ = uiCuentas_->listaCuentasLayout;
    connect(uiCuentas_->crearUsuario, &QPushButton::clicked, this, [this]() {
        auto* dialogo = new QDialog(this);
        Ui::DialogoCuenta formulario;
        formulario.setupUi(dialogo);
        dialogo->setWindowTitle(QStringLiteral("Crear usuario"));
        dialogo->setStyleSheet(estilo);
        auto* nombre = formulario.nombreUsuario;
        auto* contrasenna = formulario.contrasennaUsuario;
        auto* error = formulario.error;
        connect(formulario.botones, &QDialogButtonBox::rejected, dialogo, &QDialog::reject);
        connect(formulario.botones, &QDialogButtonBox::accepted, dialogo,
            [this, dialogo, nombre, contrasenna, error]() {
                const QString nuevoNombre = nombre->text().trimmed();
                const QString nuevaContrasenna = contrasenna->text();
                if (nuevoNombre.isEmpty() || nuevaContrasenna.trimmed().isEmpty()) {
                    error->setText(QStringLiteral("El nombre y la contraseña no pueden estar vacíos."));
                    return;
                }
                if (nuevoNombre.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0) {
                    error->setText(QStringLiteral("Ese nombre de usuario ya existe."));
                    return;
                }
                for (const Cliente& cliente : clientes) {
                    if (cliente.nombre.compare(nuevoNombre, Qt::CaseInsensitive) == 0) {
                        error->setText(QStringLiteral("El nombre de usuario ya existe."));
                        return;
                    }
                }
                int idMaximo = 0;
                for (const Cliente& cliente : clientes)
                    idMaximo = qMax(idMaximo, cliente.getId());
                Cliente cliente;
                cliente.setId(idMaximo + 1);
                cliente.setNombre(nuevoNombre);
                cliente.setContrasenna(nuevaContrasenna);
                clientes.append(cliente);
                guardarClientes();
                dialogo->accept();
            });
        if (dialogo->exec() == QDialog::Accepted)
            actualizarCuentas();
        delete dialogo;
    });
    return pagina;
}

void Administracion::actualizarCuentas()
{
    if (!listaCuentas_)
        return;
    limpiarLayout(listaCuentas_);
    for (int i = 0; i < clientes.size(); ++i) {
        auto* fila = new QFrame(uiCuentas_->contenidoListaCuentas);
        auto* layout = new QHBoxLayout(fila);
        auto* usuario = new QLabel(clientes[i].nombre, fila);
        layout->addWidget(usuario, 1);
        auto* editar = new QPushButton(QStringLiteral("Editar"), fila);
        auto* eliminar = new QPushButton(fila);
        eliminar->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
        eliminar->setToolTip(QStringLiteral("Eliminar cuenta"));
        layout->addWidget(editar);
        layout->addWidget(eliminar);
        connect(editar, &QPushButton::clicked, this, [this, i]() { editarCuenta(i); });
        connect(eliminar, &QPushButton::clicked, this, [this, i]() { eliminarCuenta(i); });
        listaCuentas_->addWidget(fila);
    }
    listaCuentas_->addStretch();
}

void Administracion::editarCuenta(int indice)
{
    if (indice < 0 || indice >= clientes.size())
        return;
    auto* dialogo = new QDialog(this);
    Ui::DialogoCuenta formulario;
    formulario.setupUi(dialogo);
    dialogo->setWindowTitle(QStringLiteral("Editar cuenta"));
    dialogo->setStyleSheet(estilo);
    formulario.nombreUsuario->setText(clientes[indice].nombre);
    formulario.contrasennaUsuario->setText(clientes[indice].contrasenna);
    auto* nombre = formulario.nombreUsuario;
    auto* contrasenna = formulario.contrasennaUsuario;
    auto* error = formulario.error;
    connect(formulario.botones, &QDialogButtonBox::rejected, dialogo, &QDialog::reject);
    connect(formulario.botones, &QDialogButtonBox::accepted, dialogo,
        [this, indice, dialogo, nombre, contrasenna, error]() {
            const QString nuevoNombre = nombre->text().trimmed();
            const QString nuevaContrasenna = contrasenna->text();
            if (nuevoNombre.isEmpty() || nuevaContrasenna.trimmed().isEmpty()) {
                error->setText(QStringLiteral("El nombre y la contraseña no pueden estar vacíos."));
                return;
            }
            if (nuevoNombre.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0) {
                error->setText(QStringLiteral("Ese nombre de usuario está reservado."));
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

void Administracion::eliminarCuenta(int indice)
{
    if (indice < 0 || indice >= clientes.size())
        return;
    const QString nombre = clientes[indice].nombre;
    if (QMessageBox::question(this, QStringLiteral("Eliminar cuenta"),
        QStringLiteral("¿Desea eliminar la cuenta de %1?").arg(nombre)) != QMessageBox::Yes)
        return;
    clientes.removeAt(indice);
    guardarClientes();
    actualizarCuentas();
}

QWidget* Administracion::crearCuotas()
{
    auto* pagina = new QWidget(this);
    uiCuotas_->setupUi(pagina);
    listaCuotas_ = uiCuotas_->listaCuotasLayout;
    return pagina;
}

QWidget* Administracion::crearCobros()
{
    auto* pagina = new QWidget(this);
    uiCobros_->setupUi(pagina);
    listaCobros_ = uiCobros_->listaCobrosLayout;
    return pagina;
}

void Administracion::actualizarCuotas()
{
    if (!listaCuotas_)
        return;
    limpiarLayout(listaCuotas_);
    uiCuotas_->errorCuotas->clear();
    QJsonArray registros;
    QString error;
    if (!cargarRegistrosCuotas(registros, &error)) {
        uiCuotas_->errorCuotas->setText(QStringLiteral("No se pudieron cargar las cuotas: %1").arg(error));
        return;
    }
    int cantidad = 0;
    for (int i = 0; i < registros.size(); ++i) {
        const QJsonObject registro = registros.at(i).toObject();
        const int meses = registro.value(QStringLiteral("meses")).toInt();
        if (meses != 3 && meses != 6 && meses != 12 && meses != 24)
            continue;
        ++cantidad;
        const QString cliente = registro.value(QStringLiteral("cliente")).toString();
        const double total = registro.value(QStringLiteral("total_con_aumento")).toDouble();
        const double pagoMensual = registro.value(QStringLiteral("pago_mensual")).toDouble();
        const QSet<int> pagadas = cuotasPagadas(registro);
        auto* tarjeta = new QFrame(uiCuotas_->contenidoListaCuotas);
        auto* tarjetaLayout = new QVBoxLayout(tarjeta);
        auto* filaTitulo = new QHBoxLayout;
        filaTitulo->addWidget(new QLabel(QStringLiteral("%1 — $%2").arg(cliente).arg(total, 0, 'f', 2), tarjeta), 1);
        auto* desplegar = new QPushButton(QStringLiteral("Ver cuotas"), tarjeta);
        filaTitulo->addWidget(desplegar);
        auto* eliminar = new QPushButton(tarjeta);
        eliminar->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
        eliminar->setToolTip(QStringLiteral("Eliminar plan de cuotas"));
        filaTitulo->addWidget(eliminar);
        connect(eliminar, &QPushButton::clicked, this, [this, i]() { eliminarCuota(i); });
        tarjetaLayout->addLayout(filaTitulo);
        auto* detalle = new QFrame(tarjeta);
        auto* detalleLayout = new QVBoxLayout(detalle);
        detalleLayout->addWidget(new QLabel(QStringLiteral("Monto total: $%1").arg(total, 0, 'f', 2), detalle));
        auto* scrollDetalle = new QScrollArea(detalle);
        scrollDetalle->setWidgetResizable(true);
        scrollDetalle->setMaximumHeight(360);
        auto* contenidoCuotas = new QWidget(scrollDetalle);
        auto* filasCuotas = new QVBoxLayout(contenidoCuotas);
        for (int cuota = 0; cuota < meses; ++cuota) {
            auto* fila = new QFrame(contenidoCuotas);
            auto* filaLayout = new QHBoxLayout(fila);
            const bool estaPagada = pagadas.contains(cuota);
            filaLayout->addWidget(new QLabel(QStringLiteral("Cuota %1").arg(cuota + 1), fila), 1);
            filaLayout->addWidget(new QLabel(estaPagada ? QStringLiteral("Pagada") : QStringLiteral("Pendiente"), fila));
            filaLayout->addWidget(new QLabel(QStringLiteral("$%1").arg(pagoMensual, 0, 'f', 2), fila));
            auto* pagar = new QPushButton(estaPagada ? QStringLiteral("Pagada") : QStringLiteral("Pagar"), fila);
            pagar->setEnabled(!estaPagada);
            filaLayout->addWidget(pagar);
            if (!estaPagada)
                connect(pagar, &QPushButton::clicked, this, [this, i, cuota]() { pagarCuota(i, cuota); });
            filasCuotas->addWidget(fila);
        }
        filasCuotas->addStretch();
        scrollDetalle->setWidget(contenidoCuotas);
        detalleLayout->addWidget(scrollDetalle);
        detalleLayout->addWidget(new QLabel(QStringLiteral("Monto pagado hasta ahora: $%1")
            .arg(pagoMensual * pagadas.size(), 0, 'f', 2), detalle), 0, Qt::AlignRight);
        detalle->hide();
        tarjetaLayout->addWidget(detalle);
        connect(desplegar, &QPushButton::clicked, this, [detalle, desplegar]() {
            const bool visible = !detalle->isVisible();
            detalle->setVisible(visible);
            desplegar->setText(visible ? QStringLiteral("Ocultar cuotas") : QStringLiteral("Ver cuotas"));
        });
        listaCuotas_->addWidget(tarjeta);
    }
    if (cantidad == 0)
        listaCuotas_->addWidget(new QLabel(QStringLiteral("No hay compras a cuotas registradas."), uiCuotas_->contenidoListaCuotas));
    listaCuotas_->addStretch();
}

void Administracion::eliminarCuota(int indiceRegistro)
{
    QJsonArray registros;
    QString error;
    if (!cargarRegistrosCuotas(registros, &error)) {
        QMessageBox::warning(this, QStringLiteral("Eliminar cuotas"), QStringLiteral("No se pudieron cargar las cuotas: %1").arg(error));
        return;
    }
    if (indiceRegistro < 0 || indiceRegistro >= registros.size())
        return;
    const QString cliente = registros.at(indiceRegistro).toObject().value(QStringLiteral("cliente")).toString();
    if (QMessageBox::question(this, QStringLiteral("Eliminar plan de cuotas"),
        QStringLiteral("¿Eliminar el plan completo de cuotas de %1 y todos sus pagos?").arg(cliente)) != QMessageBox::Yes)
        return;
    registros.removeAt(indiceRegistro);
    if (!guardarRegistrosCuotas(registros, &error)) {
        QMessageBox::warning(this, QStringLiteral("Eliminar cuotas"), QStringLiteral("No se pudo guardar la eliminación: %1").arg(error));
        return;
    }
    actualizarCuotas();
    actualizarCobros();
}

void Administracion::pagarCuota(int indiceRegistro, int indiceCuota)
{
    QJsonArray registros;
    QString error;
    if (!cargarRegistrosCuotas(registros, &error)) {
        QMessageBox::warning(this, QStringLiteral("Pago de cuota"), QStringLiteral("No se pudieron cargar las cuotas: %1").arg(error));
        return;
    }
    if (indiceRegistro < 0 || indiceRegistro >= registros.size())
        return;
    QJsonObject registro = registros.at(indiceRegistro).toObject();
    const int meses = registro.value(QStringLiteral("meses")).toInt();
    if (indiceCuota < 0 || indiceCuota >= meses)
        return;
    normalizarPagosCuotas(registro);
    if (cuotasPagadas(registro).contains(indiceCuota))
        return;
    const double monto = registro.value(QStringLiteral("pago_mensual")).toDouble();
    if (QMessageBox::question(this, QStringLiteral("Pagar cuota"),
        QStringLiteral("¿Registrar el pago de $%1?").arg(monto, 0, 'f', 2)) != QMessageBox::Yes)
        return;
    QJsonArray pagos = registro.value(QStringLiteral("pagos")).toArray();
    QJsonObject pago;
    pago.insert(QStringLiteral("cuota"), indiceCuota);
    pago.insert(QStringLiteral("fecha"), QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy hh:mm")));
    pago.insert(QStringLiteral("monto"), monto);
    pagos.append(pago);
    registro.insert(QStringLiteral("pagos"), pagos);
    registro.insert(QStringLiteral("meses_restantes"), meses - cuotasPagadas(registro).size());
    registros.replace(indiceRegistro, registro);
    if (!guardarRegistrosCuotas(registros, &error)) {
        QMessageBox::warning(this, QStringLiteral("Pago de cuota"), QStringLiteral("No se pudo guardar el pago: %1").arg(error));
        return;
    }
    actualizarCuotas();
    actualizarCobros();
}

void Administracion::actualizarCobros()
{
    if (!listaCobros_)
        return;
    limpiarLayout(listaCobros_);
    uiCobros_->errorCobros->clear();
    cargarHistorial();
    QString error;
    const QList<CobroRegistro> cobros = recolectarCobros(&error);
    if (!error.isEmpty())
        uiCobros_->errorCobros->setText(QStringLiteral("No se pudieron cargar todos los pagos: %1").arg(error));
    QMap<QString, QMap<QString, QList<CobroRegistro>>> porUsuarioMes;
    QMap<QString, QString> nombresUsuarios;
    int fechasInvalidas = 0;
    for (const CobroRegistro& cobro : cobros) {
        const QDateTime fecha = fechaDesdeRegistro(cobro.fecha);
        if (!fecha.isValid()) {
            ++fechasInvalidas;
            continue;
        }
        const QString usuarioKey = cobro.cliente.toCaseFolded();
        const QString mes = fecha.toString(QStringLiteral("yyyy-MM"));
        porUsuarioMes[usuarioKey][mes].append(cobro);
        nombresUsuarios.insert(usuarioKey, cobro.cliente);
    }
    if (fechasInvalidas > 0)
        uiCobros_->errorCobros->setText(QStringLiteral("No se pudieron mostrar %1 cobros porque su fecha guardada no es válida.").arg(fechasInvalidas));
    const QLocale locale(QLocale::Spanish);
    for (auto usuario = porUsuarioMes.cbegin(); usuario != porUsuarioMes.cend(); ++usuario) {
        auto* tarjetaUsuario = new QFrame(uiCobros_->contenidoListaCobros);
        auto* usuarioLayout = new QVBoxLayout(tarjetaUsuario);
        usuarioLayout->addWidget(new QLabel(nombresUsuarios.value(usuario.key()), tarjetaUsuario));
        for (auto mes = usuario.value().cbegin(); mes != usuario.value().cend(); ++mes) {
            const QDate fechaMes = QDate::fromString(mes.key() + QStringLiteral("-01"), QStringLiteral("yyyy-MM-dd"));
            double totalMes = 0.0;
            for (const CobroRegistro& cobro : mes.value())
                totalMes += cobro.monto;
            auto* filaMes = new QFrame(tarjetaUsuario);
            auto* filaLayout = new QHBoxLayout(filaMes);
            filaLayout->addWidget(new QLabel(QStringLiteral("%1 — $%2")
                .arg(locale.toString(fechaMes, QStringLiteral("MMMM yyyy"))).arg(totalMes, 0, 'f', 2), filaMes), 1);
            auto* abrir = new QPushButton(QStringLiteral("Abrir"), filaMes);
            filaLayout->addWidget(abrir);
            const QString usuarioKey = usuario.key();
            const QString mesKey = mes.key();
            connect(abrir, &QPushButton::clicked, this, [this, usuarioKey, mesKey]() { mostrarDetalleCobros(usuarioKey, mesKey); });
            usuarioLayout->addWidget(filaMes);
        }
        listaCobros_->addWidget(tarjetaUsuario);
    }
    if (porUsuarioMes.isEmpty() && fechasInvalidas == 0)
        listaCobros_->addWidget(new QLabel(QStringLiteral("Todavía no hay cobros registrados."), uiCobros_->contenidoListaCobros));
    listaCobros_->addStretch();
}

bool Administracion::eliminarCobro(bool esCuota, int indiceRegistro, int indicePago)
{
    if (QMessageBox::question(this, QStringLiteral("Eliminar cobro"),
        QStringLiteral("¿Desea eliminar este cobro de los datos guardados?")) != QMessageBox::Yes)
        return false;
    QString error;
    if (!esCuota) {
        cargarHistorial();
        if (indiceRegistro < 0 || indiceRegistro >= historialCompras.size())
            return false;
        historialCompras.removeAt(indiceRegistro);
        guardarHistorial();
        return true;
    }
    QJsonArray registros;
    if (!cargarRegistrosCuotas(registros, &error)) {
        QMessageBox::warning(this, QStringLiteral("Eliminar cobro"), QStringLiteral("No se pudieron cargar los pagos: %1").arg(error));
        return false;
    }
    if (indiceRegistro < 0 || indiceRegistro >= registros.size())
        return false;
    QJsonObject registro = registros.at(indiceRegistro).toObject();
    if (indicePago < 0) {
        registro.insert(QStringLiteral("pago_inicial_eliminado"), true);
    } else {
        normalizarPagosCuotas(registro);
        QJsonArray pagos = registro.value(QStringLiteral("pagos")).toArray();
        if (indicePago >= pagos.size())
            return false;
        pagos.removeAt(indicePago);
        registro.insert(QStringLiteral("pagos"), pagos);
    }
    const int meses = registro.value(QStringLiteral("meses")).toInt();
    registro.insert(QStringLiteral("meses_restantes"), meses - cuotasPagadas(registro).size());
    registros.replace(indiceRegistro, registro);
    if (!guardarRegistrosCuotas(registros, &error)) {
        QMessageBox::warning(this, QStringLiteral("Eliminar cobro"), QStringLiteral("No se pudo guardar la eliminación: %1").arg(error));
        return false;
    }
    return true;
}

void Administracion::mostrarDetalleCobros(const QString& usuarioKey, const QString& mes)
{
    QString error;
    const QList<CobroRegistro> todosLosCobros = recolectarCobros(&error);
    if (!error.isEmpty())
        QMessageBox::warning(this, QStringLiteral("Detalle de cobros"), QStringLiteral("No se pudieron cargar todos los pagos: %1").arg(error));
    QMap<QDate, QList<CobroRegistro>> porDia;
    double totalMes = 0.0;
    QString nombreMostrado = usuarioKey;
    for (const CobroRegistro& cobro : todosLosCobros) {
        if (cobro.cliente.toCaseFolded() != usuarioKey)
            continue;
        const QDateTime fecha = fechaDesdeRegistro(cobro.fecha);
        if (!fecha.isValid() || fecha.toString(QStringLiteral("yyyy-MM")) != mes)
            continue;
        porDia[fecha.date()].append(cobro);
        totalMes += cobro.monto;
        nombreMostrado = cobro.cliente;
    }
    auto* dialogo = new QDialog(this);
    Ui::DetalleCobros formulario;
    formulario.setupUi(dialogo);
    dialogo->setStyleSheet(estilo);
    formulario.tituloDetalle->setObjectName(QStringLiteral("titulo"));
    const QDate fechaMes = QDate::fromString(mes + QStringLiteral("-01"), QStringLiteral("yyyy-MM-dd"));
    formulario.mesDetalle->setText(QStringLiteral("%1 — %2").arg(nombreMostrado)
        .arg(QLocale(QLocale::Spanish).toString(fechaMes, QStringLiteral("MMMM yyyy"))));
    formulario.totalMes->setText(QStringLiteral("Ganancias del mes: $%1").arg(totalMes, 0, 'f', 2));
    for (auto dia = porDia.cbegin(); dia != porDia.cend(); ++dia) {
        auto* tarjetaDia = new QFrame(formulario.contenidoListaDetalle);
        auto* diaLayout = new QVBoxLayout(tarjetaDia);
        double totalDia = 0.0;
        for (const CobroRegistro& cobro : dia.value())
            totalDia += cobro.monto;
        diaLayout->addWidget(new QLabel(QStringLiteral("%1 — Ganancias del día: $%2")
            .arg(dia.key().toString(QStringLiteral("dd/MM/yyyy"))).arg(totalDia, 0, 'f', 2), tarjetaDia));
        for (const CobroRegistro& cobro : dia.value()) {
            const QDateTime fecha = fechaDesdeRegistro(cobro.fecha);
            auto* fila = new QFrame(tarjetaDia);
            auto* filaLayout = new QHBoxLayout(fila);
            filaLayout->addWidget(new QLabel(QStringLiteral("Cobro — %1").arg(fecha.time().toString(QStringLiteral("hh:mm"))), fila), 1);
            filaLayout->addWidget(new QLabel(QStringLiteral("$%1").arg(cobro.monto, 0, 'f', 2), fila));
            auto* eliminar = new QPushButton(fila);
            eliminar->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
            eliminar->setToolTip(QStringLiteral("Eliminar cobro"));
            connect(eliminar, &QPushButton::clicked, this, [this, dialogo, cobro]() {
                if (eliminarCobro(cobro.esCuota, cobro.indiceRegistro, cobro.indicePago))
                    dialogo->accept();
            });
            filaLayout->addWidget(eliminar);
            diaLayout->addWidget(fila);
        }
        formulario.listaDiasLayout->addWidget(tarjetaDia);
    }
    formulario.listaDiasLayout->addStretch();
    dialogo->exec();
    delete dialogo;
    actualizarCobros();
    actualizarCuotas();
}
