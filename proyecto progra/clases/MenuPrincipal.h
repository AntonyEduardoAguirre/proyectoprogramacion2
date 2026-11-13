#pragma once

#include <QWidget>

namespace Ui
{
class MenuPrincipalClass;
}

class MenuPrincipal : public QWidget
{
	Q_OBJECT

public:
	explicit MenuPrincipal(const QString& usuario, bool esAdmin = false,
						   QWidget* parent = nullptr);
	~MenuPrincipal() override;

signals:
	void sesionCerrada();

private slots:
	void abrirCatalogo();
	void salir();

private:
  void conectarSenales();
	void abrirSubmenu(QWidget* ventana);

	QString usuario_;
	Ui::MenuPrincipalClass* ui;
};
