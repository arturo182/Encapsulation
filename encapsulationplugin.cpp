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

#include <cppeditor/cppeditorconstants.h>

#include <cpptools/cppmodelmanagerinterface.h>
#include <cpptools/cpptoolsconstants.h>

#include <cplusplus/Overview.h>
#include <cplusplus/Scope.h>
#include <cplusplus/Symbols.h>
#include <cplusplus/Names.h>


#include <QCoreApplication>
#include <QtPlugin>
#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

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
	Core::ActionManager *am = Core::ActionManager::instance();

    Core::Context context(CppEditor::Constants::C_CPPEDITOR);

	QAction *action = new QAction(tr("Encapsulate"), this);
    Core::Command *cmd = am->registerAction(action, Constants::ACTION_ID, context);
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

using namespace CPlusPlus;


CPlusPlus::Symbol* currentSymbol(Core::IEditor *editor)
{
    int line = editor->currentLine();
    int column = editor->currentColumn();

    CppTools::CppModelManagerInterface *modelManager
            = CppTools::CppModelManagerInterface::instance();

    if (!modelManager) {
        QMessageBox::information(0, QString::fromLatin1("Error"), QString::fromLatin1("Model manager is null"));
        return 0;
    }


    const Snapshot snapshot = modelManager->snapshot();
	Document::Ptr doc = snapshot.document(editor->document()->filePath());
    if (!doc) {
        QMessageBox::information(0, QString::fromLatin1("Error"), QString::fromLatin1("doc manager is null"));
        return 0;
    }

    return doc->lastVisibleSymbolAt(line, column);
}

void switchToSource(Core::IEditor *editor)
{
    // Switch to source file
	const Core::ActionManager *am = Core::ActionManager::instance();

	QFileInfo fi(editor->document()->filePath());

	const Core::MimeType mimeType = Core::MimeDatabase::findByFile(fi);
    const QString typeName = mimeType.type();
    if (typeName == QLatin1String(CppTools::Constants::C_HEADER_MIMETYPE) ||
        typeName == QLatin1String(CppTools::Constants::CPP_HEADER_MIMETYPE)) {
        am->command(CppTools::Constants::SWITCH_HEADER_SOURCE)->action()->trigger();
    }
}

void switchToHeader(Core::IEditor *editor)
{
    // Switch to source file
	const Core::ActionManager *am = Core::ActionManager::instance();

	QFileInfo fi(editor->document()->filePath());
	const Core::MimeType mimeType = Core::MimeDatabase::findByFile(fi);
    const QString typeName = mimeType.type();
    if (typeName == QLatin1String(CppTools::Constants::C_SOURCE_MIMETYPE) ||
        typeName == QLatin1String(CppTools::Constants::CPP_SOURCE_MIMETYPE)) {
        am->command(CppTools::Constants::SWITCH_HEADER_SOURCE)->action()->trigger();
    }
}

void Plugin::triggerAction()
{
	Core::IEditor* editor = Core::EditorManager::currentEditor();

    CPlusPlus::Symbol* lastSymbol = currentSymbol(editor);


    if (lastSymbol == 0) {
         return;
    }

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
        QString get_name = (add_get ? QString::fromLatin1("Get") + QString::fromLatin1(camel_case ? "" : "_") + cap : name);
        QString set_name = QString::fromLatin1("Set") + QString::fromLatin1(camel_case ? "" : "_") + cap;

		bool set_exists = false;
		bool get_exists = false;


		if(!decl->type()->isFunctionType() &&
			(decl->isPrivate() || decl->isProtected()) &&
			decl->enclosingScope() &&
			decl->enclosingScope()->isClass()) {

            QString className = CPlusPlus::Overview().prettyType(decl->enclosingClass());

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
                QMessageBox::information(Core::ICore::mainWindow(), QCoreApplication::tr("Can't do it"), QCoreApplication::tr("You need to have at least one public function."));
				return;
			}

			int l = public_func.last()->line();
			int c = public_func.last()->column();

            QString get;// = var_type;
            QString set;// = QString::fromLatin1("void \t");


            if(!decl->type()->isPointerType()) {
                get += get_name + QString::fromLatin1("() const");

                if (decl->type()->isIntegerType() || decl->type()->isFloatType()) {
                    set += set_name + QString::fromLatin1("(") + var_type + QString::fromLatin1(" ");
                } else {
                    set += set_name + QString::fromLatin1("(const ") + var_type + QString::fromLatin1("& ");
                }
			} else {
                get += get_name + QString::fromLatin1("()");
                set += set_name + QString::fromLatin1("(") + var_type;
			}

            QString localName = name[0].toLower() + name.mid(1);
            set += localName + QString::fromLatin1(")");


            QString getDefn = var_type + QString::fromLatin1(" ") + className + QString::fromLatin1("::") + get;
            QString setDefn = QString::fromLatin1("void ") + className + QString::fromLatin1("::") + set;

            get = var_type + QString::fromLatin1(" \t") + get;
            set = QString::fromLatin1("void \t") + set;


			if(cpp_file) {
                get += QString::fromLatin1(";");
                set += QString::fromLatin1(";");

                getDefn += QString::fromLatin1("\n{\n\treturn ") + var_name + QString::fromLatin1(";\n}\n\n");
                setDefn += QString::fromLatin1("\n{\n\t") + var_name + QString::fromLatin1(" = ") + localName + QString::fromLatin1(";\n}\n\n");
			} else {
                get += QString::fromLatin1(" { return ") + var_name + QString::fromLatin1("; }");
                set += QString::fromLatin1(" { ") + var_name + QString::fromLatin1(" = ") + localName + QString::fromLatin1("; }");
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

            if (cpp_file) {
                switchToSource(editor);

				editorWidget = qobject_cast<TextEditor::BaseTextEditorWidget*>(Core::EditorManager::currentEditor()->widget());
                if(editorWidget) {

                    editorWidget->moveCursor(QTextCursor::End);
                    editorWidget->find(QString::fromAscii("}"), QTextDocument::FindBackward);
                    editorWidget->moveCursor(QTextCursor::EndOfLine);
                    editorWidget->insertPlainText(QString::fromAscii("\n"));
                    editorWidget->insertLineBelow();


                    if(!get_exists && get_first) {
                        editorWidget->insertPlainText(getDefn);
                        editorWidget->insertLineAbove();
                    }

                    if(!set_exists) {
                        editorWidget->insertPlainText(setDefn);
                        editorWidget->insertLineAbove();
                    }

                    if(!get_exists && !get_first) {
                        editorWidget->insertPlainText(getDefn);
                        editorWidget->insertLineAbove();
                    }
                } else {
                     QMessageBox::information(0, QString::fromLatin1("Info"), QString::fromLatin1("Editor widget is null"));
                }

				switchToHeader(Core::EditorManager::currentEditor());
            }
		}

	}

}

Q_EXPORT_PLUGIN2(Encapsulation, Plugin)

}
}
