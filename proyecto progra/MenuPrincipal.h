#pragma once

#include <QWidget>

class MenuPrincipal : public QWidget
{
	Q_OBJECT

public:
	explicit MenuPrincipal(const QString& usuario, bool esAdmin = false,
						   QWidget* parent = nullptr);

signals:
	void sesionCerrada();

private slots:
	void abrirCuenta();
	void abrirCobro();
	void abrirCatalogo();
	void abrirCarrito();
	void abrirAdmin();
	void salir();

private:
	void abrirSubmenu(QWidget* ventana);

	QString usuario_;
	bool esAdmin_;
};
