#include "clases/MenuPrincipal.h"
#include "ui_MenuPrincipal.h"
#include "clases/FuenteCuotas.h"
#include "clases/FuenteHistorialVentas.h"
#include "clases/RepositorioBitacora.h"
#include "clases/RepositorioCuotas.h"
#include "clases/ServicioCierre.h"
#include "clases/VentanaCuotas.h"
#include "clases/datos.h"
#include "clases/menuPaginacion.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>

#include <memory>

namespace {

// Sello único con fecha y hora para los respaldos.
QString selloTiempo()
{
    return QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
}

bool copiarRespaldo(const QString& nombreArchivo, const QString& sello, QString* error)
{
    const QString origen = rutaArchivo(nombreArchivo);
    QFile archivo(origen);
    if (!archivo.exists())
        return true;   // no hay archivo que respaldar

    const QString carpeta = carpetaDatos() + QStringLiteral("/respaldo");
    if (!QDir().mkpath(carpeta)) {
        if (error)
            *error = QStringLiteral("No se pudo crear la carpeta de respaldo.");
        return false;
    }

    const QString base = nombreArchivo.left(nombreArchivo.lastIndexOf('.'));
    const QString destino = carpeta + "/" + base + "_" + sello + ".json";
    if (!QFile::copy(origen, destino)) {
        if (error)
            *error = QStringLiteral("No se pudo respaldar %1 en %2.")
                         .arg(nombreArchivo, destino);
        return false;
    }
    return true;
}

bool mismaVentaCierre(const VentaDia& a, const VentaDia& b)
{
    return a.tipo == b.tipo && a.caja == b.caja && a.fecha == b.fecha
        && a.cliente == b.cliente && qAbs(a.monto - b.monto) < 0.005
        && a.ultimos4 == b.ultimos4 && a.referencia == b.referencia;
}

bool referenciaEnCierre(const CierreDia& cierre, const QString& referencia)
{
    for (const VentaDia& venta : cierre.detalle) {
        if (venta.referencia == referencia)
            return true;
    }
    return false;
}

// (e) Marca como enviados solo los pagos y facturas que fueron al cierre.
bool marcarCuotasEnviadas(const CierreDia& cierre, const QString& fechaEnvio,
                          QString* error)
{
    RepositorioCuotas repositorio;
    QList<Cuota> cuotas;
    if (!repositorio.cargar(&cuotas, error))
        return false;

    QList<MarcaEnvioCuotas> marcas;
    for (int i = 0; i < cuotas.size(); ++i) {
        const Cuota& cuota = cuotas.at(i);
        MarcaEnvioCuotas marca;
        marca.indiceCuota = i;

        for (int j = 0; j < cuota.pagos.size(); ++j) {
            const PagoCuota& pago = cuota.pagos.at(j);
            if (pago.enviadoAdmin)
                continue;
            const QString id = RepositorioCuotas::idPago(
                cuota.cliente, pago.fecha, pago.monto);
            if (referenciaEnCierre(cierre, id))
                marca.pagos.append(j);
        }

        if (cuota.factura.numero > 0 && !cuota.factura.enviadaAdmin
            && referenciaEnCierre(cierre, cuota.factura.numeroTexto())) {
            marca.factura = true;
        }

        if (!marca.pagos.isEmpty() || marca.factura)
            marcas.append(marca);
    }

    if (marcas.isEmpty())
        return true;   // nada nuevo que marcar
    return repositorio.marcarEnviados(marcas, fechaEnvio, error);
}

// (d) Relee la bitácora y comprueba que contiene todo lo enviado.
bool verificarBitacora(const QDate& fecha, const CierreDia& enviado, QString* error)
{
    RepositorioBitacora bitacora;
    QList<CierreDia> cierres;
    if (!bitacora.todos(&cierres, error))
        return false;

    for (const CierreDia& guardado : cierres) {
        if (guardado.fecha != fecha)
            continue;

        for (const VentaDia& esperada : enviado.detalle) {
            bool encontrada = false;
            for (const VentaDia& fila : guardado.detalle) {
                if (mismaVentaCierre(esperada, fila)) {
                    encontrada = true;
                    break;
                }
            }
            if (!encontrada) {
                if (error)
                    *error = QStringLiteral("La bitácora no contiene un registro enviado "
                                            "(%1 · %2 · $%3).")
                                 .arg(esperada.tipo, esperada.cliente,
                                      QString::number(esperada.monto, 'f', 2));
                return false;
            }
        }
        return true;
    }

    if (error)
        *error = QStringLiteral("No se encontró el cierre en la bitácora tras guardarlo.");
    return false;
}

// (e) Borra de historial.json SOLO los registros Efectivo/Tarjeta que se enviaron.
// "Tarjeta (cuotas)" no coincide con "Tarjeta" y permanece intacto.
bool borrarVentasEnviadas(const CierreDia& cierre, QString* error)
{
    cargarHistorial();

    QList<Compra> restantes;
    for (const Compra& compra : historialCompras) {
        bool enviada = false;
        if (compra.metodo == QStringLiteral("Efectivo")
            || compra.metodo == QStringLiteral("Tarjeta")) {
            for (const VentaDia& venta : cierre.detalle) {
                if (venta.tipo != compra.metodo || venta.fecha != compra.fecha)
                    continue;
                if (qAbs(venta.monto - compra.total) >= 0.005)
                    continue;
                enviada = true;
                break;
            }
        }
        if (!enviada)
            restantes.append(compra);
    }

    QJsonArray arreglo;
    for (const Compra& compra : restantes)
        arreglo.append(compra.toJson());

    QSaveFile archivo(rutaArchivo(QStringLiteral("historial.json")));
    if (!archivo.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("No se pudo abrir historial.json para limpiarlo: %1")
                         .arg(archivo.errorString());
        return false;
    }
    if (archivo.write(QJsonDocument(arreglo).toJson(QJsonDocument::Indented)) == -1) {
        if (error)
            *error = QStringLiteral("No se pudo escribir historial.json: %1")
                         .arg(archivo.errorString());
        archivo.cancelWriting();
        return false;
    }
    if (!archivo.commit()) {
        if (error)
            *error = QStringLiteral("No se pudo guardar historial.json: %1")
                         .arg(archivo.errorString());
        return false;
    }

    historialCompras = restantes;   // mantiene sincronizada la memoria
    return true;
}

} // namespace

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
	connect(ui->btnCuotas, &QPushButton::clicked, this, &MenuPrincipal::abrirCuotas);
	connect(ui->btnCierre, &QPushButton::clicked, this, &MenuPrincipal::realizarCierre);
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

void MenuPrincipal::abrirCuotas()
{
	abrirSubmenu(new VentanaCuotas());
}

void MenuPrincipal::realizarCierre()
{
	const QDate hoy = QDate::currentDate();

	ServicioCierre servicio;
	servicio.agregarFuente(std::make_unique<FuenteHistorialVentas>(
		QStringLiteral("Efectivo"), QStringLiteral("Efectivo")));
	servicio.agregarFuente(std::make_unique<FuenteHistorialVentas>(
		QStringLiteral("Tarjeta"), QStringLiteral("Tarjeta")));
	servicio.agregarFuente(std::make_unique<FuenteCuotas>());

	const QString caja = usuarioActual.nombre.isEmpty() ? usuario_ : usuarioActual.nombre;
	CierreDia cierre;
	QString error;
	if (!servicio.generar(hoy, caja, &cierre, &error)) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"), error);
		return;
	}

	if (cierre.detalle.isEmpty()) {
		QMessageBox::information(this, QStringLiteral("Cierre del día"),
								 QStringLiteral("No hay datos nuevos para enviar"));
		return;
	}

	RepositorioBitacora bitacora;
	QString errorBitacora;
	const bool yaExiste = bitacora.existeCierre(hoy, &errorBitacora);
	if (!errorBitacora.isEmpty()) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"), errorBitacora);
		return;
	}

	QString resumen;
	if (yaExiste)
		resumen += QStringLiteral("Ya existe un cierre de hoy; se agregarán los nuevos registros.\n\n");

	resumen += QStringLiteral("Cierre del %1\n").arg(hoy.toString(QStringLiteral("dd/MM/yyyy")));
	for (const ResumenTipo& tipo : cierre.resumen) {
		resumen += QStringLiteral("%1: %2 — $%3\n")
					   .arg(tipo.tipo)
					   .arg(tipo.cantidad)
					   .arg(QString::number(tipo.total, 'f', 2));
	}
	resumen += QStringLiteral("\nTotal general: $%1")
				   .arg(QString::number(cierre.totalGeneral, 'f', 2));
	resumen += QStringLiteral("\n\nSe enviarán a la bitácora y se borrarán los datos locales "
							 "de tarjeta y efectivo. Las cuotas se conservan en el menú de cuotas.");

	QMessageBox confirmacion(this);
	confirmacion.setWindowTitle(QStringLiteral("Cierre del día"));
	confirmacion.setText(resumen);
	QPushButton* btnConfirmar = confirmacion.addButton(
		QStringLiteral("Confirmar"), QMessageBox::AcceptRole);
	QPushButton* btnCancelar = confirmacion.addButton(
		QStringLiteral("Cancelar"), QMessageBox::RejectRole);
	confirmacion.setDefaultButton(btnCancelar);
	confirmacion.exec();

	if (confirmacion.clickedButton() != btnConfirmar)
		return;

	// (a) el cierre ya se generó; (b) se respaldan los archivos a modificar.
	const QString sello = selloTiempo();
	if (!copiarRespaldo(QStringLiteral("historial.json"), sello, &error)
		|| !copiarRespaldo(QStringLiteral("cuotas.json"), sello, &error)
		|| !copiarRespaldo(QStringLiteral("bitacora.json"), sello, &error)) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"),
							 QStringLiteral("No se pudo crear el respaldo; no se borró nada.\n%1")
								 .arg(error));
		return;
	}

	// (c) guardar con QSaveFile.
	// TODO(ADMIN): reemplazar por el envío real al menú admin
	if (!bitacora.guardar(cierre, &error)) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"),
							 QStringLiteral("No se pudo guardar el cierre; no se borró nada.\n%1")
								 .arg(error));
		return;
	}

	// (d) releer y verificar antes de tocar los datos locales.
	if (!verificarBitacora(hoy, cierre, &error)) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"),
							 QStringLiteral("La verificación de la bitácora falló; no se borró nada.\n%1")
								 .arg(error));
		return;
	}

	// (e) solo si pasó la verificación se limpia y se marca lo enviado.
	if (!borrarVentasEnviadas(cierre, &error)) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"),
							 QStringLiteral("El cierre se guardó, pero no se pudo limpiar "
											"los datos locales:\n%1").arg(error));
		return;
	}

	const QString fechaEnvio = cierre.horaGeneracion.toString(
		QStringLiteral("dd/MM/yyyy hh:mm:ss"));
	if (!marcarCuotasEnviadas(cierre, fechaEnvio, &error)) {
		QMessageBox::warning(this, QStringLiteral("Cierre del día"),
							 QStringLiteral("El cierre se guardó, pero no se pudieron marcar "
											"los pagos de cuotas:\n%1").arg(error));
		return;
	}

	QMessageBox::information(this, QStringLiteral("Cierre enviado"),
							 QStringLiteral("Cierre enviado a la bitácora"));
}

void MenuPrincipal::salir()
{
	const auto respuesta = QMessageBox::question(
		this,
		QStringLiteral("Cerrar sesión"),
		QStringLiteral("¿Seguro que deseas cerrar sesión?"),
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
