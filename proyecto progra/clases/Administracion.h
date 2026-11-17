#pragma once

#include <QWidget>
#include <QString>

class QVBoxLayout;

namespace Ui {
class AdminCobros;
class AdminCuentas;
class AdminCuotas;
class AdminProductos;
class Administracion;
}

class Administracion : public QWidget
{
public:
	explicit Administracion(QWidget* parent = nullptr);
	~Administracion();

private:
	QWidget* crearProductos();
	QWidget* crearCuentas();
	QWidget* crearCuotas();
	QWidget* crearCobros();
	void actualizarProductos();
	void actualizarCuentas();
	void actualizarCuotas();
	void actualizarCobros();
	void editarProducto(int indice);
	void eliminarProducto(int indice);
	void editarCuenta(int indice);
	void eliminarCuenta(int indice);
	void eliminarCuota(int indiceRegistro);
	void pagarCuota(int indiceRegistro, int indiceCuota);
	bool eliminarCobro(bool esCuota, int indiceRegistro, int indicePago);
	void mostrarDetalleCobros(const QString& usuarioKey, const QString& mes);

	Ui::Administracion* ui;
	Ui::AdminProductos* uiProductos_;
	Ui::AdminCuentas* uiCuentas_;
	Ui::AdminCuotas* uiCuotas_;
	Ui::AdminCobros* uiCobros_;
	QVBoxLayout* listaProductos_;
	QVBoxLayout* listaCuentas_;
	QVBoxLayout* listaCuotas_;
	QVBoxLayout* listaCobros_;
};
