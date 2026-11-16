#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QVBoxLayout;
class QStackedWidget;

class Administracion;

class AccesoAdmin : public QWidget
{
public:
	explicit AccesoAdmin(QWidget* parent = nullptr);

private:
	void validarContrasenna();
	QLineEdit* contrasenna_;
	QLabel* error_;
};

class Administracion : public QWidget
{
public:
	explicit Administracion(QWidget* parent = nullptr);

private:
	QWidget* crearProductos();
	QWidget* crearCuentas();
	void actualizarProductos();
	void actualizarCuentas();
	void editarProducto(int indice);
	void eliminarProducto(int indice);
	void editarCuenta(int indice);

	QStackedWidget* paginas_;
	QVBoxLayout* listaProductos_;
	QVBoxLayout* listaCuentas_;
};
