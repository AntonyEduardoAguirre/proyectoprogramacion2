#include "MenuPrincipal.h"

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	MenuPrincipal menu(QStringLiteral("anderson"), true);
	menu.show();

	return app.exec();
}
