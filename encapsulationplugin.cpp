#include "encapsulationplugin.h"
#include "encapsulationconstants.h"
#include "encapsulationsettingspage.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/variablemanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>

#include <cplusplus/Overview.h>
#include <cpptools/cppmodelmanager.h>
#include <extensionsystem/pluginmanager.h>

#include <TranslationUnit.h>
#include <Symbols.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>

namespace Encapsulation
{
namespace Internal
{

Plugin::Plugin():
	m_settings(new Settings())
{
}

Plugin::~Plugin()
{
}

bool Plugin::initialize(const QStringList &arguments, QString *errorString)
{
	Q_UNUSED(arguments)
	Q_UNUSED(errorString)
	Core::ActionManager *am = Core::ICore::instance()->actionManager();

	QAction *action = new QAction(tr("Encapsulate"), this);
	Core::Command *cmd = am->registerAction(action, Constants::ACTION_ID, Core::Context(Core::Constants::C_EDIT_MODE));
	cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+E")));
	connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

	am->actionContainer(CppTools::Constants::M_TOOLS_CPP)->addAction(cmd);

	addAutoReleasedObject(new SettingsPage(m_settings));

	return true;
}

void Plugin::extensionsInitialized()
{
	m_settings->fromSettings(Core::ICore::settings());
}

ExtensionSystem::IPlugin::ShutdownFlag Plugin::aboutToShutdown()
{
	return SynchronousShutdown;
}

void Plugin::triggerAction()
{
	Core::IEditor *editor = Core::EditorManager::instance()->currentEditor();
	CPlusPlus::CppModelManagerInterface *modelManager = ExtensionSystem::PluginManager::instance()->getObject<CPlusPlus::CppModelManagerInterface>();
	CPlusPlus::Document::Ptr doc = modelManager->snapshot().document(editor->file()->fileName());

	CPlusPlus::Symbol *lastSymbol = doc->lastVisibleSymbolAt(editor->currentLine(), 9999);
	if(lastSymbol->isDeclaration()) {
		CPlusPlus::Declaration *decl = lastSymbol->asDeclaration();

		QString prefix = m_settings->fieldPrefix;
		QString suffix = m_settings->fieldSuffix;
		bool add_get = m_settings->addGetString;
		bool camel_case = m_settings->useCamelCase;
		bool cpp_file = m_settings->cppFile;
		bool get_first = m_settings->mutatorFirst;

		QString var_type = CPlusPlus::Overview().prettyType(lastSymbol->type());
		QString var_name = CPlusPlus::Overview().prettyName(lastSymbol->name());
		QString name = var_name.mid(prefix.length(), var_name.length() - (prefix.length() + suffix.length()));
		QString cap = camel_case ? (name[0].toUpper() + name.mid(1)) : name;
		QString get_name = (add_get ? QString("get") + (camel_case ? "" : "_") + cap : name);
		QString set_name = QString("set") + (camel_case ? "" : "_") + cap;

		bool set_exists = false;
		bool get_exists = false;

		if(!decl->type()->isFunctionType() &&
			(decl->isPrivate() || decl->isProtected()) &&
			decl->enclosingScope() &&
			decl->enclosingScope()->isClass()) {
			QList<CPlusPlus::Symbol*> public_func;
			for(unsigned int i = 0; i < decl->enclosingClass()->memberCount(); ++i) {
				CPlusPlus::Symbol *symb = decl->enclosingClass()->memberAt(i);

				if(symb->type()->isFunctionType()) {
					QString sym_name = CPlusPlus::Overview().prettyName(symb->name());

					if(sym_name == set_name) {
						set_exists = true;
					} else if(sym_name == name) {
						get_exists = true;
					}

					if(symb->isPublic() && !symb->type()->asFunctionType()->isSlot()) {
						public_func << symb;
					}
				}
			}

			if(set_exists && get_exists) {
				return;
			}

			if(!public_func.count()) {
				QMessageBox::information(Core::ICore::mainWindow(), QCoreApplication::tr("Can't do it"), QCoreApplication::tr("You need to have atleast one public function."));
				return;
			}

			int l = public_func.last()->line();
			int c = public_func.last()->column();

			QString get = var_type;
			QString set = "void " + set_name;

			if(!decl->type()->isPointerType()) {
				get += " " + get_name + "() const";
				set +=  + "(const " + var_type + " &";
			} else {
				get += get_name + "()";
				set += "(" + var_type;
			}

			set += name + ")";

			//QString cpp_get;
			//QString cpp_set;

			if(cpp_file) {
				get += ";";
				set += ";";
			} else {
				get += " { return " + var_name + "; }";
				set += " { " + var_name + " = " + name + "; }";
			}

			TextEditor::BaseTextEditorWidget *editorWidget = qobject_cast<TextEditor::BaseTextEditorWidget*>(editor->widget());
			if(editorWidget) {
				QTextCursor cur = editorWidget->textCursor();

				editorWidget->gotoLine(l, c);
				editorWidget->insertLineBelow();

				if(!get_exists && !get_first) {
					editorWidget->insertPlainText(get);
					editorWidget->insertLineAbove();
				}

				if(!set_exists) {
					editorWidget->insertPlainText(set);
					editorWidget->insertLineAbove();
				}

				if(!get_exists && get_first) {
					editorWidget->insertPlainText(get);
					editorWidget->insertLineAbove();
				}

				editorWidget->setTextCursor(cur);
			}
		}
	}
}

Q_EXPORT_PLUGIN2(Encapsulation, Plugin)

}
}
